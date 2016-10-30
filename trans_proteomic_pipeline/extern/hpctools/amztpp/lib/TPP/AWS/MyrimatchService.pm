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
# $Id: MyrimatchService.pm 6285 2013-09-20 16:50:03Z slagelwa $
#
package TPP::AWS::MyrimatchService;
use strict;
use warnings;

use Cwd qw( abs_path getcwd );
use File::Basename;
use File::Copy;
use File::Spec::Functions qw( rel2abs );
use File::Which qw(which);

use TPP::AWS;
use TPP::AWS::Logger qw( $log );

use base qw(TPP::AWS::SearchService);


#-- @GLOBALS ----------------------------------------------------------------#

our $REVISION = (q$Revision: 6285 $ =~ /(\d+)/g)[0] || '???'; 


#-- @PUBLIC -----------------------------------------------------------------#

#
# Constructor.
#
# @arg  Path to mzXML formatted spectrum file to search with Myrimatch
# @arg  Path to parameters file to use for search
# @arg  Path to output directory (optional)
#
sub new
   {
   my $class = shift;
   my ( $input, $params, $odir ) = @_;

   my $self = $class->SUPER::new( @_);

   # Get database files and any command line flags from the parameters file
   my @p = $self->_readParams( $self->{INPUT_FILES}->{PARAMS} );
   $self->{INPUT_FILES}->{DB} = $p[0];
   $self->{CMD_FLAGS}         = $p[1] || '';
   return $self;
   }
 
 
#-- @PROTECTED ----------------------------------------------------------------#

#
# Runs myrimatch search on given input.
#
sub run
   {
   my ( $self ) = @_;

   my $input  = $self->{WORK_FILES}->{INPUT};
   my $params = $self->{WORK_FILES}->{PARAMS};
   my $db     = $self->{WORK_FILES}->{DB};

   my ( $root, $idir, $suffix ) = fileparse( $input, qr/\.[^.]*$/ );
   my $odir = $self->odir();
   $odir .= '/';

   my $output = $self->{OUTPUT_FILES};
   unlink "$root.pep.xml", "$root.myrimatch.log";

   my $interactparser = which('InteractParser')
      or $log->logdie( "Error InteractParser program not found in path" );

   # Use location of inspect executable for directory containing resources
   my $myrimatch = which('myrimatch')
      or $log->logdie( "Error myrimatch program not found in path" );
   my $res = (fileparse( $myrimatch ))[1];

   # Fix database path
   $self->_updateParams( $params, $db );

   # Run search
   $log->debug( "invoking myrimatch search on $root$suffix" );
   push @$output, "$root.myrimatch.log";
   my $cmd = "$myrimatch -cfg $params $self->{CMD_FLAGS} '$input'";
   $self->system( $cmd, "$root.myrimatch.log" );

   # Now run InteractParser to (potentially) fix the output
   # Use FULL paths with no extra extensions
   $log->debug( "invoking InteractParser on $root.pepXML" );
   $cmd = "$interactparser '$odir$root.pep.xml' '$odir$root.pepXML' ";
   $cmd .= "-L0 -R9999 -a'$idir'";
   $self->system( $cmd, "$root.myrimatch.log" );
   unlink "$root.pepXML";

   # Now fix paths in the output
   push @$output, rel2abs("$root.pep.xml");
   $self->_updatePepXML( $output->[-1], qw(INPUT PARAMS DB) );
   }


#-- @PRIVATE -----------------------------------------------------------------#

#
# Read the inspect parameters file for the database name
#
# @arg   parameter filename
#
sub _readParams
   {
   my ( $self, $params ) = @_;
   
   my $db  = undef;
   my $cmd = "";
   open( PARAMS, $params ) or die "can't open file $params, $!\n";
   while ( <PARAMS> )
      {
      if ( /^\s*ProteinDatabase\s*=\s*(.*)\s*$/i )    # found database?
         {
         $db = $1; 
         }
      if ( /^\s*#\$\s*(-\w+)\s+(.*)\s*/i )	      # found cmdline parameter?
         {
         $cmd .= "$1 $2 ";
         }
      }
   close( PARAMS );
   return( rel2abs( $db ), $cmd );
   }

#
# Changes the contents of a myrimatch parameter file, replacing specific values
# of parameters as directed.
#
#  @arg   source parameter filename 
#  @arg   name of database file to replace (optional)
#
sub _updateParams
   {
   my ( $self, $src, $db ) = @_;

   $log->debug( "updating $src parameters" ); 
   
   open( IN,  "+< $src" ) or die( "$0: can't open $src, $!\n" );

   my $out = '';        # use memory 'cause file is small
   while ( <IN> )
      {
      s/(\s*ProteinDatabase\s*)=\s*(.*)\s*$/$1=$db\n/i;
      $out .= $_;
      }
   truncate( IN, 0 );
   seek( IN, 0, 0 ) or die( "$0: can't seek to start of $src: $!" );
   print IN $out;

   close IN;
   }

1;
