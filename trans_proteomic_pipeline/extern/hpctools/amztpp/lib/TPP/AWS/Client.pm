#
# Program: TPP AWS Tool
# Author:  Joe Slagel
#
# Copyright (C) 2011-2012 by Joseph Slagel
# 
# This library is free software; you can redistribute it and/or             
# modify it under the terms of the GNU Lesser General Public                
# License as published by the Free Software Foundation; either              
# version 2.1 of the License, or (at your option) any later version.        
#                                                                           
# This library is distributed in the hope that it will be useful,           
# but WITHOUT ANY WARRANTY; without even the implied warranty of            
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         
# General Public License for more details.
#                                                                           
# You should have received a copy of the GNU Lesser General Public          
# License along with this library; if not, write to the Free Software       
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
# 
# Institute for Systems Biology
# 1441 North 34th St.
# Seattle, WA  98103  USA
# jslagel@systemsbiology.org
#
# $Id: Client.pm 6285 2013-09-20 16:50:03Z slagelwa $
#

# 
# Class which implements the background processing on the client side of 
# the TPP <=> AWS workflow. 
#
# NOTE: out of pure lazyiness this module uses several program global variables
# including $::prog, $::s3mgr, etc.. 
#
package TPP::AWS::Client;
use strict;
use warnings;

use Class::InsideOut qw( :std );
use File::Spec::Functions;
use Module::Load;

use TPP::AWS;
use TPP::AWS::Logger qw( $log );
use TPP::AWS::ParallelManager;
use TPP::AWS::Poll;


#-- @GLOBALS ----------------------------------------------------------------#

our $REVISION = (q$Revision: 6285 $ =~ /(\d+)/g)[0] || '???'; 

our $signal = 0;


#-- @ATTRIBUTES -------------------------------------------------------------#

readonly keys => my %keys;
readonly opts => my %opts;

public pidfile  => my %pidfile;         # Where to store client PID
public logfile  => my %logfile;         # Where to write client logging info

readonly poll   => my %poll;            # Frequency to wake up and do work

private ec2total => my %ec2total;       # Total EC2 instances ever started
private prevPend => my %prevPend;       # Previous # of pending jobs

private dnmgr => my %dnmgr;             # Manage parallel file uploads
private upmgr => my %upmgr;             # Manage parallel file downloads


#-- @PUBLIC -----------------------------------------------------------------#

#
# Constructor
#
sub new
   {
   my ( $class, $keys, $opts ) = @_;
   
   # Create object
   my $self = Class::InsideOut::new( $class );
   my $id   = id $self;
   my $upid = $keys->[0] ? "-$keys->[0]" : '';
 
   $keys{$id} = $keys;
   $opts{$id} = $opts;
   
   $logfile{$id} = $opts->{log}; 
   $pidfile{$id} = $opts->{pidfile};
   $pidfile{$id} ||= catfile( File::Spec->tmpdir(), ".amztpp${upid}.pid" );
   $poll{$id}    = TPP::AWS::Poll->new( AFTER => (60 * 10), MAX => 60 );
   
   $ec2total{$id} = 0;
   $prevPend{$id} = 0;
   
   $upmgr{$id} = TPP::AWS::ParallelManager->new( $opts->{maxprocs} );
   $dnmgr{$id} = TPP::AWS::ParallelManager->new( $opts->{maxprocs} );
   
   return $self;
   }

#
# Returns the PID of the client if its running
#
sub pid
   {
   my $pid = $_[0]->_readpid();
   return ( $pid && kill( 0, $pid ) ) ? $pid : undef;
   }
   
