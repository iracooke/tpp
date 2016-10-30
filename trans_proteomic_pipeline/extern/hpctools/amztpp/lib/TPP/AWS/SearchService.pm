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
# $Id: SearchService.pm 6285 2013-09-20 16:50:03Z slagelwa $
#
package TPP::AWS::SearchService;
use strict;
use warnings;

use Cwd qw( abs_path cwd );
use File::Basename;
use File::Copy;
use File::Path qw( mkpath );
use File::pushd;
use File::Spec::Functions qw( rel2abs splitdir catdir );

use TPP::AWS;
use TPP::AWS::Logger qw( $log );

use base qw(TPP::AWS::Service);


#-- @GLOBALS ----------------------------------------------------------------#

our $REVISION = (q$Revision: 6285 $ =~ /(\d+)/g)[0] || '???'; 


#-- @PUBLIC -----------------------------------------------------------------#

#
# Constructor.
#
sub new
   {
   my ( $class, $input, $params, $odir ) = @_;

   my $fullinput = rel2abs( $input );
   my ( $root, $dir, $suffix ) = fileparse( $fullinput, qr/\.[^.]*$/ );

   my $self = $class->SUPER::new( $class, @_ );
   $self->{NAME} = $fullinput;                  # Name of search (input file)
   $self->{ODIR_LIST} = [ splitdir($odir || $dir) ];
   $self->{WDIR_LIST} = $self->{ODIR_LIST};

   $self->{INPUT_FILES} = {                     # Local paths to the input files
      INPUT  => $fullinput,                     # S3 key to input file
      PARAMS => rel2abs( $params ),             # S3 key to parameters file
      DB     => undef,                          # S3 key to database file
      };
   $self->{INPUT_KEYS}   = {};                  # S3 keys for the input files
   $self->{OUTPUT_FILES} = [];                  # EC2 paths to output files
   $self->{OUTPUT_KEYS}  = [];                  # S3 keys for the output files

   $self->{RETRY} = 9;                          # Num of times to requeue search

   return $self;
   }

#
# Change to the working directory
#
sub chwdir 
   {
   my $wdir = $_[0]->wdir();
   $log->debug( "change to directory $wdir" );
   mkpath( $wdir );
   return pushd $wdir
      or $log->logdie( "Error: can't chdir to $wdir: $!\n" );
   }

#
# FIXME: remove this
# Run on the input file using the parameters and database
#
sub ___run
   {
   my ( $self, $s3m, $servicelog, $sqsm ) = @_;

   $log->debug( ref($self) . " run started" );

   my $cwd;                             # preserve's cwd when changing
   eval 
      {
      $cwd = $self->chwdir();
      $self->downloadInput( $s3m );
      $self->_search( $servicelog );
      };

   if ( my $e = $@ )                    # something go wrong?
      {
      if ( $e =~ /download failed.*max retries/ && $self->{RETRY} )
          {
          $log->warn( "download failed, will requeue service for another try" );
          $self->{RETRY}--;
          return 1;
          }
      elsif ( $e =~ /child died with signal 15/ )
          {
          $log->warn( "service received TERM signal, requeue for another try" );
          return 1;
          }
      else
          {
          push @{ $self->{ERRORS} }, $e;
          $log->fatal( $e );
          }
      }

   eval { $self->uploadOutput( $s3m ) };
   if ( my $e = $@ )                    # anything go wrong?
      {
      push @{ $self->{ERRORS} }, $e;
      $log->fatal( $e );
      }

   $log->debug( ref($self) . " run finished" );
   return 0;
   }


#-- @PROTECTED ---------------------------------------------------------------#

#
# FIXME: for the time being manually fix the paths in the pep.xml file on
# the server as updateAllPaths.pl is broke
#
sub _updatePepXML
   {
   my ( $self, $pepxml, @files ) = @_;

   $log->debug( "updating paths in pep.xml file $pepxml" );

   # Client input file basename
   my $cliIn = $self->{INPUT_FILES}->{INPUT};
   my $srvIn = $self->{WORK_FILES}->{INPUT};
   $cliIn =~ s/\.gz$//;                 # wack off extensions
   $cliIn =~ s/\.(.*)?$//;
   $srvIn =~ s/\.gz$//;                 # wack off extensions
   $srvIn =~ s/\.(.*)?$//;

   # Client output pepxml basename
   my $srvOut = $pepxml;
   my $cliOut = $pepxml;
   if ( $self->{ODIR_LIST}->[0] )       # windows client?
      {
      $cliOut = File::Spec::Win32::catdir( splitdir( $cliOut ) );
      }
   $cliOut =~ s/\.pep\.xml$//;          # wack off extensions
   $srvOut =~ s/\.pep\.xml$//;

   my $tmp = File::Temp->new();
   open( TMP, ">$tmp" )    or $log->logdie( "can't open file $tmp: $!" );
   open( PEP, "<$pepxml" ) or $log->logdie("can't open file $pepxml: $!");
   while ( my $line = <PEP> )
      {
      # Fix any additional parm/db/taxonomy/etc file paths...
FILE: foreach ( @files )
         {
         my $srv = $self->{WORK_FILES}->{$_}  or next FILE; 
         my $cli = $self->{INPUT_FILES}->{$_} or next FILE;
         if ( $line =~ s/\Q$srv/$cli/g ) {
            $log->debug( "fixed $_" );
            $log->debug( "  srv $srv" );
            $log->debug( "  cli $cli" );
            $log->debug( " line $line" );
            }
         }
      if ( $line =~ s/\Q$srvOut/$cliOut/g ) {
         $log->debug( "fixed output path: $srvOut $cliOut" );
      }
      if ( $line =~ s/\Q$srvIn/$cliIn/g ) {
         $log->debug( "fixed input path: $srvIn $cliIn" );
      }
      print TMP $line;	
      }
   close( TMP );
   close( PEP );
   copy( $tmp, $pepxml ) or $log->logdie( "can't move $tmp to $pepxml: $!");
   }

#
# !!! DEPRECATED !!!
# Update (repair) mzXML/pep.xml paths found in the given file.
#
# @arg   input filename to update
# @arg   new location of database file (optional)
#
sub _updatePaths
   {
   my ( $self, $filename, $database ) = @_;

   # Check input/paths
   $database = abs_path( $database );
   $log->warn( "missing $filename " )     if ( !-f $filename );
   $log->warn( "missing $database file" ) if ( $database && !-f $database );
   if ( !-f $filename || ($database && !-f $database) )
      {
      $log->error( "unable to update paths in $filename" );
      return 0 
      }

   # Build command
   my $UPDATEPATHS = which('updateAllPaths.pl')
      or $log->logdie( "Error updateAllPaths.pl program not found in path" );
   my $db  = ( $database ? "--database \"$database\"" : '' );
   my $cmd = "$UPDATEPATHS $db --quiet $filename > /dev/null";

   # Believe it or not we need to run it twice due to a bug in updateAllPaths
   # skipping the update of the database if the paths are similar/same
   $self->system( $cmd );
   $self->system( $cmd );

   return 1;
   }

1;
