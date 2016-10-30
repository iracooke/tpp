#
# Program: TPP AWS Tool
# Author:  Joe Slagel
#
# Copyright (C) 2012 by Joseph Slagel
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
# $Id: ParallelManager.pm 6285 2013-09-20 16:50:03Z slagelwa $
#

# 
# Manages spawning processes for uploading and downloading files in parallel.
#
package TPP::AWS::ParallelManager;
use strict;
use warnings;

use Class::InsideOut qw( :std );
use File::Spec::Functions;
use Module::Load;
use POSIX ':sys_wait_h';

use TPP::AWS::Logger qw( $log );


#-- @GLOBALS ----------------------------------------------------------------#

our $REVISION = (q$Revision: 6285 $ =~ /(\d+)/g)[0] || '???'; 


#-- @ATTRIBUTES -------------------------------------------------------------#

readonly children => my %children;
public   max      => my %max;


#-- @PUBLIC -----------------------------------------------------------------#

#
# Constructor
#
# @arg   Maximum number of parallel upload/download processes (default 2)
#
sub new
   {
   my ( $class, $max ) = @_;
   
   # Create object
   my $self = Class::InsideOut::new( $class );
   my $id   = id $self;
   
   $max{$id}      = defined $max ? $max : 2;
   $children{$id} = {};  
   
   return $self;
   }
   
#
# Spawn another parallel upload
#
sub spawnUpload
   {
   my ( $self, $s3m, $sqsm, $srv ) = @_;
   
   $log->debug( "spawning upload process" );
   
   my $children = $self->children();
   if ( my $pid = fork() )
      {
      # Kludge to update list of files that should have been uploaded
      foreach ( values %{ $srv->{INPUT_FILES} } )
         {
         $s3m->putList()->{$_} = 1;
         }
         
      $children->{$pid} = time();
      return $pid;
      }
   elsif ( $pid == 0 )
      {
      local $SIG{INT} = $SIG{TERM} = 'DEFAULT';
      $log->debug( "spawned upload of input data (PID:$$)" );
      
      # Include PID in debugging logging messages
      $log->{fmt} = '[%d] (%p) PID:%P %m' if ( $log->{level} == 1 );
         
      eval {
         $srv->uploadInput( $s3m ); 
         $sqsm->queueService( $srv );
         };
      if ( my $e = $@ ) 
         {
         push @{ $srv->{ERRORS} }, $e;
         $log->error( "error uploading search: $e" );
         $sqsm->queueDone( $srv );
         }
      
      $sqsm->deleteMessage( $sqsm->upQueue(), $srv );
      exit( 0 );
      }
   else
      {
      $log->logdie( "can't fork() process: $!" );	
      }
   }
   
#
# Spawn another parallel download
#
sub spawnDownload
   {
   my ( $self, $s3m, $sqsm, $srv ) = @_;
   	
   $log->debug( "spawning download process" );
   
   if ( my $pid = fork() )
      {
      $self->children()->{$pid} = time();
      return $pid;
      }
   elsif ( $pid == 0 )
      {
      local $SIG{INT} = $SIG{TERM} = 'DEFAULT';
      $log->debug( "spawned download of output data (PID:$$)" );
      
      # Include PID in debugging logging messages
      $log->{fmt} = '[%d] (%p) PID:%P %m' if ( $log->{level} == 1 );
      
      eval { 
         $srv->chodir(); 
         $srv->downloadOutput( $s3m ); 
      };
      if ( my $e = $@ )
         {
	 push @{ $srv->{ERRORS} }, "downloading data: $e";
         }
      $log->error( "service error: $_" ) foreach ( @{ $srv->{ERRORS} } );
      $sqsm->queueDone( $srv );
      $sqsm->deleteMessage( $sqsm->downQueue(), $srv );
      exit( 0 );
      }
   else
      {
      $log->logdie( "can't fork() process: $!" );	
      }
   }

#
# Return count of children
#
sub count
   {
   my ( $self ) = @_;	
   return scalar( keys %{ $self->children() } );
   }

#
# Checks to see if any childred processes have finished. Does so in a
# non-blocking way.
#
sub reap
   {
   my ( $self ) = @_;
   
   $log->debug( "reaping children processes" );
   
   my $pids = $self->children();
   my @reap;
   foreach ( keys %$pids )
      {
      my $kid = waitpid( $_, WNOHANG );	
      next if ( $kid == 0 || $kid == -1 );      # Still running
      $log->debug( "child (PID:$kid) finished exited($?)" );
      delete $pids->{$kid} or $log->logdie( "reaped child process $kid not found in PID table");
      push @reap, $kid;
      }
   return @reap;
   }

1;