#
# Start the client
#
sub start
   {
   my ( $self ) = @_;   
   
   # Are we already running?
   if ( my $pid = $self->pid() )
      {
      $log->debug( "start() client (PID:$pid) is already running" );
      return 0;
      }
      
   # Make sure queues exist just in case we can't in background
   $::sqsm->createQueues();
       
   # Open log file just in case we can't in background
   my $logh;
   if ( my $logfile = $self->opts()->{log} )
      {
      open( $logh, ">>$logfile" ) or $log->logdie( "can't open file $logfile: $!" );
      $logh->autoflush(1);	
      }
   
   # Spawn into background? 
   if ( !$self->opts()->{foreground} )
      {
      my $pid = $self->_spawn( $logh );         # Returns child PID
      return $pid if ( $pid );                  # ...I'm the parent
      # ...I'm the detached process (daemon)
      }
      
   # Finish setting up the detached process
   select( ( select(STDOUT), $| = 1 )[0] );     # No buffering
   select( ( select(STDERR), $| = 1 )[0] );     # No buffering
   
   $log->{fmt}    = '[%d] (%p) %C: %m';
   $log->{handle} = $logh if ( $logh );
   $log->info( "$::prog $TPP::AWS::VERSION (r$REVISION) starting up" );
   
   $self->_writepid();
   $SIG{INT} = $SIG{TERM} = \&_catch;
   $SIG{HUP} = sub { $log->error( "got SIGHUP, pause and ignore"); sleep(30); };
   $0 = "$::prog-client";                       # Give us a sensible name
   
   # Background Loop
   while ( !$signal ) 
      {
      no warnings;
      my $interval = $self->_work( $::ec2m, $::s3m, $::sqsm );
      last unless $interval;                    # Are we done?
      $log->debug( "sleeping $interval" );
      sleep( $interval );
      }
      
   $self->_finish( $::ec2m );
   exit( 0 );
   }
   
#
# Signal the background process to stop
#
sub stop
   {
   my ( $self ) = @_;
   
   my $pid = $self->pid();
   if ( !$pid )                         # No background process to kill?
      {
      $log->debug( "no client background process to stop" );
      return 0;
      }
   
   $log->debug( "sending TERM to $pid\n" );
   kill TERM => $pid;
   return $pid if ( !$self->pid() );
   
   sleep(1);  
   return $pid if ( !$self->pid() );
   $log->debug( "sending KILL to $pid\n" );
   kill KILL => $pid;
   return $pid if ( !$self->pid() );
   
   sleep(1);  
   return $pid if ( !$self->pid() );
   
   return undef;  # something went wrong
   }
 
 
#-- @PRIVATE ----------------------------------------------------------------#

#
# Reads the pidfile
#
sub _readpid
   {
   my ( $self ) = @_;
   local *PID;
   open( PID, "<", $self->pidfile() ) or return;
   chomp( my $pid = <PID> );
   close PID;
   $log->debug( "read PID($pid) from file " . $self->pidfile() );
   return $pid;
   }

#
# Writes the pidfile
#
sub _writepid
   {
   my ( $self ) = @_;
   local *PID;
   open( PID, ">", $self->pidfile() ) 
      or $log->logdie( "can't open pid file " . $self->pidfile() . ": $!" );
   print PID $$;
   close PID;
   $log->debug( "wrote PID($$) to file " . $self->pidfile() );
   }

#
# Signal handler
#
sub _catch
   {
   my $name = shift;
   $log->debug( "got signal SIG$name shutting down" );
   $signal = 1;
   }

#
# Spawns uploads, downloads, or new EC2 instances as needed. 
#
# @ret   Returns seconds to next interval to run or 0 when done
#
sub _work
   {
   my ( $self, $ec2m, $s3m, $sqsm ) = @_;   
   
   # Are we done?  call here is important in order to update srvCounts
   return 0 if ( !$sqsm->hasActiveQueues() && $self->poll()->quit() );
   
   # Spawn more uploads?
   my $up = $upmgr{id $self};
   $up->reap();                         # check for any finished children
   while ( $up->count() < $up->max() )  
      {
      my $upsrv = $sqsm->dequeueUpload() or last;
      $up->spawnUpload( $s3m, $sqsm, $upsrv );
      $self->poll()->reset();
      }
      
   # Spawn more downloads?
   my $dn = $dnmgr{id $self};
   $dn->reap();                         # check for any finished children
   while ( $dn->count() < $dn->max() )  
      {
      my $downsrv = $sqsm->dequeueDownload() or last;
      $log->error( $_ ) foreach ( @{ $downsrv->{ERRORS} } );
      $dn->spawnDownload( $s3m, $sqsm, $downsrv );
      $self->poll()->reset();
      }
        
   # Allocate EC2 instances?
   my $cnt = eval { $self->_needEC2( $ec2m, $sqsm ) };
   if ( $@ )
      {
      $log->error( "error describing instances/spots: $@" );
      $log->warn( "skipping EC2 provision, unknown state of queues/instances" );
      }
   elsif( $cnt > 0 )
      {
      $log->debug( "EC2 launch, starting $cnt instance(s)" );
      ::ec2Launch( $cnt ); 
      $self->poll()->reset();
      }
   elsif( $cnt < 0 )
      {
      $cnt = -$cnt;
      $log->debug( "EC2 launch,canceling $cnt requested instance(s)" );
      $ec2m->cancel( $cnt );
      $self->poll()->reset();
      }
      
   return( $self->poll()->next );
   }
        
