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
# $Id: OMSSAService.pm 6285 2013-09-20 16:50:03Z slagelwa $
#
package TPP::AWS::OMSSAService;
use strict;
use warnings;

use Cwd qw( getcwd );
use File::Basename;
use File::Spec::Functions qw( rel2abs );
use File::Which qw( which );

use TPP::AWS;
use TPP::AWS::Logger qw( $log );

use base qw(TPP::AWS::SearchService);


#-- @GLOBALS ----------------------------------------------------------------#

our $REVISION = (q$Revision: 6285 $ =~ /(\d+)/g)[0] || '???'; 


#-- @PUBLIC -----------------------------------------------------------------#

#
# Constructor.
#
# @arg  Path to mzXML formatted spectrum file to search with OMSSA
# @arg  Path to parameters file to use for search
# @arg  Path to output directory (optional)
#
sub new
   {
   my $class = shift;
   my ( $input, $params, $odir ) = @_;

   my $self = $class->SUPER::new( @_);

   # Add blast database file(s)
   my @flags = $self->_readFlags( $self->{INPUT_FILES}->{PARAMS} );
   foreach ( @flags )
      {
      if ( /-d\s+(.*)\s*$/ )
         {
         my $db = rel2abs($1);
         my $i = '';
         foreach my $ext ( "", qw( .phr .pin .psd .psi .psq ) )
            {
            next unless ( -f "$db$ext" );
            $log->debug( "adding database $db$ext" );
            $self->{INPUT_FILES}->{"DB$i"} = "$db$ext";
            $i = 0 unless ( $i );
            $i++;
            }
         last;
         }
      }
   ( $self->{INPUT_FILES}->{DB} ) 
      or $log->logdie( "missing database files in omssa parameters file");
   ( $self->{INPUT_FILES}->{DB1} ) 
      or $log->logdie( "database doesn't appear to be NCBI formatted");

   # Add modification files (if any)
   foreach ( @flags )
      {
      $self->{INPUT_FILES}->{MXMOD}  = rel2abs($1) if ( /-mx\s+(.*)\s*/ );
      $self->{INPUT_FILES}->{MUXMOD} = rel2abs($1) if ( /-mux\s+(.*)\s*/ );
      }

   return $self;
   }


#-- @PROTECTED ----------------------------------------------------------------#

#
# Runs OMSSA search on given input.
#
sub run
   {
   my ( $self ) = @_;

   my $input  = $self->{WORK_FILES}->{INPUT};
   my $params = $self->{WORK_FILES}->{PARAMS};
   my $db     = $self->{WORK_FILES}->{DB};

   # Get flags and correct the file paths
   my @flags = $self->_readFlags( $params );
   foreach ( @flags )
      {
      $_ = "$1" . $db          if ( /^(\s*-d\s+)(.*)\s*/ );
      $_ = "$1" . basename($2) if ( /^(\s*-mux\s+)(.*)\s*/ );
      $_ = "$1" . basename($2) if ( /^(\s*-mx\s+)(.*)\s*/ );
      }

   my $output = $self->{OUTPUT_FILES};
   my $cmd;
   my $flags = join( ' ', @flags );

   my ( $root, $idir, $suffix ) = fileparse( $input, qr/\.[^.]*$/ );
   my $odir = $self->odir();
   $odir .= '/';

   unlink "$idir$root.mgf", "$root.pep.xml", "$root.pep.xml", "$root.omssa.log";

   my $MZXML2SEARCH = which('MzXML2Search')
      or $log->logdie( "Error MzXML2Search program not found in path" );
   my $OMSSACL = which('omssacl')
      or $log->logdie( "Error omssacl program not found in path" );
   my $INTERACTPARSER = which('InteractParser')
      or $log->logdie( "Error InteractParser program not found in path" );

   # Convert to mgf (is created next to input)
   $log->debug( "converting input to mgf for omssa search" );
   push @$output, "$root.omssa.log";
   $cmd = "$MZXML2SEARCH -mgf '$input' > '$root.omssa.log' 2>&1";
   $self->system( $cmd, "$root.omssa.log" );

   # Run capturing output
   $log->debug( "invoking omssa search on $root.mgf" );
   $cmd = "$OMSSACL -fm '$idir$root.mgf' -op '$odir$root.pepXML' $flags";
   $self->system( $cmd, "$root.omssa.log" );

   # Now run InteractParser to (potentially) fix the output
   # (Use FULL paths without any extra extensions)
   $log->debug( "invoking InteractParser on '$root.pepXML'" );
   $cmd = "$INTERACTPARSER '$odir$root.pep.xml' '$odir$root.pepXML' ";
   $cmd .= "-L0 -R9999 -a'$idir'";
   $self->system( $cmd, "$root.omssa.log" );

   # Now fix paths in the output
   push @$output, rel2abs("$root.pep.xml");
   $self->_updatePepXML( $output->[-1], qw(INPUT PARAMS DB) );
   }


#-- @PRIVATE -----------------------------------------------------------------#

#
# Load the omssa parameters.  These are expected to be in a simple text file
# containing  unix style flags, one per a line.  All text (comments) following
# the '#' character will be stripped.
#
# @arg   parameter filename
#
sub _readFlags
   {
   my ( $self, $params ) = @_;
   my @flags;
   open( PARAMS, $params ) or  die "can't open file $params, $!\n";
   while ( <PARAMS> )
      {
      chomp; s/#.*$//; s/\r$//;         # remove comments and endline
      next if ( /^$/ );                 # skip empty lines
      push @flags, $_;
      }
   close( PARAMS );
   
   return @flags;
   }

1;
