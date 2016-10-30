#
# Program: TPP AWS Search Tool
# Author:  Joe Slagel
#
# Copyright (C) 2010-2012 by Joseph Slagel
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
use Test::More tests => 8;
use Cwd 'abs_path';
use TPP::AWS::Credentials qw( credentials );
use TPP::AWS::S3Manager;
use TPP::AWS::Logger qw( $log );
use File::Basename;
use File::Spec::Functions;
use File::pushd;
use File::Which qw(which);

#$log->{level} = 1;
#$log->{fmt}   = '[%d] (%p) %m';

BEGIN { 
      use_ok( 'TPP::AWS::CometService' );
      }

note( "Test Service Object Creation" );
my $dir    = abs_path "t/data";
my $input  = "$dir/test.mzXML";
my $params = "$dir/comet.params";

my $cwd = pushd $dir;
my $s = new_ok( TPP::AWS::CometService => [ $input, $params ] );
is( basename($s->{INPUT_FILES}->{DB}), "test.fasta",     "...found test.fasta" );

#note( "Test _updateParams()" );
#my $testParams = "$dir/test.comet.params";
#`cp $params $testParams`;
#$s->_updateParams( $testParams, "$dir/test.fasta" );
#my $found=`egrep '^ProteinDatabase=$dir/test.fasta\$' $testParams`;
#ok( $found,					"...db updated" );

SKIP: {
   skip( "comet not found on system", 4 ) unless which('comet');
   
   note( "Test run() ... this could take a little while" );
   unlink catfile( $dir, "test.pep.xml" );
   $s->{WORK_FILES} = { %{$s->{INPUT_FILES}} };
   ok( $s->run(),                       "run()" );
   is( $s->{ERROR}, undef,              "...no error in comet" );
   ok( -f "$dir/test.pep.xml",		"...test.pep.xml exists" );
   ok( -f "$dir/test.comet.log",	"...test.comet.log exists" );
}

SKIP: {
   my @keys = credentials();
   skip "skipping due to no AWS credentials", 1 if ( !$keys[0] && !$keys[1] );
   my $s3m = TPP::AWS::S3Manager->new();
   $keys[3] = lc "tpp-$keys[0]-test";
   $s3m->open( @keys );
   
   is( $s->uploadInput( $s3m ), 3,      "uploadInput()" );
   $s3m->delete();
}

# Remove test files
unlink catfile( $dir, "test.fasta.index" );
unlink catfile( $dir, "test.comet.log" );
unlink catfile( $dir, "test.comet.params" );
unlink catfile( $dir, "test.pep.xml" );
unlink catfile( $dir, "test.pepXML" );

