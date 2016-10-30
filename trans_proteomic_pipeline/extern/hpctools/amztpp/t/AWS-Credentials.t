#
# $Id: $
# $Name:  $
#
use Test::More tests => 18;
use FindBin;
use File::HomeDir;
use File::Spec::Functions qw( catfile );
use File::Temp qw( tempdir );

BEGIN { 
      use_ok( 'TPP::AWS::Credentials' );
      }

our @k;
our $home = "/dev/null/";

# Mask any existing settings
{
   no warnings;	
   $ENV{AWS_CREDENTIAL_FILE} = undef;
   $ENV{EC2_ACCESS_KEY} = undef;
   $ENV{EC2_SECRET_KEY} = undef;
   $ENV{EC2_REGION}     = undef;
   sub File::HomeDir::my_data { $home }
}

note "credentials() not set at all";
( @k ) = TPP::AWS::Credentials::credentials();
is( $k[0], undef,               "No access key" );
is( $k[1], undef,               "No secret key" );
is( $k[2], undef,               "No region key" );

note "write a temporary credentials file in tmp";
$home = tempdir( CLEANUP => 1 );
ok( TPP::AWS::Credentials::write( 'testfilekey', 'testfilepass' ),
    "wrote credentials file" );

note "credentials() trying using AWS_CREDENTIAL_FILE";
{
   $ENV{AWS_CREDENTIAL_FILE} = catfile( $home, '.awssecret' );
   local $home = undef;
   ( @k ) = TPP::AWS::Credentials::credentials();
   is( $k[0], 'testfilekey',	"Got access key credentials" );
   is( $k[1], 'testfilepass',      "Got secret key credentials" );
   is( $k[2], undef,               "Region should be empty ");
   delete $ENV{AWS_CREDENTIAL_FILE};
}

note "credentials() trying from pseudo home credentials file";
( @k ) = TPP::AWS::Credentials::credentials();
is( $k[0], 'testfilekey',	"Got access key credentials" );
is( $k[1], 'testfilepass',      "Got secret key credentials" );
is( $k[2], undef,               "Region should be empty ");

note "credentials() trying from environment, overriding file";
$ENV{EC2_ACCESS_KEY} = 'testenvkey';
$ENV{EC2_SECRET_KEY} = 'testenvpass';
$ENV{EC2_REGION}     = 'testregion';
( @k ) = TPP::AWS::Credentials::credentials();
is( $k[0], 'testenvkey',	"Got access key credentials" );
is( $k[1], 'testenvpass',	"Got secret key credentials" );
is( $k[2], 'testregion',	"Got EC2 region credentials" );

note "credentials() trying from parameters, overriding env & file";
( @k ) = TPP::AWS::Credentials::credentials( 'userp', 'passp', 'regionp');
is( $k[0], 'userp',             "Got access key credentials" );
is( $k[1], 'passp',             "Got secret key credentials" );
is( $k[2], 'regionp',           "Got EC2 region credentials" );

ok( TPP::AWS::Credentials::remove(), "credentials file removed" );
