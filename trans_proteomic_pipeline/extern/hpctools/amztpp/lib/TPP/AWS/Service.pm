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
# $Id: Service.pm 6285 2013-09-20 16:50:03Z slagelwa $
#
package TPP::AWS::Service;
use strict;
use warnings;

use File::pushd;
use File::Path qw( mkpath );
use File::Spec::Functions qw( rel2abs splitdir rootdir curdir catdir );
use File::Spec::Win32;
use File::Which qw(which);

use TPP::AWS;
use TPP::AWS::Logger qw( $log );
use TPP::AWS::S3Manager;


#-- @GLOBALS ------------------------------------------------------------------#

our $REVISION = (q$Revision: 6285 $ =~ /(\d+)/g)[0] || '???'; 

our $_num = 1;                  # Identifier for service
our $_servicelog = undef;       # HACK: Symlink system() logs for viewing 


#-- @PUBLIC -------------------------------------------------------------------#

#
# Constructor.
#
sub new
   {
   my ( $class ) = @_;

   my $this = {
      VERSION      => $TPP::VERSION,    # Our version
      NUM          => $_num++,          # Identifier for the service
      BUCKET       => undef,            # S3 bucket containing input/output
      ERRORS       => [],               # List of errors
      MSG_HANDLE   => undef,            # Handle for last SQS message

      ODIR_LIST => undef,               # Output directory path (list parts)
      WDIR_LIST => undef,               # Working directory path (list parts) 
      INPUT_FILES  => {},               # ID/path (on client) of input files
      OUTPUT_FILES => [],               # List (on client) of output files

      CLIENT => undef,                  # Hostname of client
      CLIENT_UPCOUNT   => 0,            # Num of files uploaded by client
      CLIENT_UPBYTES   => 0,            # Num of bytes uploaded by client
      CLIENT_UPSECS    => 0,            # Num of secs uploading
      CLIENT_DOWNCOUNT => 0,            # Num of files downloaded by client
      CLIENT_DOWNBYTES => 0,            # Num of bytes downloaded by client
      CLIENT_DOWNSECS  => 0,            # Num of secs downloading
      CLIENT_SUBMIT    => undef,        # time() service submitted
      CLIENT_RECEIVE   => undef,        # time() service received

      SERVER           => undef,        # Hostname of server
      SERVER_UPCOUNT   => 0,            # Num of files uploaded by server
      SERVER_UPBYTES   => 0,            # Num of bytes uploaded by server
      SERVER_UPSECS    => 0,            # Num of secs uploading
      SERVER_DOWNCOUNT => 0,            # Num of files downloaded by server
      SERVER_DOWNBYTES => 0,            # Num of bytes downloaded by server
      SERVER_DOWNSECS  => 0,            # Num of secs downloading
      SERVER_RECEIVE   => undef,        # time() service received
      SERVER_SUBMIT    => undef,        # time() service submitted
      SERVER_WTIME     => undef,        # wall time of server job
      };

   bless( $this, $class );
   return $this;
   }


#-- @ABSTRACT -----------------------------------------------------------------#

#
# Run the service. Abstract method to be overridden by the subclass
#
sub run
   {
   die "abstract method called";
   }


#-- @PROTECTED ----------------------------------------------------------------#

#
# Mounts working directory.  Linux EC2 instances have a fixed 10GB root
# partition and one or more instance stores of various sizes depending on the
# instance type.  They all mount one store as /mnt on the sytem. This routine
# binds the filesystem to the root directory of the input/output paths so we
# use the instance store space as opposed to filling up the root partition.
#
sub mount
   {
   $log->debug( "Mounting store for mapping file paths" );
   my ( $self ) = @_;

   my %mnts;
   foreach my $k ( keys %{ $self->{INPUT_KEYS} } )
      {
      my ( $root ) = splitdir( $self->{INPUT_KEYS}->{$k} );
      next if ( $mnts{$root} || -d $root );
      $log->debug( "Binding /mnt/$root to /$root" );
      mkpath( "/mnt/$root" );
      mkpath( "/$root" ); 
      if ( CORE::system("mount --bind /mnt/$root /$root") )
         {
         $log->error( "failed to mount directory: $@" );
         }
      else
         {
         $mnts{$root} = 1;
         }
      }

   return;
   }

#
# Return the output directory in path format specific to local OS
# 
sub odir
   {
   my $self = shift;
   my @odir = @{$self->{ODIR_LIST}};
   $odir[0] ||= (splitdir( rel2abs(curdir()) ))[0];      # current dir volume
   unshift @odir, rootdir() unless ( $^O =~ /MSWin/ );
   return catdir( @odir );
   }

#
# Return the working directory in path format specific to local OS
# 
sub wdir
   {
   my $self = shift;
   my @wdir = @{$self->{WDIR_LIST}};
   unshift @wdir, rootdir() unless ( $^O =~ /MSWin/ );
   return catdir( @wdir );
   }

#
# Change to the working directory
#
sub chwdir 
   {
   my $wdir = $_[0]->wdir();
   $log->debug( "change to directory $wdir" );
   mkpath( $wdir );
   return pushd $wdir or $log->logdie( "Error: can't chdir to $wdir: $!\n" );
   }

#
# Change to the output directory
#
sub chodir
   {
   my $odir = $_[0]->odir();
   $log->debug( "change to directory $odir" );
   mkpath( $odir );
   return pushd $odir or $log->logdie( "Error: can't chdir to $odir: $!\n" );
   }

