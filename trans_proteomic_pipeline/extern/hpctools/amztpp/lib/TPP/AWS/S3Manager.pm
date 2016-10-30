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
# $Id: S3Manager.pm 6285 2013-09-20 16:50:03Z slagelwa $
#
package TPP::AWS::S3Manager;
use strict;
use warnings;

use Class::InsideOut qw( :std );
use Cwd qw( abs_path );
use Digest::MD5::File qw( file_md5_base64 file_md5_hex );
use File::Basename;
use File::Path;
use File::Spec::Functions qw( catfile splitdir splitpath );
use File::Temp;
use IO::Compress::Gzip qw(gzip $GzipError);
use IO::Uncompress::Gunzip qw(gunzip $GunzipError);
use Net::Amazon::S3;
use TPP::AWS::Logger qw( $log );


#-- @GLOBALS ----------------------------------------------------------------#

our $REVISION = (q$Revision: 6285 $ =~ /(\d+)/g)[0] || '???'; 

use constant MAX_KEY_LEN => 1024;

# Use a exponential backoff of up ~5 minutes for retries
use constant RETRIES => (  1,  2,  4,  8, 16, 32, 
                          32, 32, 32, 32, 32, 32, 32, 32, 0 );


#-- @ATTRIBUTES -------------------------------------------------------------#

readonly s3         => my %s3;		# Net::Amazon::S3 object
readonly bucket	    => my %bucket;	# S3 bucket object

public   putList    => my %putList;	# hash of files uploaded (by path)
public   getList    => my %getList;	# hash of files downloaded (by path)

readonly uploadCount   => my %uploadCount;	# number of files uploaded
readonly uploadBytes   => my %uploadBytes;	# number of bytes uploaded
readonly uploadSecs    => my %uploadSecs;	# time spent uploading

readonly downloadCount => my %downloadCount;	# number of files downloaded
readonly downloadBytes => my %downloadBytes;	# number of bytes downloaded
readonly downloadSecs  => my %downloadSecs;	# time spent downloading


#-- @PUBLIC -----------------------------------------------------------------#

#
# Constructor.
#
sub new
   {
   my ( $class ) = @_;
   
   # Create object
   my $self = Class::InsideOut::new( $class );
   my $id   = id $self;
   
   $putList{$id} = {};
   $getList{$id} = {};
   
   $uploadCount{$id}   = 0;
   $uploadBytes{$id}   = 0;
   $uploadSecs{$id}    = 0;
   
   $downloadCount{$id} = 0;
   $downloadBytes{$id} = 0;
   $downloadSecs{$id}  = 0;
   
   return $self;
   }

#
# Open connection to S3
#
sub open
   {
   my ( $self, $accessKey, $secretKey, $region, $bucketName ) = @_;
   my $id = id $self;
   $region ||= 'us-west-2'; 
   
   if ( $^O =~ /MSWin/ )
      {
      # BIG HACK: Net::Amazon::S3 uses LWP::UserAgent which calls the method 
      # env_proxy(). This function does a "require Encode::Locale", which 
      # eventually does a runs "qx(chcp)". Problem is that when this is run as a
      # detached background process the qx() briefly opens a new console window.
      require Win32::Console;
      if ( !Win32::Console::Info() )
         {
         # ...trick it to skip a config section to keep a console from popping up
         $Encode::Locale::ENCODING_CONSOLE_IN = 'cp437';
         require Encode::Locale;
         }
      }
   
   # Initialize S3 interface
   my $s3 = $s3{$id} = Net::Amazon::S3->new( aws_access_key_id     => $accessKey,
                                             aws_secret_access_key => $secretKey,
                                             retry => 1,
                                           );
                                           
   # "Better get a bucket. I'm gonna throw up.""
   $bucketName ||= lc("TPP-$accessKey");
   my $resp = $s3->buckets() or $log->logdie( $s3->err . ": " . $s3->errstr );
   ( $bucket{$id} ) = grep { $_->{bucket} eq $bucketName } @{ $resp->{buckets} };
   if ( !$bucket{$id} )
      {
      $log->debug( "creating S3 bucket $bucketName" );
      $bucket{$id} = $s3->add_bucket( { bucket => $bucketName, 
#  FIXME: when S3 supports this location_constraint => $region
                                      } )
         or $log->logdie( $s3->err . ": " . $s3->errstr );
      }
      
   $log->debug( "S3Manager initialized (bucket $bucketName)" );
   return $self;
   }

