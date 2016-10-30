#
# Program: TPP AWS Search Tool
# Author:  Joe Slagel
#
# Copyright (C) 2009-2012 by Joseph Slagel
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
# $Id: Poll.pm 5805 2012-04-24 17:21:56Z slagelwa $
#
package TPP::AWS::Poll;
use strict;
use warnings;
use POSIX qw( floor );


#-- @GLOBALS ----------------------------------------------------------------#

our $REVISION = (q$Revision: 5805 $ =~ /(\d+)/g)[0] || '???'; 


#-- @PUBLIC -----------------------------------------------------------------#

sub new
    {
    my $class = shift;
    my %opts  = @_;
    my $self = {
    	       MIN   => $opts{MIN} || 5,                           
    	       MAX   => $opts{MAX} || 5 * 60,            
    	       AFTER => $opts{AFTER} || (60 * 30),   # default quit after 30 min 
    };
    $self->{POLL} = $self->{MIN};
    $self->{QUIT} = $self->{AFTER};
    bless( $self, $class );
    return $self;
    }

sub next
   {
   my $self = shift;	
   
   my $s = $self->{POLL};
   $self->{POLL} *= 1.2;
   $self->{POLL} = $self->{MAX} if ( $self->{POLL} > $self->{MAX} );
   $self->{QUIT} -= POSIX::floor($self->{POLL});
   return POSIX::floor $s;
   }

sub interval
   {
   return POSIX::floor $_[0]->{POLL};
   }
   
sub reset
   {
   my $self = shift;	
   $self->{POLL} = $self->{MIN};
   $self->{QUIT} = $self->{AFTER};
   }

sub quit
   {
   return( $_[0]->{QUIT} <= 0 );
   }
