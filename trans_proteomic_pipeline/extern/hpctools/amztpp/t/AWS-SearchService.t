#
# $Id: $
# $Name:  $
#
use Test::More tests => 2;
use Cwd 'abs_path';
use TPP::AWS::Credentials qw( credentials );

BEGIN { 
      use_ok( 'TPP::AWS::SearchService' );
      }

my $dir    = abs_path "t/data";
my $input  = "$dir/test.mzXML";
my $params = "$dir/tandem.params";

my $q = new_ok( TPP::AWS::SearchService => [ $input, $params ] );
