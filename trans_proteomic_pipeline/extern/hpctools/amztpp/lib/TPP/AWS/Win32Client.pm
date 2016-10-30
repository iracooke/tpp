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
# $Id: Win32Client.pm 5922 2012-08-13 21:30:25Z slagelwa $
#

# 
# Class which implements the background processing on the client side of 
# the TPP <=> AWS workflow. 
#
# NOTE: out of pure lazyiness this module uses several program global variables
# including $::prog, $::s3mgr, etc.. 
#
package TPP::AWS::Win32Client;
use strict;
use warnings;

use Class::InsideOut qw( :std );
use File::Basename;
use File::Spec::Functions;
use Win32;
use Win32::Process;
use Win32::ShellQuote qw( quote_native );

use TPP::AWS::Client;
use TPP::AWS::Logger qw( $log );
use TPP::AWS::Poll;

use base 'TPP::AWS::Client';


#-- @GLOBALS ----------------------------------------------------------------#

our $REVISION = (q$Revision: 5922 $ =~ /(\d+)/g)[0] || '???'; 


#-- @ATTRIBUTES -------------------------------------------------------------#


#-- @PUBLIC -----------------------------------------------------------------#

    
#-- @PRIVATE ----------------------------------------------------------------#

#
# Spawn a background Win32 process 
#
sub _spawn
   {
   my ( $self, $logh ) = @_;
   my $keys = $self->keys();
   
   # Rebuild the command to run 
   # (this is error prone but I don't see a better way of doing it)
   my $perl = $^X; 
   my $prog = File::Spec->catfile( dirname($0), "amztpp" );
   my @cmd  = ( $perl, $prog, 'start', '-f' );                    
   push @cmd, '-v' if ( $self->opts()->{verbose} );
   push @cmd, '-r' if ( $self->opts()->{report} );
   push @cmd, '--access-key', $keys->[0];       # ...use the user's keys
   push @cmd, '--secret-key', $keys->[1];
   push @cmd, '--region',     $keys->[2] if ( $keys->[2] );
   push @cmd, '--bucket',     $keys->[3] if ( $keys->[3] );
   push @cmd, '--pidfile',    $self->pidfile(); # ...use the user's pid file
   foreach ( qw(log maxprocs ec2-num ec2-max ec2-id ec2-type ec2-key ec2-group ec2-spot) )
      {
      push @cmd, "--$_", $self->opts()->{$_} if ( defined $self->opts()->{$_} ); 
      }

   # Build command to run
   $log->debug( "starting $perl $prog ..." );
   my $sysdrive = $ENV{SYSTEMDRIVE} || '';
   Win32::Process::Create( my $p,  $perl, quote_native(@cmd),
                           0, DETACHED_PROCESS | NORMAL_PRIORITY_CLASS,
                          "$sysdrive/" ) 
      or $log->logdie( "can't start background process: " . win32Msg() );
      
   $log->debug( "launched background process" );
   
   # May not immediately start
   my $pid;
   for ( 0..4 )
      {
      return $pid if ( $pid = $self->pid() );
      sleep(1);
      }

   return "0E0";        # return zero but true as it may not have spawned
   }

1;
