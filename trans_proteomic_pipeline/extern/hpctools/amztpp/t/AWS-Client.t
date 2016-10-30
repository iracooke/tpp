#
# $Id: $
# $Name:  $
#
use File::Basename;
use Test::More tests => 2;

BEGIN  {  use_ok( 'TPP::AWS::Client' ); }

our $prog = basename($0);
my $c = new_ok( TPP::AWS::Client );
