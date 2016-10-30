#
# Program: TPP AWS Search Tool
# Author:  Joe Slagel
#
# Copyright (C) 2013 by Joseph Slagel
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
use Test::More tests => 9;

use File::Basename;
use File::Spec::Functions qw( rel2abs splitdir catdir catfile );
use File::pushd;
use File::Path qw( remove_tree );
use File::Which qw(which);

use TPP::AWS::Credentials qw( credentials );
use TPP::AWS::S3Manager;
use TPP::AWS::Logger qw( $log );


#$log->{level} = 1;
#$log->{fmt}   = '[%d] (%p) %m';

BEGIN { 
      use_ok( 'TPP::AWS::RNASeqPDBService' );
      }

my $dir    = rel2abs( catfile( 't', 'data' ) );
my $input  = catfile( $dir, "test.fastq" );
my $params = catfile( $dir , "rnaseqpdb.params" );


note( "RNASeqPDBService Creation" );
my $cwd = pushd $dir;
my $s = new_ok( TPP::AWS::RNASeqPDBService => [ $input, $params ] );

my @list;
TPP::AWS::RNASeqPDBService::_addFiles( \@list, $dir );
ok( @list, 	"_addFiles() found a few files in $dir" );

make_params();

SKIP: {
    skip( "rnaseqpdb not found on system", 5 ) unless which('rnaseqpdb.pl');
    skip( "takes too long", 5 );

    note( "Test run() ... this *will* take some time" );
    unlink catfile( $dir, "test.rnaseqpdb.log" );
    $s->{WORK_FILES} = { %{$s->{INPUT_FILES}} };
    ok( $s->run(),              "run()" );
    is( $s->{ERROR}, undef,     "...no error in rnaseqpdb" );
    ok( -d "$dir/test-fastqc",	"...test-fastqc exists" );
    ok( -d "$dir/test-tophat",	"...test-tophat exists" );
    ok( -d "$dir/test-dbase",	"...test-dbase exists" );
}

SKIP: {
   my @keys = credentials();
   skip "skipping due to no AWS credentials", 1 if ( !$keys[0] && !$keys[1] );
   my $s3m = TPP::AWS::S3Manager->new();
   $keys[3] = lc "tpp-$keys[0]-test";
   $s3m->open( @keys );
   
   is( $s->uploadInput( $s3m ), 2,      "uploadInput()" );
   $s3m->delete();
}

# Remove test files
unlink $params;
remove_tree( catfile( $dir, "test-fastqc" ) );
remove_tree( catfile( $dir, "test-tophat" ) );
remove_tree( catfile( $dir, "test-dbase" ) );
exit 0;


sub make_params
{
   note( "Generating rnaseqpdb.params" );

   my $tpphome    = which('PeptideProphetParser');
   my $rnaseqhome = which('rnaseqpdb.pl');
   my $snpeffhome = which('joinSnpEff.pl');

   open( PARAMS, ">$params" ) or die( "Can't open $params: $!" );
   print PARAMS "# Auto-generated do not edit\n";
   if ( $tpphome )
      {
      my @d = splitdir( $tpphome ); 
      pop @d; pop @d; $tpphome = catdir( @d );
      print PARAMS "\$TPP_HOME = '$tpphome';\n";
      }
   if ( $rnaseqhome )
      {
      my @d = splitdir( $rnaseqhome ); 
      pop @d; pop @d; $rnaseqhome = catdir( @d );
      print PARAMS "\$RNASEQ_HOME = '$rnaseqhome';\n";
      }
   if ( $snpeffhome )
      {
      my @d = splitdir( $snpeffhome ); 
      pop @d; pop @d; $snpeffhome = catdir( @d );
      print PARAMS "\$SNPEFF_HOME = '$snpeffhome';\n";
      }
   print PARAMS "1;\n";
   close( PARAMS );
}
