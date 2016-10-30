#
# $Id: $
#
use File::Temp;
use Test::More tests => 25;
use TPP::AWS::Credentials qw( credentials );
use FindBin;

BEGIN { 
      use_ok( 'TPP::AWS::S3Manager' );
      }


chdir( "$FindBin::Bin/data" );

SKIP: {
   my @keys = credentials();
   skip "skipping due to no AWS credentials", 24 if ( !$keys[0] && !$keys[1] );

   $keys[2] ||= 'us-west-2';
   $keys[3] = lc "tpp-$keys[0]-test";		# use a test bucket

   my $s3m = new_ok( TPP::AWS::S3Manager );
   ok( $s3m->open( @keys ), "open()" );
   
   note "Try upload/download of a simple file";
   my $fh    = File::Temp->new();
   my $fname = $fh->filename;
   print $fh "test file";
   $fh->flush();
   my $size = length "test file";
   
   my $key;
   ok( $key = $s3m->put( $fname ),	'put()' );
   $fh = undef;

   ok( $fname = $s3m->get( $key ),	'get()' );
   ok( -e $fname, 			'file exists' );
   my $c = do { local( @ARGV, $/ ) = $fname; <> };
   is( $c, 'test file',			'contents are correct' );

   my $pre  = (stat($fname))[9];
   ok( $s3m->get( $key ),		'get() again' );
   my $post = (stat($fname))[9];
   is( $pre, $post,			'file not overwritten' );

   is( $s3m->uploadCount(), 1,		'check upload count' );
   is( $s3m->uploadBytes(), $size,	'check upload size' );
   is( $s3m->downloadCount(), 1,	'check download count' );
   is( $s3m->downloadBytes(), $size,	'check download size' );
   
   note "Try a file that was already in S3 that we didn't know about";
   $s3m = new_ok( TPP::AWS::S3Manager );
   ok( $s3m->open( @keys ), "re open()" );
   ok( $key = $s3m->put( $fname ),	'put()' );
   
   unlink( $fname );
   
   note "Now try a file that auto gzips";
   $fh = File::Temp->new( SUFFIX => '.mzML' );
   $fname = $fh->filename;
   print $fh "test file";
   $fh->flush();
   
   $key = $s3m->put( $fname );
   ok( $key,				'put()' );
   $fh = undef;
   
   ok( $fname = $s3m->get( $key ),	'get()' );
   ok( -e $fname, 			'file exists' );
   $c = do { local( @ARGV, $/ ) = $fname; <> };
   is( $c, 'test file',			'contents are correct' );
   
   note "Try a file that auto gzips again to check filename";
   ok( $fname = $s3m->get( $key ),	'get()' );
   ok( -e $fname, 			'file exists' );
   $c = do { local( @ARGV, $/ ) = $fname; <> };
   is( $c, 'test file',			'contents are correct' );
   unlink( $fname );
   
   ok( $s3m->list(),			'list()' );
   ok( $s3m->delete(),			'delete()' );
   }
