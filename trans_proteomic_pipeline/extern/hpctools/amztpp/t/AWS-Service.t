#
# $Id: $
#
use Cwd;
use File::Spec::Functions qw( catfile splitdir );
use Test::More tests => 5;

BEGIN { 
      use_ok( 'TPP::AWS::Service' );
      }

{
   my $s = new_ok( TPP::AWS::Service => [] );

   test_run();
   test_cwdir();
   exit 0;
}

sub test_run
{
   my $s = TPP::AWS::Service->new();
   eval { $s->run };
   ok( $@ =~ /abstract/, "run() method is abstract" );
}

sub test_cwdir
{
   note('Testing cwdir()');
   my $s = TPP::AWS::Service->new();
   my $sav = getcwd();
   {
      $s->{WDIR_LIST} = [ splitdir catfile( $sav, 't', 'data' ) ];
      my $cwd = $s->chwdir();
      cmp_ok( $cwd, 'ne', $sav, 'cwd changed in scope' );
   }
   cmp_ok( getcwd(), 'eq', $sav, 'cwd restored out of scope' );
}
