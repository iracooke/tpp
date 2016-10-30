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
# $Id: RNASeqPDBService.pm 6285 2013-09-20 16:50:03Z slagelwa $
#
package TPP::AWS::RNASeqPDBService;
use strict;
use warnings;

use File::Basename;
use File::Find;
use File::Spec::Functions qw( rel2abs splitdir catfile );
use File::Which qw(which);

use TPP::AWS;
use TPP::AWS::Logger qw( $log );

use base qw(TPP::AWS::Service);


#-- @GLOBALS ----------------------------------------------------------------#

our $REVISION = (q$Revision: 6285 $ =~ /(\d+)/g)[0] || '???'; 


#-- @PUBLIC -----------------------------------------------------------------#

#
# Constructor.
#
# @arg   Path to RNA-seq fastq or sar file
# @arg   Path to parameters file to use
#
sub new
   {
   my ( $class, $input, $params, $odir ) = @_;
   my $self = $class->SUPER::new(@_);

   my $fullinput = rel2abs( $input );
   my ( $root, $dir, $suffix ) = fileparse( $fullinput, qr/\.[^.]*$/ );

   $self->{NAME} = $fullinput;                  # Name of service (input file)

   $self->{ODIR_LIST} = [ splitdir($odir || $dir) ];
   $self->{WDIR_LIST} = $self->{ODIR_LIST};

   # Assign input files
   $self->{INPUT_FILES}->{RNASEQ} = rel2abs($input);
   $self->{INPUT_FILES}->{PARAMS} = rel2abs($params) if ( $params ); 

   return $self;
   }

#
# Runs the RNA-Seq script to generate a suitable database for peptide
# identification.
#
# @arg   Reference to list of input files (server side)
# @arg   Reference to list of output files (server side)
#
sub run
   {
   my ( $self ) = @_;

   my $input  = $self->{WORK_FILES}->{RNASEQ};
   my $params = $self->{WORK_FILES}->{PARAMS};
   my $output = $self->{OUTPUT_FILES};

   my ( $root, $dir, $suffix ) = fileparse( $input, qr/\.[^.]*$/ );

   # Run rnaseqpdb script capturing output
   $log->debug( "invoking rnaseqpdb on input file" );
   my $prog = which('rnaseqpdb.pl');
   $prog || $log->logdie( "rnaseq program not found in path" );

   my $cmd = "$prog -vv";
   $cmd .= " -p $params" if ( $params ); 
   $cmd .= " $input";
   push @$output, "$root.rnaseqpdb.log";
   $self->system( $cmd, "$root.rnaseqpdb.log" );

   # Recursively add files in output directories
   _addFiles( $output, catfile( $self->odir(), "$root-fastqc" ) );
   _addFiles( $output, catfile( $self->odir(), "$root-tophat" ) );
   _addFiles( $output, catfile( $self->odir(), "$root-dbase" ) );
   }


#-- @PRIVATE -----------------------------------------------------------------#

#
# Recursively descent a directory adding each file it finds to a list
#
sub _addFiles
   {
   my ( $list, $dir ) = @_;
 
   find( sub { -f && push @$list, rel2abs($_); }, $dir );
   }

1;
