#
# Program: TPP AWS Search Tool
# Author:  Joe Slagel
#
# Copyright (C) 2009-2012 by Joseph Slagel
# 
# This library is free software; you can redistribute it and/or             
# modify it under the terms of the GNU Lesser General Public                
# License as published by the Free Software Foundation; either              
# version 2.1 of the License, or (at your option) any later version.        
#                                                                           
# This library is distributed in the hope that it will be useful,           
# but WITHOUT ANY WARRANTY; without even the implied warranty of            
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         
# General Public License for more details.                                  
#                                                                           
# You should have received a copy of the GNU Lesser General Public          
# License along with this library; if not, write to the Free Software       
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
# 
# Institute for Systems Biology
# 1441 North 34th St.
# Seattle, WA  98103  USA
# jslagel@systemsbiology.org
#
# $Id: $
#
use Test::More tests => 21;
use Cwd 'abs_path';
use File::Copy;
use File::pushd;
use File::Spec::Functions;
use File::Which qw(which);

use TPP::AWS::Credentials qw( credentials );
use TPP::AWS::Logger qw( $log );
use TPP::AWS::S3Manager;

BEGIN { 
      use_ok( 'TPP::AWS::TandemService' );
      }

#$log->{level} = 1;
#$log->{fmt}   = '[%d] (%p) %m';

# Simple helper function
sub contains
   {
   my ( $file, $qr ) = @_;
   my $contents = do { local( @ARGV, $/ ) = $file; <> };
   return $contents =~ $qr;
   }

note( "Test Service Object Creation" );
my $dir      = abs_path catfile( "t", "data" );
my $input    = catfile( $dir, "test.mzXML" );
my $params   = catfile( $dir, "tandem.params" );
my $db       = catfile( $dir, "test.fasta" );
my $defaults = catfile( $dir, "default_input.xml" );
my $taxonomy = catfile( $dir, "taxonomy.xml" );
my $cwd = pushd $dir;

my $s = new_ok( TPP::AWS::TandemService => [ $input, $params ] );
is( $s->{TAXON}, 'testtaxon',                   "...found testtaxon" );
is( $s->{THREADS}, 2,                           "...found threads" );
is( $s->{INPUT_FILES}->{DB}, $db,               "...found test.fasta" );
is( $s->{INPUT_FILES}->{DEFAULTS}, $defaults,   "...found default_input.xml" );
is( $s->{INPUT_FILES}->{TAXONOMY}, $taxonomy,   "...found taxonomy.xml" );


note( "Test _copyParams()" );
my $testParams = catfile( $dir, "test.tandem.params" );
my $out = $s->_copyParams( $params, $testParams, "test.mzXML",
                          "test.taxonomy.xml", "test.default.xml",
                        );
ok( $out,                                       "_copyParams()" );
ok( -f $testParams,                             "...test.tandem.params exists" );
ok( contains($testParams, '>test.mzXML<'),      "...input file was set" );
ok( contains($testParams, 'test.tandem<'),      "...output file was set" );
ok( contains($testParams, '>test.default.xml<'),"...default file was set" );
ok( contains($testParams, '>test.taxonomy.xml<'),"...taxonomy file was set" );


note( "Test _makeTaxonomy()" );
$taxonomy = catfile( $dir, "test.taxonomy.xml" );
$out = $s->_makeTaxonomy( $taxonomy, $s->{TAXON}, [ $s->{INPUT_FILES}->{DB} ] );
ok( $out,                                       "_makeTaxonomy()");
ok( -f $taxonomy,                               "...taxonomy file exists" );
ok( contains($taxonomy, "label=\"$s->{TAXON}\""), "...taxon was set" );
ok( contains($taxonomy, 'test.fasta"'),         "...fasta file was set" );


SKIP: {
   skip( "tandem not found on system", 3 ) unless 
      (which('tandem') || which('tandem.exe'));
   
   note( "Test _search() ... this could take a little while" );
   unlink catfile( $dir, "test.pep.xml" );
   copy( catfile($dir,"default_input.xml"), catfile($dir, "test.default.xml") );
   $s->{INPUT_FILES}->{TAXONOMY} = $taxonomy;
   $s->{WORK_FILES} = { %{$s->{INPUT_FILES}} };
   ok( $s->run(),                                 "_search()" );
   ok( -f catfile( $dir, "test.pep.xml" ), 	  "...test.pep.xml exists" );
   ok( -f catfile( $dir, "test.tandem.log" ),     "...test.tandem.log exists" );
}

SKIP: {
   my @keys = credentials();
   skip "skipping due to no AWS credentials", 1 if ( !$keys[0] && !$keys[1] );
   
   $keys[3] = lc "tpp-$keys[0]-test";

   my $s3m = TPP::AWS::S3Manager->new();
   $s3m->open( @keys );
   
   ok( $s->uploadInput( $s3m ),                 "uploadInput()" );
   $s3m->delete();
   }

# Remove test files
unlink catfile( $dir, "test.tandem.params" );
unlink catfile( $dir, "test.taxonomy.xml" );
unlink catfile( $dir, "test.default.xml" );
unlink catfile( $dir, "test.tandem" );
unlink catfile( $dir, "test.tandem.log" );
unlink catfile( $dir, "test.output_sequences" );
unlink catfile( $dir, "test.pep.xml" );
