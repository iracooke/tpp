#
# $Id: $
# $Name:  $
#
use File::Basename;
use Test::More tests => 2;

BEGIN  {  use_ok( 'TPP::AWS::DaemonClient' ); }

SKIP: {
   skip 'Tests not relevant for windows systems', 1 if ( $^O =~ /MSWin/ );
   our $prog = basename($0);
   my $c = new_ok( TPP::AWS::DaemonClient );
}