#
# Call system() and throw an exception if something goes wrong.  Always use
# this function instead of perl's system in a Service to ensure that any 
# signals sent to the child process are raised in the parent process.
#
# @arg  Optional file to append STDOUT/STDERR 
#
sub system
   {
   my ( $self, $cmd, $logfile ) = @_;

   if ( $logfile ) 
      {
      $cmd .= " >> '$logfile' 2>&1";
      if ( $self->{SERVICE_LOG} && !-f $self->{SERVICE_LOG} )
         {
         symlink( rel2abs($logfile), $self->{SERVICE_LOG} )
            or $log->warn( "unable to create symbolic link for service log" );
         }
      }

   $log->debug( "system( $cmd )" );
   system( $cmd );
   if ( $? == -1 ) 
      {
      $log->logdie( "failed to execute system: $!" );
      }
   elsif ( $? & 127 ) 
      {
      $log->logdie( sprintf("child died with signal %d", $? & 127) );
      }
   elsif ( $? >> 8 )
      {
      $log->logdie( sprintf("child exited with value %d", $? >> 8) );
      }
   }

#
# Upload the set of input files needed to perform the service
#
# @arg  Reference to the S3 manager to use to upload
# @ret  Number of input files uploaded
#
sub uploadInput
   {
   my ( $self, $s3m ) = @_;

   $log->debug( "uploading service input files" );

   my $cnt   = $s3m->uploadCount();
   my $bytes = $s3m->uploadBytes();
   my $secs  = $s3m->uploadSecs();

   foreach ( sort keys %{ $self->{INPUT_FILES} } )
      {
      my $file = $self->{INPUT_FILES}->{$_};
      if ( ! -f $file )
         {
         $log->error( "missing input file $file" );
         next;
         }
      $self->{INPUT_KEYS}->{$_} = $s3m->put( $file );
      }

   $self->{CLIENT_UPCOUNT} = $s3m->uploadCount() - $cnt;
   $self->{CLIENT_UPBYTES} = $s3m->uploadBytes() - $bytes;
   $self->{CLIENT_UPSECS}  = $s3m->uploadSecs() - $secs;
   return( scalar( keys %{$self->{INPUT_KEYS}} ) );
   }

#
# Download service's input files
#
# @arg   reference to S3Manager object
#
sub downloadInput
   {
   my ( $self, $s3m ) = @_;

   $log->debug( "downloading service input files" );

   my $cnt   = $s3m->downloadCount();
   my $bytes = $s3m->downloadBytes();
   my $secs  = $s3m->downloadSecs();

   $self->{WORK_FILES} = {};
   foreach ( sort { $a cmp $b } keys %{ $self->{INPUT_KEYS} } )
      {
      my $s3key = $self->{INPUT_KEYS}->{$_};
      $self->{WORK_FILES}->{$_} = rel2abs( $s3m->get( $s3key ) );
      }

   $self->{SERVER_DOWNCOUNT} += ($s3m->downloadCount() - $cnt);
   $self->{SERVER_DOWNBYTES} += ($s3m->downloadBytes() - $bytes);
   $self->{SERVER_DOWNSECS}  += ($s3m->downloadSecs() - $secs);

   return( $self->{WORK_FILES} );
   }

#
# Upload resulting output files from the service
#
# @arg   reference to S3Manager object
# @arg   list of paths to files to upload
#
sub uploadOutput
   {
   my ( $self, $s3m ) = @_;

   $log->debug( "uploading service output files" );

   my $cnt   = $s3m->uploadCount();
   my $bytes = $s3m->uploadBytes();
   my $secs  = $s3m->uploadSecs();

   foreach my $out ( @{$self->{OUTPUT_FILES}} )
      {
      if ( !-f $out )
         {
         $log->error( "missing output file $out" );
         next;
         }
      push @{$self->{OUTPUT_KEYS}}, $s3m->put( $out );
      }

   $self->{SERVER_UPCOUNT} = $s3m->uploadCount() - $cnt;
   $self->{SERVER_UPBYTES} = $s3m->uploadBytes() - $bytes;
   $self->{SERVER_UPSECS}  = $s3m->uploadSecs() - $secs;
   }

#
# Download resulting output files from the service
#
sub downloadOutput
   {
   my ( $self, $s3m ) = @_;

   $log->debug( "downloading service output files" );

   my $cnt   = $s3m->downloadCount();
   my $bytes = $s3m->downloadBytes();
   my $secs  = $s3m->downloadSecs();

   $s3m->get($_) foreach ( @{$self->{OUTPUT_KEYS}} );

   $self->{CLIENT_DOWNCOUNT} += ($s3m->downloadCount() - $cnt);
   $self->{CLIENT_DOWNBYTES} += ($s3m->downloadBytes() - $bytes);
   $self->{CLIENT_DOWNSECS}  += ($s3m->downloadSecs() - $secs);
   }

#
# Startup the service by calling it's run method()
#
# @ret  True if service was interrupted and should be considered for retrying
#
sub start
   {
   my ( $self, $s3m, $sqsm, $servicelog ) = @_;

   $log->debug( ref($self) . "service starting" );
   $self->{SERVICE_LOG} = $servicelog;

   my $cwd;                             # preserves cwd for scope of routine
   eval
      {
      $cwd = $self->chwdir();
      $self->downloadInput( $s3m );
      $self->run( $self->{WORK_FILES}, $self->{OUTPUT_FILES} );
      };

   if ( my $e = $@ )                    # caught an exception?
      {
      if ( $e =~ /download failed.*max retries/ ||
           $e =~ /child died with signal 15/ )
          {
          $log->error( "service $e" );
          return $self->{RETRY}--;
          }
      else
          {
          $log->fatal( "service $e" );
          push @{ $self->{ERRORS} }, $e;
          }
      }

   eval { $self->uploadOutput( $s3m ) };
   if ( my $e = $@ )                    # couldn't upload files?
      {
      $log->fatal( $e );
      push @{ $self->{ERRORS} }, $e;
      }

   $log->debug( ref($self) . " service finished" );
   return 0;
   }

1;
