#
# $Id: $
# $Name:  $
#
use Test::More;
use FindBin;
use File::HomeDir;
use File::Spec;
use File::Temp qw( tempdir );

BEGIN
{
if ( $^O !~ /MSWin/ )
   {
   plan skip_all => 'Tests not relevant for non-windows systems';	
   }
else
   {
   plan tests => 3;	
   use_ok( 'TPP::AWS::Win32Client' ) if ( $^O =~ /MSWin/ );
   }
}

ok( TPP::AWS::Win32Client::createService, "create service" );
ok( TPP::AWS::Win32Client::deleteService, "delete service" );