#
# List some of the contents of the bucket
#
sub list
   {
   my ( $self, $max ) = @_;
   my $id = id $self;
   
   return [] unless $self->bucket();
   
   my @keys;
   my $list = $self->bucket()->list( $max ? { max_keys => $max } : {} )
      or $log->logdie( $self->s3()->err . ": " . $self->s3()->errstr );
   return( $list->{is_truncated}, $list->{keys} );
   }
   
#
# Convert a local filesystem path to a S3 key
#
sub path2key
   {
   my ( $self, $path ) = @_;
   my $key;

   # Using the absolute full pathname...
   $path = File::Spec->rel2abs( $path );
   my ( $v, $d, $f ) = splitpath( $path );
   $d =~ s#[\\/]$##;                             # ...without trailing '/'
   $key = join( '/', splitdir( $d ), $f );       # ...use unix seperators
   $key =~ s#^/## if ( !$v );                    # ...without a leading '/'
   $key = $v . $key;                             # ...include possible volume 
   $key .= '.gz' if ( $path =~ /(mzML|mzXML)$/ );# ...will gzip file

   ( length($key) <= MAX_KEY_LEN )
      or $log->logdie( "maximum length of S3 key exceeded\n" );

   return $key;
   }

#
# Convert an S3 key to a local filesystem path
#
sub key2path
   {
   my ( $self, $key ) = @_;
   my $path = catfile( File::Spec::Unix->splitdir( "/$key" ) );
   $path =~ s/^\\([A-Z]:)/$1/;                  # ...without a leading '\'      
   return $path;
   }

#
# Store given file in S3.  Gzip only certain files that TPP can handle
# as gzipped. If a file with the same key & checksum as one already 
# uploaded then skip it. If the checksum is different, replace it.
#
# @arg   filename to store
#
sub put
   {
   my ( $self, $path, $nomd5 ) = @_;
   my $id = id $self;
   
   $path = File::Spec->rel2abs( $path );        # use abs path
# TODO: FIX THIS
#   $path = Cwd::realpath( $path );              # rm links/".."
   my $key = $self->path2key( $path );
    
   $log->debug( "put file $path" );
   $log->debug( "put key  $key" );
   
   # Did we already upload it?
   if ( $putList{$id}->{$path} )
      {
      $log->debug( "put file already in S3, skipping" );
      return( $key );	
      }
      
   # Gzip the file?
   my $content = $path;
   if ( $path =~ /(mzML|mzXML)$/ )
      {
      $content = File::Temp->new();
      gzip( $path => "$content" ) 
         or $log->logdie( "put gzip failed: $GzipError\n" );
      }
   
   # Is something already there that we haven't uploaded?
   if ( my $etag = $self->keyExists( $key ) )
      {
      if ( file_md5_hex("$content") eq $etag )
         {
         $log->debug( "put file previously existed in S3, skipping" );
         $putList{$id}->{$path} = $etag;		# remember this
         return $key;
         }
      $log->warn( "put replacing file in S3 with different contents" );
      }
   
   my $md5 = file_md5_base64( "$content" ) . "==";      # pad to 24 bytes long
   
   $log->debug( "put uploading file to S3" );
   my $start = time();
   $self->_retryupload( $key, "$content", ($nomd5 ? undef : $md5), 5 );
   my $finish = time();
   $log->debug( "put finished upload" );
   
   $uploadCount{id $self} += 1;
   $uploadBytes{id $self} += (-s "$content");
   $uploadSecs{id $self}  += ($finish - $start);
   $putList{$id}->{$path} = $md5;
   
   return( $key );
   }