#
# Need more EC2 instances? 
# TODO: Probably should replace with a true PID controller
#
sub _needEC2
   {
   my ( $self, $ec2m, $sqsm ) = @_;
   
   # Previously fetched
   my ( $srvPend, $srvActive ) = @{ $sqsm->srvCounts() };
   my ( $upPend, $upActive )   = @{ $sqsm->upCounts() };
   my $prev = $prevPend{id $self};
   $prevPend{id $self} = $srvPend;
   
   my ( $ec2Pend, $ec2Run ) = $ec2m->counts();
   my $ec2Max  = $self->opts()->{'ec2-max'};
   my $ec2Util = $self->opts()->{'ec2-util'};

   my $util = ($ec2Pend + $ec2Run)/(($srvPend + $srvActive) || 1);      # % util
   
   $log->debug( "services pend: $srvPend active: $srvActive prev: $prev" );
   $log->debug( " uploads pend: $upPend active: $upActive" );
   $log->debug( "     ec2 pend: $ec2Pend run: $ec2Run max: $ec2Max" );
   $log->debug( sprintf( "     ec2 util: %.2f max: %.2f", $util,  $ec2Util ) );
      
   if ( $ec2Pend && ($util >= $ec2Util) && (!$srvPend || ($srvPend - $prev) < 0))
      {
      $log->debug( "cancel EC2, above utilization and trending down" );
      return -1;
      }
   elsif ( !$srvPend ) 
      {
      $log->debug( "skip EC2 launch, no pending services" );
      return 0;
      }
   elsif ( ($ec2Pend + $ec2Run) >= $ec2Max ) 
      {
      $log->debug( "skip EC2 launch, max $ec2Max EC2 instances reached" );
      return 0;
      }
   elsif ( ($ec2Run - $srvActive) > 1 )
      {
      $log->debug( "skip EC2 launch, more than one idle EC2 instances" );
      return 0;
      }
   #elsif ( $ec2Pend && ($ec2Pend >= $srvPend) )         # TODO: necessary?
   #   {
   #   $log->debug( "skip EC2 launch, enough starting instances for pend services" );
   #   return 0;
   #   }
   elsif ( ($srvPend - $prev) < 0 )
      {
      $log->debug( "skip EC2 launch, pending trending down" );
      return 0;
      }
   elsif ( $util >= $ec2Util )
      {
      $log->debug( "skip EC2 launch, above utilization" );
      return 0;
      }
   else
      {
      $log->debug( "launch EC2, below utilization and trending up" );
      return 1;
      }
   }

 
#
# Invoked when stopping the client
#
sub _finish
   {
   my ( $self, $ec2m ) = @_;
   
   $log->debug( "background process stopping" );
   
   # Cancel spot requests
   my @s = eval { $ec2m->getSpotRequests() };
   $log->error( "getting spot requests to cancel on finish" ) if ( $@ );
   if ( @s )
      {
      $log->debug( "cancelling " . scalar(@s) . " spot instances" );
      eval { $ec2m->ec2()->cancel_spot_instance_requests( @s ) };
      $log->logdie( "unable to cancel spot instances: $@" ) if ( $@ );
      } 
   
   unlink $self->pidfile() or $log->warn( "couldn't unlink the pid file $!" );
   $log->info( "background process stopped" );
   close STDERR; close STDOUT;
   }
    
1;
