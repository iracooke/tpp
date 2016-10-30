#
# $Id: $
# $Name:  $
#
use Test::More tests => 5;

BEGIN { 
      use_ok( 'TPP::AWS::Logger', qw($log) );
      }

my $output = '';
open( $fh, '>', \$output ) or die "$!";
my $log = new_ok( TPP::AWS::Logger => [ 'info', $fh ] );
$log->debug( 'silent' );
ok ( $output eq '',	'Debug output was silent' );
$log->warn( 'warn' );
like( $output, qr/warn/,'Warn output was seen' );
$output = '';

eval { $log->logdie( "dying gasp" ) };
like( $@,      qr/dying gasp/,	'As expected, logdie called die()' );
#like( $output, qr/dying gasp/,  'Output from logdie was seen' );

