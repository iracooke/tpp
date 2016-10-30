#
# $Id: $
# $Name:  $
#
use Test::More tests => 9;

BEGIN { 
      use_ok( 'TPP::AWS::Poll' );
      }

my $poll = new_ok( TPP::AWS::Poll => [] );
$poll->{MIN} = 1;	# speed tests up
$poll->{POLL} = 1;	# speed tests up
$poll->{QUIT} = 2;	# speed tests up
is( $poll->next(), 1, "next()" );
ok( !$poll->quit(), "not time to quit" );
$poll->next();
$poll->next();
$poll->next();
is( $poll->next(), 2, "next() timer is incrementing" );
ok( $poll->quit(), "time to quit" );
ok( $poll->reset(), "reset()" );
is( $poll->next(), 1, "poll was reset" );
ok( !$poll->quit(), "quit timer was reset" );