#
# Query S3 to see if a file exists
#
# @ret   The Etag checksum of the object
#
sub keyExists
   {
   my ( $self, $key ) = @_;
   
   my @retry = RETRIES;
   while ( @retry )
      {
      my $h;
      eval { $h = $self->bucket()->head_key( $key ) };
      if ( $@ )
         {
         $log->warn( "S3 file exists failed (".$#retry."x retries): $@" );
         sleep( shift @retry );
         }
      else
         {
         return( $h ? $h->{'etag'} : undef );
         }
      }

   $log->warn( "can't verify file for $key is in S3" );
   return();
   }

sub _retryupload
   {
   my ( $self, $key, $path, $md5, $retry ) = @_;
   
   my %hdrs;
   $hdrs{'Content-MD5'}      = $md5   if ( $md5 );
   $hdrs{'Content-Encoding'} = 'gzip' if ( $path =~ /(mzML.gz|mzXML.gz)$/ );

   my @retry = (defined $retry && $retry == 0 ) ? ( 0 ) : RETRIES;
   my $hex;
   while ( @retry )
      {
      eval { 
         $self->bucket()->add_key_filename( $key, $path, \%hdrs ) or
      	    die( $self->s3()->err . ", " . $self->s3()->errstr . "\n" );
      
         # file uploading is silently failing? so check
         my $h = $self->bucket()->head_key( $key ) or
      	    die( $self->s3()->err . ", " . $self->s3()->errstr . "\n" );
      	    
         $hex ||= file_md5_hex( $path );
      	 ( $h->{etag} eq $hex ) or 
      	    die( "head etag $h->{etag} doesn't match file checksum $hex\n" );
         };
      return if ( !$@ );
      	     
      $log->warn( "file upload failed (".$#retry."x retries): $@" );
      sleep( shift @retry );
      }
      
   $log->logdie( "file upload failed (max retries), giving up" ); 
   }
   
#
# Download the specified file from S3.  If a file with the same name was already
# retrieved then don't bother with the download.
#
# @arg   Key for file to download
#
sub get
   {
   my ( $self, $key ) = @_;
   my $id = id $self;
   
   $log->debug( "S3 get key $key" );
   
   my $path = catfile( File::Spec::Unix->splitdir( "/$key" ) );
   $path =~ s/^\\([A-Z]:)/$1/;
   
   # Already downloaded?
   if ( my $p = $getList{$id}->{$key} )
      {
      $log->debug( "get skipping file already downloaded" );
      return( $p );
      }
      
   # Make sure directory path exists
   eval { mkpath( dirname $path ) };
   $@ && $log->logdie( "can't create path $path: $@" );
      
   # Download file
   $log->debug( "get downloading file from S3" );
   my $start = time();
   $self->_retrydownload( $key, $path );
   my $finish = time();
   $log->debug( "get downloaded file from S3" );
      
   $downloadCount{id $self} += 1;
   $downloadBytes{id $self} += (-s $path ) || 0;
   $downloadSecs{id $self}  += ($finish - $start);
      
   # Uncompress downloaded file
   if ( $path =~ /(mzML.gz|mzXML.gz|pep.xml.gz)$/ )
      {
      $log->debug( "get gunzipping downloaded file $path" );
      (my $output = $path) =~ s/.gz$//;
      if ( gunzip( $path => $output ) )
         {
         unlink $path;
         $path = $output;
         }
      else
         {
         $log->logdie( "gunzip of $path failed: $GunzipError\n" );
         }
      }
      
   $getList{$id}->{$key} = $path;
   return( $path );
   }

sub _retrydownload
   {
   my ( $self, $key, $path, $retry ) = @_;
   
   my @retry = (defined $retry && $retry == 0 ) ? ( 0 ) : RETRIES;
   my $res;
   while ( @retry )
      {
      eval {
         $res = $self->bucket()->get_key_filename( $key, 'GET', $path )
            or die( $self->s3()->err . ", " . $self->s3()->errstr . "\n" );

         # Check checksums for network errors
         if ( $res->{etag} ne file_md5_hex( $path ) ) 
            {
            $log->debug( "File checksum = " . file_md5_hex( $path ) );
            $log->debug( "  S3 checksum = " . $res->{etag} );
            unlink( $path );
            die( "checksum for downloaded file $path did not match\n" );
            }
         }; 
      return unless ( $@ );

      $log->warn( "file download failed (".$#retry."x retries): $@" );
      sleep( shift @retry );
      }

   $log->logdie( "file $key download failed (max retries), giving up" ); 
   }

#
# Remove S3 objects and bucket associated with this manager.
#
sub delete
   {
   my ( $self ) = @_;
   my $id  = id $self;
   my $cnt = 0;
   
   # Delete contents
   my $list = $self->bucket()->list_all()
      or $log->logdie( $self->s3()->err . ": " . $self->s3()->errstr );
   foreach my $key ( @{$list->{keys}} )
      {
      ( $bucket{$id}->delete_key( $key->{key} ) ) 
         ? $cnt++
         : $log->warn( "deleting file in S3: " . $self->s3()->errstr );
      }
   $putList{$id} = {};
   $getList{$id} = {};

   # Remove bucket
#   $bucket{$id}->delete_bucket() 
#      or $log->warn( "deleting bucket in S3: " . $self->s3()->errstr );
#   $bucket{$id} = undef;
   return $cnt;
   }
  
1;
__END__
