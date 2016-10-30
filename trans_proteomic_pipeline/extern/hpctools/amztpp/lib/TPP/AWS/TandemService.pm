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
# $Id: TandemService.pm 6285 2013-09-20 16:50:03Z slagelwa $
#
package TPP::AWS::TandemService;
use strict;
use warnings;

use File::Basename;
use File::Spec::Functions qw( rel2abs catfile );
use File::Which qw(which);
use XML::Simple;

use TPP::AWS;
use TPP::AWS::S3Manager;
use TPP::AWS::Logger qw( $log );

use base qw(TPP::AWS::SearchService);


#-- @GLOBALS ----------------------------------------------------------------#

our $REVISION = (q$Revision: 6285 $ =~ /(\d+)/g)[0] || '???'; 


#-- @PUBLIC -----------------------------------------------------------------#

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

   $self->{INPUT_FILES}->{INPUT}  = rel2abs($input);
   $self->{INPUT_FILES}->{PARAMS} = rel2abs($params); 

   # Store additional information about tandem search...
   my $p = $self->_readParams( $self->{INPUT_FILES}->{PARAMS} );

   $self->{TAXON}    = $p->{TAXON};             # ...taxon label
   $self->{THREADS}  = $p->{THREADS};           # ...# of threads to use

   my $in = $self->{INPUT_FILES};
   $in->{DEFAULTS} = $p->{DEFAULTS};            # ...path defaults file
   $in->{TAXONOMY} = $p->{TAXONOMY};            # ...path to taxonomy file
   $in->{DB}       = shift @{$p->{DBS}};        # ...path to fasta db file
   for ( my $i = 1; $p->{DBS}->[$i]; $i++ )     # ...add additional db files
      {
      $in->{"DB$i"} = $p->{DBS}->[$i];
      }

   return $self;
   }

#
# Runs X!Tandem search on given input.
#
sub run 
   {
   my ( $self ) = @_;

   my $input    = $self->{WORK_FILES}->{INPUT};
   my $params   = $self->{WORK_FILES}->{PARAMS};
   my $defaults = $self->{WORK_FILES}->{DEFAULTS};
   my $taxonomy = $self->{WORK_FILES}->{TAXONOMY};
   my @dbs;
   for ( keys %{$self->{WORK_FILES}} )
      {
      push @dbs, $self->{WORK_FILES}->{$_} if ( /^DB[0-9]{0,}$/ );
      }

   my ( $root, $dir, $suffix ) = fileparse( $input, qr/\.[^.]*$/ );

   my $output = $self->{OUTPUT_FILES};
   unlink "$root.tandem", "$root.pep.xml", "$root.tandem.log", 
      "$root.output_sequences";

   # Search setup
   $self->_makeTaxonomy( $taxonomy, $self->{TAXON}, \@dbs );
   $self->_copyParams( $params, "$root.tandem.params", $input, $defaults, $taxonomy );
   push @$output, "$root.tandem.params";

   # Run tandem capturing output
   $log->debug( "invoking tandem on input file" );
   my $tandem = which('tandem.exe');
   $tandem ||= which('tandem');
   $tandem || $log->logdie( "tandem program not found in path" );

   my $cmd = "$tandem $root.tandem.params";
   push @$output, "$root.tandem.log";
   $self->system( $cmd, "$root.tandem.log" );
   push @$output, "$root.tandem";

   # Convert tandem output to pep.xml
   $log->debug( "invoking Tandem2XML on output file" );
   my $tandem2xml = which('Tandem2XML')
      or $log->logdie( "tandem2xml program not found in path" );
   $cmd = "$tandem2xml $root.tandem $root.pep.xml";
   $self->system( $cmd, "$root.tandem.log" );
   push @$output, rel2abs("$root.pep.xml");

   $self->_updatePepXML( $output->[-1], qw(INPUT PARAMS DB DEFAULTS TAXONOMY) );
   }


#-- @PRIVATE -----------------------------------------------------------------#

