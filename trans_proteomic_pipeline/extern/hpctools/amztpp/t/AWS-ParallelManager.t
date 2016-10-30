#
# $Id: $
# $Name:  $
#
use Test::More tests => 7;

use TPP::AWS::Logger qw( $log );

BEGIN  {  use_ok( 'TPP::AWS::ParallelManager' ) }

my $srv  = MockService->new();
my $s3m  = MockS3Manager->new();
my $sqsm = MockSQSManager->new();


# debug
#$log->{level} = 1;
#$log->{fmt}   = '[%d] (%p) %m';

my $pm = new_ok( TPP::AWS::ParallelManager => [4] );
ok( $pm->spawnUpload( $s3m, $sqsm, $srv ),      'spawned an upload' );
is( $pm->count(), 1,                            'pid table contains 1' );
ok( $pm->spawnDownload( $s3m, $sqsm, $srv ),    'spawned an download' );
is( $pm->count(), 2,                            'pid table contains 2' );

note( 'reap children (for 8 seconds) ' );
my $c = 0;
for ( 0..7 )
   {
   $c += $pm->reap();
   sleep(1);	
   }
is( $c, 2,                                      'reaped children' );

exit( 0 );


#-- @MOCK CLASSES -----------------------------------------------------------#

package MockService;

sub new { bless {} }
sub chwdir {}
sub chodir {}
sub uploadInput    { sleep( 2 + int(rand(4))) }  # sleep for 2-5 seconds
sub downloadOutput { sleep( 2 + int(rand(4))) }  # sleep for 2-5 seconds

package MockS3Manager;

sub new { bless {} }

package MockSQSManager;

sub new { bless {} }
sub deleteMessage {}
sub queueService {}
sub queueDone {}
sub upQueue {}
sub downQueue {}
sub doneQueue {}
