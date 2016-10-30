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
# $Id: DaemonClient.pm 5922 2012-08-13 21:30:25Z slagelwa $
#

# 
# Subclass of Client that implements a unix like daemon client using fork.
#
package TPP::AWS::DaemonClient;
use strict;
use warnings;

use Class::InsideOut qw( :std );
use File::Spec::Functions;

use TPP::AWS::Logger qw( $log );
use TPP::AWS::Poll;
use TPP::AWS::Report;

use base 'TPP::AWS::Client';


#-- @GLOBALS ----------------------------------------------------------------#

our $REVISION = (q$Revision: 5922 $ =~ /(\d+)/g)[0] || '???'; 


#-- @ATTRIBUTES -------------------------------------------------------------#


#-- @PUBLIC -----------------------------------------------------------------#


#-- @PRIVATE ----------------------------------------------------------------#

#
# fork a background process to turn it into a daemon
#
sub _spawn
   {
   my ( $self, $logh ) = @_;
   
   $log->debug( "forking background process" );
   if ( my $pid = fork() ) 
      { 
      return $pid;              # parent process comes here
      }
   elsif ( (defined $pid) && ($pid == 0) )
      {
      close $_ for *STDIN, *STDOUT, *STDERR;
      chdir( '/' ) or $log->error( "cannot change to '/': $!" );
      open( $logh, ">/dev/null" ) unless $logh;
      open( STDIN,  "</dev/null" );
      open( STDOUT, ">&LOG" ); select( ( select(STDOUT), $| = 1 )[0] );
      open( STDERR, ">&LOG" ); select( ( select(STDERR), $| = 1 )[0] );
      $log->{handle} = \*STDERR;
      POSIX::setsid() or $log->logdie( "cannot start a new session: $!" );
      return 0;
      }
   else
      {
      $log->logdie( "cannot fork background process: $!" );   
      }
   }


1;