#
# Parse the contents of a tandem parameters file and return a list of values 
# of important parameters.
#
# Returns a reference to a hash containing:
#
#   DEFAULTS  => The absolute path to tandem defaults file (default_input.xml)
#   TAXON     => The taxon label
#   TAXONOMY  => The absolute path to taxonomy file
#   THREADS   => The number of threads 
#   DBS       => Absolute path(s) to database file(s)
#
sub _readParams
   {
   my ( $self, $paramsFile ) = @_;    # @arg filename of parameters file

   $log->logdie( "missing parameter file $paramsFile" ) 
      unless ( $paramsFile && -f $paramsFile );

   my %setup = ( DEFAULTS => undef,
                 TAXON    => undef,
                 TAXONOMY => undef,
                 THREADS  => undef,
                 DBS      => [],
               );

   # Read configuration from parameters file
   my $defaultsRe  = qr~list path,\s*default parameters~i;
   my $taxonRe     = qr~protein,\s*taxon~i;
   my $taxonfileRe = qr~list path,\s*taxonomy information~i;
   my $threadsRe   = qr~spectrum,\s*threads~i;
   eval {
      my $xml = XMLin($paramsFile);
      foreach ( @{ $xml->{note} } )
         {
         next if (ref($_) ne 'HASH');
         $setup{DEFAULTS} = $_->{content} if ($_->{label} =~ /$defaultsRe/);
         $setup{TAXON}    = $_->{content} if ($_->{label} =~ /$taxonRe/);
         $setup{TAXONOMY} = $_->{content} if ($_->{label} =~ /$taxonfileRe/);
         $setup{THREADS}  = $_->{content} if ($_->{label} =~ /$threadsRe/);
         }
   };
   if ($@)
      {
      $log->logdie( "can't process tandem parameter file $paramsFile,\n$@" );
      }
   $setup{DEFAULTS} = rel2abs( $setup{DEFAULTS} );
   $setup{TAXON}    =~ s/^\s+|\s+$//g;
   $setup{TAXONOMY} = rel2abs( $setup{TAXONOMY} );

   $log->debug( "tandem DEFAULTS = '$setup{DEFAULTS}'" );
   $log->debug( "tandem TAXON    = '$setup{TAXON}'" );
   $log->debug( "tandem TAXONOMY = '$setup{TAXONOMY}'" );

   # Read taxon info file for location of additional data files
   eval {
      my $taxxml = XMLin($setup{TAXONOMY});
      my $taxons = $taxxml->{taxon};
      $taxons = [$taxons] unless ref($taxons) eq 'ARRAY';
      foreach ( @{$taxons} )
         {
         next if ( $_->{label} ne $setup{TAXON} );
         push @{$setup{DBS}}, rel2abs( $_->{file}->{URL} ); 
         }
   };
   if ( $@ )
      {
      $log->logdie( "can't process tandem taxonomy file $setup{TAXONOMY}:\n$@" );
      }
   if ( !@{ $setup{DBS} } ) 
      {
      $log->logdie( "unable to determine fasta files for taxonomy $setup{TAXONOMY}" );
      }

   $log->debug( "tandem DBS      = @{ $setup{DBS} }" );
   return \%setup;
   }

#
# Makes a tandem params file specific for the given input file by copying
# the contents of a default template file and setting the appropriate tandem
# parameters in the new file.
#
# The parameters in the copy are set to match the input file name as follows:
#
#    Path                               Description
#    ---------------------------------- --------------------------------------
#    <basename>.mzXML                   input file name
#    <basename>.tandem                  tandem file name
#    <basename>.output_sequences        output sequences file name
#    <defaults>                         default parameters file (optional)
#    <taxoninfo>                        taxonomy info file (optional)
#
#  Where basename is the basename of the mzXML file.
#
#  @arg   source (template) parameter filename to copy
#  @arg   destination parameter filename
#  @arg   replace tandem input filename
#  @arg   replace tandem defaults filename (optional)
#  @arg   replace tandem taxononomy filename (optional)
#  @ret   returns the path to the new file created
#
sub _copyParams
   {
   my ( $self, $src, $dst, $input, $defaults, $taxonomy ) = @_;

   $log->debug( "copying tandem parameters file $src to $dst" );

   my ( $root, $dir, $s ) = fileparse( $input, qr/\.([^.]*|[^.]*.gz)$/ );
   $dir = "" if ( $dir eq "./");

   open( IN,  "< $src" ) or $log->logdie( "can't open $src: $!" );
   open( OUT, "> $dst" ) or $log->logdie( "can't open $dst: $!" );

   # Regexes to replace parameter attributes
   my $inputRe =
      qr~(<note type="input"\s+label="spectrum,\s*path">).*?(</note>)~im;
   my $taxonomyRe =
      qr~(<note type="input"\s+label="list path, taxonomy information">).*?(</note>)~im;
   my $defaultsRe =
      qr~(<note type="input"\s+label="list path, default parameters">).*?(</note>)~im;
   my $tandemRe = 
      qr~(<note type="input"\s+label="output,\s*path">).*?(</note>)~im;
   my $sequencesRe =
      qr~(<note type="input"\s+label="output,\s*sequence path">).*?(</note>)~im;

   # Real paths (use full paths else tandem can crash)
   my $tandem    = catfile( $self->odir(), "$root.tandem" );
   my $sequences = catfile( $self->odir(), "$root.output_sequences" );

   $log->debug( "    input = $input" );
   $log->debug( "   tandem = $tandem" );
   $log->debug( "sequences = $sequences" );
   
   while ( <IN> )
      {
      s/$taxonomyRe/$1${taxonomy}$2/ if ( $taxonomy );
      s/$defaultsRe/$1${defaults}$2/ if ( $defaults );

      s/$inputRe/$1$input$2/;
      s/$tandemRe/$1$tandem$2/;
      s/$sequencesRe/$1$sequences$2/;
      print OUT $_;
      }

   close IN;
   close OUT;
   return $dst;
   }

#
# Write a new taxonomy file
#
# @arg   Name of taxonomy file
# @arg   Label for taxonomy
# @arg   List of fasta database files
#
sub _makeTaxonomy
   {
   my ( $self, $filename, $label, $dbs ) = @_;
   open( OUT, "> $filename" ) or $log->logdie( "can't open $filename: $!" );
   print OUT qq#<?xml version="1.0"?>\n#;
   print OUT qq#<bioml label="x! taxon-to-file matching list">\n#;
   print OUT qq#<taxon label="$label">\n#;
   foreach (@$dbs)
      {
      print OUT qq#<file format="peptide" URL="$_" />\n#;
      }
   print OUT qq#</taxon>\n#;
   print OUT qq#</bioml>\n#;
   close OUT;
   return $filename;
   }

1;
