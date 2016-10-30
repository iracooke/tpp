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
# $Id: InspectService.pm 6285 2013-09-20 16:50:03Z slagelwa $
#
package TPP::AWS::InspectService;
use strict;
use warnings;

use File::Basename;
use File::Copy;
use File::Spec::Functions qw( rel2abs catfile );
use File::Which qw( which );

use TPP::AWS;
use TPP::AWS::Logger qw( $log );

use base qw(TPP::AWS::SearchService);


#-- @GLOBALS -----------------------------------------------------------------#

our $REVISION = (q$Revision: 6285 $ =~ /(\d+)/g)[0] || '???'; 


#-- @PUBLIC ------------------------------------------------------------------#

#
# Constructor.
#
# @arg  Path to mzXML formatted spectrum file to search with Inspect
# @arg  Path to parameters file to use for search
# @arg  Path to output directory (optional)
#
sub new
   {
   my $class = shift;
   my ( $input, $params, $odir ) = @_;

   my $self = $class->SUPER::new( @_);

   # Get database file from parameters file
   $self->{INPUT_FILES}->{DB} = $self->_readParams( $self->{INPUT_FILES}->{PARAMS} );

   return $self;
   }


#-- @PUBLIC ------------------------------------------------------------------#

#
# Runs inspect search on given input.
#
sub run
   {
   my ( $self ) = @_;

   my $input  = $self->{WORK_FILES}->{INPUT};
   my $params = $self->{WORK_FILES}->{PARAMS};
   my $db     = $self->{WORK_FILES}->{DB};

   my ( $root, $dir, $suffix ) = fileparse( $input, qr/\.[^.]*$/ );

   my $output      = $self->{OUTPUT_FILES};
   my $mgfFile     = catfile( $self->odir(), "$root.mgf" );
   my $inspectFile = catfile( $self->odir(), "$root.inspect" );
   my $pepxmlFile  = catfile( $self->odir(), "$root.pep.xml" );
   unlink "$root.inspect.params", $inspectFile, $pepxmlFile,
      "$root.inspect.log";

   # Copy parameters
   $self->_copyParams( $params, "$root.inspect.params", $mgfFile, $db );
   push @$output, "$root.inspect.params";

   # Use location of inspect executable for directory containing resources
   my $inspect = which('inspect')
      or $log->logdie( "Error inspect program not found in path" );
   my $res = (fileparse( $inspect ))[1];

   # Convert input to mgf (inspect only reads mzML files)
   $log->debug( "converting input to mgf for inspect search" );
   unlink $mgfFile;
   my $MZXML2SEARCH = which('MzXML2Search')
      or $log->logdie( "Error MzXML2Search program not found in path" );
   push @$output, "$root.inspect.log";
   my $cmd = "$MZXML2SEARCH -mgf $input";
   $self->system( $cmd, "$root.inspect.log" );

   move( "$dir$root.mgf", $mgfFile )
      or $log->logdie( "can't move mgf file: $!");
   push @$output, $mgfFile;

   # Run search
   $log->debug( "invoking inspect search on $root$suffix" );
   $cmd = "$inspect -r $res -i $root.inspect.params -o $inspectFile"
           . " -e $root.inspect.err";
   eval { $self->system( $cmd, "$root.inspect.log" ) };
   if ( my $e = $@ ) {
      $self->system( "cat $root.inspect.err", "$root.inspect.log" );
      $log->logdie( $e );
   }
   push @$output, $inspectFile;
   $self->system( "cat $root.inspect.err", "$root.inspect.log" );

   # Convert output
   $log->debug( "converting inspect $inspectFile output" );
   $cmd = "python ${res}InspectToPepXML.py -i $inspectFile -o $pepxmlFile"
        . " -p $root.inspect.params ";
   $self->system( $cmd, "$root.inspect.log" );
   if ( -f $pepxmlFile )
      {
      push @$output, rel2abs($pepxmlFile);
      $self->_updatePepXML( $output->[-1], qw(INPUT PARAMS DB) );
      }
   else
      {
      $log->logdie( "invoking InspectToPepXML.py $?");
      }

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

   my $db = undef;
   open( PARAMS, $params ) or $log->logdie( "can't open file $params: $!" );
   while ( <PARAMS> )
      {
      chomp; s/#.*$//;                                  # remove comments
      next if ( /^$/ );                                 # skip empty lines
      $db = $2 if ( /\s*(SequenceFile|DB)\s*,(.*)$/i )  # found database?
      }
   close( PARAMS );
   $db || $log->logdie( "no database found in $params" );
   return rel2abs($db);
   }

#
# Copies the contents of a inspect parameter file, replacing specific values
# of parameters as directed.  Useful to create a input specific parameters
# file from a template as inspect includes the input file name as one of its
# parameters, necessitating a parameter file for each input.
#
#  @arg   source (template) parameter filename to copy from
#  @arg   destination parameter filename
#  @arg   name of inspect input filename to replace (optional)
#  @arg   name of database file to replace (optional)
#
sub _copyParams
   {
   my ( $self, $src, $dst, $input, $db ) = @_;

   $log->debug( "copying $src parameters to $dst" ); 

   open( IN,  "< $src" ) or $log->logdie( "can't open $src: $!\n" );
   open( OUT, "> $dst" ) or $log->logdie( "can't open $dst: $!\n" );

   while ( <IN> )
      {
      s/^(\s*spectra\s*),.*$/$1,$input/i        if ( $input );
      s/^(\s*SequenceFile\s*),.*$/$1,$db/i      if ( $db );
      print OUT $_;
      }

   close IN;
   close OUT;
   return $dst;
   }

1;
