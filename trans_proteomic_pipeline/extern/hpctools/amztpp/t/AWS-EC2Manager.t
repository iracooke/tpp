#
# $Id: $
# $Name:  $
#
use Test::More tests => 15;
use TPP::AWS::Credentials qw( credentials );
use TPP::AWS::Logger qw( $log );

BEGIN { 
      use_ok( 'TPP::AWS::EC2Manager' );
      }
      

#$log->{level} = 1;
#$log->{fmt}   = '[%d] (%p) %m';

SKIP: {
   ( $accessKey, $secretKey, $ec2region ) = credentials();
   skip "skipping due to no AWS credentials", 14 if ( !$accessKey && !$secretKey );
   
   note "Using us-east-1 region (default)";
   $ec2region = 'us-east-1';
   my $ec2m = new_ok( TPP::AWS::EC2Manager => [ $accessKey, $secretKey, $ec2region ] );
   ok( $ec2m->getImages(), 		'At least one image is available' );
   
   note "Using us-west-2 region";
   $ec2region = "us-west-2";
   $ec2m = new_ok( TPP::AWS::EC2Manager => [ $accessKey, $secretKey, $ec2region ] );
   ok( $ec2m->getImages(), 		'At least one image is available' );
   ok( my $i = $ec2m->start( 1, undef, 'm1.large' ),	'start() one instance' );
   my ( $s, $r ) = $ec2m->counts();
   ok( $s == 1 || $r == 1, 	 	'Is one started?' );
   ok( $ec2m->terminate(),		'terminate() one instance' );
   
   note "Request one spot instance";
   ok( $i = $ec2m->start( 1, undef, 'm1.large', undef, undef, '0.01' ),
       'request one spot instance' );
   ( $s, $r ) = $ec2m->counts();
   ok( $s == 1,		 	 	'is a spot instance requested?' );
   ok( $ec2m->cancel(1),		'cancel() instances' );
   ( $s, $r ) = $ec2m->counts();
   ok( $s == 0,		 	 	'did it get cancelled?' );

   note "Using ap-southeast-1 region";
   $ec2region = "ap-southeast-1";
   $ec2m = new_ok( TPP::AWS::EC2Manager => [ $accessKey, $secretKey, $ec2region ] );
   eval { $ec2m->getImages() };
   like( $@, qr/Unable to locate any/,   'No image should be available' );
   
   note "Using bogus region";
   $ec2region = "ap-mars-1";
   eval { $ec2m = TPP::AWS::EC2Manager->new( $accessKey, $secretKey, $ec2region ) };
   like( $@, qr/Can't connect/,         'Got error on bogus region' );
   }
