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
use Test::More tests => 7;
use Cwd 'abs_path';
use TPP::AWS::Credentials qw( credentials );
use TPP::AWS::S3Manager;
use FindBin;
use File::Basename;
use File::Spec::Functions;
use File::Which qw(which);

BEGIN { 
      use_ok( 'TPP::AWS::InspectService' );
      }

my ( $accessKey, $secretKey, $ec2region );
my $dir = abs_path catfile( "t", "data" );

chdir( "$FindBin::Bin/data" );
   
# Initialize service
my $input  = 'test.mzXML';
my $params = 'inspect.params';
my @dbs    = ('test.fasta');
my $s = new_ok( TPP::AWS::InspectService => [ $input, $params ] );

# Read params
is( basename( ($s->_readParams( $params ))[0] ), 'test.fasta', 
              "_readParams() is test.fasta" );

# Directly run a search
SKIP: {
   skip( "inspect not found on system", 3 ) unless which('inspect');
   note( "running inspect search...this may take some time" );
   
   unlink catfile( $dir, "test.pep.xml" );
   $s->{WORK_FILES} = { %{$s->{INPUT_FILES}} };
   ok( $s->run(),                       "run()" );
   ok( -f "test.inspect.log",	        "inspect log file exists" );
   ok( -f "test.pep.xml",       	"inspect pep.xml file exists" );
}

# Try a upload
SKIP: {
   my @keys = credentials();
   skip "skipping due to no AWS credentials", 1 if ( !$keys[0] && !$keys[1] );
   $keys[3] = lc "tpp-$keys[0]-test";		# use a test bucket

   my $s3m = TPP::AWS::S3Manager->new();
   $s3m->open( @keys );
   is( $s->uploadInput( $s3m ), 3,      "uploaded inspect input files" );
   $s3m->delete();
}

# Remove test files
unlink catfile( $dir, "test.tandem.params" );
unlink catfile( $dir, "AdditionalDB.trie" );
unlink catfile( $dir, "test.inspect" );
unlink catfile( $dir, "AdditionalDB.index" );
unlink catfile( $dir, "inspect.err" );
unlink catfile( $dir, "test.pep.xml" );
unlink catfile( $dir, "test.inspect.log" );
unlink catfile( $dir, "test.inspect.params" );
