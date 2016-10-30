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
# $Id: Logger.pm 5805 2012-04-24 17:21:56Z slagelwa $
#
package TPP::AWS::Logger;
use strict;
use warnings;
use base qw( Exporter );


#-- @GLOBALS ----------------------------------------------------------------#

our $REVISION = (q$Revision: 5805 $ =~ /(\d+)/g)[0] || '???'; 

our @EXPORT_OK = qw( $log );

my $LEVEL = { debug => 1, info => 2, warn => 3, error => 4, fatal => 5 };

my $log;		# TODO: allow override of this


#-- @PUBLIC -----------------------------------------------------------------#

sub new
   {
   my ( $class, $level, $handle ) = @_;
   $level  = $LEVEL->{$level ? $level : 'info'};
   $handle ||= \*STDERR;
   my $self = { handle => $handle, level => $level, fmt => '%m' };
   
   bless( $self, $class );
   return $self;
   }
   
sub debug { is_debug(@_) && print { $_[0]->{handle} } fmt(shift,'debug',@_) }
sub info  { is_info(@_)  && print { $_[0]->{handle} } fmt(shift,'info',@_) }
sub warn  { is_warn(@_)  && print { $_[0]->{handle} } fmt(shift,'warn',@_) }
sub error { is_error(@_) && print { $_[0]->{handle} } fmt(shift,'error',@_) }
sub fatal { is_fatal(@_) && print { $_[0]->{handle} } fmt(shift,'fatal',@_) }

sub is_debug { shift->is_level('debug') }
sub is_info  { shift->is_level('info')  }
sub is_warn  { shift->is_level('warn') }
sub is_error { shift->is_level('error') }
sub is_fatal { shift->is_level('fatal') }

sub logdie { 
   if ( !$^S )
      {
      fatal( @_ );
      exit 1;
      }
   CORE::die( shift && "Error: @_\n" );
};

sub is_level
   {
   my ( $self, $level ) = @_;	
   return $LEVEL->{lc($level)} >= $self->{level};
   }
   
sub log 
   {
   my ($self, $level, @msgs) = @_;

   # Check log level
   return $self unless $self->is_level( $level );

   my $time = localtime(time);
   my $msgs = join "\n", @msgs;

   # Caller
   my ($pkg, $line) = (caller())[0, 2];
      ($pkg, $line) = (caller(1))[0, 2] if $pkg eq ref $self;

   # Write
   my $s = $self->{fmt};
   $s =~ s/\%m/$msgs/;
   $s =~ s/\%p/$level/;
   $s =~ s/\%d/$time/;
   $s =~ s/\%L/$line/;
   $s =~ s/\%C/$pkg/;
   $s =~ s/\%P/$$/;
   chomp $s;
   print { $self->{handle} } "$s\n";
   return $self;
   }
   
sub fmt
   {
   my ($self, $level, @msgs) = @_;
   my $time = localtime(time);
   my $msgs = join "\n", @msgs;

   # Caller
   my ($pkg, $line) = (caller())[0, 2];
      ($pkg, $line) = (caller(1))[0, 2] if $pkg eq ref $self;

   # Write
   my $s = $self->{fmt};
   $s =~ s/\%m/$msgs/;
   $s =~ s/\%p/$level/;
   $s =~ s/\%d/$time/;
   $s =~ s/\%L/$line/;
   $s =~ s/\%C/$pkg/;
   $s =~ s/\%P/$$/;
   chomp $s;
   return "$s\n";
   }   
   
BEGIN 
   {
   $TPP::AWS::Logger::log = TPP::AWS::Logger->new();
   $TPP::AWS::Logger::log->{level} = 2;
   }
   
1;
