#
# Program: TPP AWS Search Tool
# Author:  Joe Slagel
#
# Copyright (C) 2009-2013 by Joseph Slagel
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
# $Id: CometService.pm 6285 2013-09-20 16:50:03Z slagelwa $
#
package TPP::AWS::CometService;
use strict;
use warnings;

use Cwd qw( abs_path getcwd );
use File::Basename;
use File::Copy;
use File::Spec::Functions qw( rel2abs );
use File::Which qw(which);
use XML::Simple;

use TPP::AWS;
use TPP::AWS::Logger qw( $log );

use base qw(TPP::AWS::SearchService);


#-- @GLOBALS ------------------------------------------------------------------#

our $REVISION = (q$Revision: 6285 $ =~ /(\d+)/g)[0] || '???'; 


#-- @PUBLIC -------------------------------------------------------------------#

#
# Constructor.
#
# @arg  Path to mzXML formatted spectrum file to search with tandem
# @arg  Path to tandem parameters file to use for search
# @arg  Path to output directory (optional)
#
sub new
   {
   my $class = shift;
   my ( $input, $params, $odir ) = @_;

   my $self = $class->SUPER::new(@_);
   $self->{INPUT_FILES}->{INPUT}  = abs_path($input);
   $self->{INPUT_FILES}->{PARAMS} = abs_path($params); 
   $self->{INPUT_FILES}->{DB}     = $self->_readParams( $params );
   return $self;
   }


#-- @PROTECTED ----------------------------------------------------------------#

#
# Runs comet search on given input
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
   unlink "$root.pepXML", "$root.pep.xml", "$root.comet.log";

   # Locate programs needed
   my $comet = which('comet')
      or $log->logdie( "Error comet program not found in path" );
   my $interactparser = which('InteractParser')
      or $log->logdie( "Error InteractParser program not found in path" );

   # Fix database path
   $self->_updateParams( $params, $db );

   # Run search
   $log->debug( "invoking comet search on $root$suffix" );
   push @$output, "$root.comet.log";
   my $cmd = "$comet -P$params -N'$odir$root' '$input'";
   $self->system( $cmd, "$root.comet.log" );

   # Now run InteractParser to (potentially) fix the output
   # Use FULL paths with no extra extensions
   move( "$odir$root.pep.xml", "$odir$root.pepXML" );
   $log->debug( "invoking InteractParser on $odir$root.pepXML" );
   $cmd = "$interactparser '$odir$root.pep.xml' '$odir$root.pepXML' ";
   $cmd .= "-L0 -R9999 -a'$idir'";
   $cmd .= " >> '$root.comet.log' 2>&1";
   $self->system( $cmd, "$root.comet.log" );
   unlink "$odir$root.pepXML";

   # Now fix paths in the output
   push @$output, "$odir$root.pep.xml";
   $self->_updatePepXML( $output->[-1], qw(INPUT PARAMS DB) );
   }


#-- @PRIVATE ------------------------------------------------------------------#

#
# Parse the contents of a comet parameters file and return a list of values 
# of important parameters.
#
# Returns absolute path to database file
#
sub _readParams
   {
   my ( $self, $paramsFile ) = @_;    # @arg filename of parameters file

   $log->logdie( "missing parameter file $paramsFile" ) 
      unless ( $paramsFile && -f $paramsFile );
      
   # Read configuration from parameters file
   my ( $db, $pepxml ) = ( undef, undef );
   open( PARAMS, $paramsFile ) or  die "can't open file $paramsFile, $!\n";
   while ( <PARAMS> )
      {
      $db     = $1 if ( /^\s*database_name\s*=\s*(.*)\s*/i );
      $pepxml = $1 if ( /^\s*output_pepxmlfile\s*=\s*(.*)\s*/i );
      }
   close( PARAMS );
   
   $db = rel2abs( $db );
   $log->logdie( "missing database $db in parameter file $paramsFile" ) 
      unless ( $db && -f $db );
   $log->logdie( "output_pepxmlfile parameter should be set to 1" ) 
      unless ( $pepxml );
      
   $log->debug( "comet DB = $db" );
   return $db;
   }

#
# Changes the contents of a comet parameter file, replacing specific values
# of parameters as directed.
#
#  @arg   source parameter filename
#  @arg   name of database file to replace
#
sub _updateParams
   {
   my ( $self, $params, $db ) = @_;

   $log->debug( "updating $params parameters" );

   open( IN,  "+< $params" ) or die( "$0: can't open $params, $!\n" );

   my $out = '';        # use memory 'cause file is small
   while ( <IN> )
      {
      s/^(\s*database_name\s*=\s*)(.*)\s*$/$1$db\n/i;
      $out .= $_;
      }
   truncate( IN, 0 );
   seek( IN, 0, 0 ) or die( "$0: can't seek to start of $params: $!" );
   print IN $out;

   close IN;
   }

1;
