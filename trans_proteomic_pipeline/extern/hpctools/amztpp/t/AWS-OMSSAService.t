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
use Test::More tests => 9;
use Cwd 'abs_path';
use File::Spec::Functions;
use FindBin;
use File::Which qw(which);

use TPP::AWS::Credentials qw( credentials );
use TPP::AWS::S3Manager;
use TPP::AWS::Logger qw( $log );

BEGIN { 
      use_ok( 'TPP::AWS::OMSSAService' );
      }

#$log->{level} = 1;
#$log->{fmt}   = '[%d] (%p) %m';

my ( $accessKey, $secretKey, $ec2region );
my $dir = abs_path catfile( "t", "data" );

chdir( "$FindBin::Bin/data" );

# Initialize service
my $input  = 'test.mzXML';
my $params = 'omssa.params';
my $s = new_ok( TPP::AWS::OMSSAService => [ $input, $params ] );
   
# Load flags
my @flags;
ok( @flags = $s->_readFlags( $params ), "_readFlags() " );
is( $flags[0], "-d test.fasta",      "flags are correct");
   
# Directly run a search
SKIP: {
   skip( "TPP not found on system", 4 )   unless which('MzXML2Search');
   skip( "omssa not found on system", 4 ) unless which('omssacl');
   
   note( "running omssa search...this may take some time" );
   unlink catfile( $dir, "test.pep.xml" );
   $s->{WORK_FILES} = { %{$s->{INPUT_FILES}} };
   ok( $s->run(), "run()" );
   is( $s->{ERROR}, undef,              "no error messages" );
   ok( -f "test.omssa.log",             "omssa log file exists" );
   ok( -f "test.pep.xml",	        "omssa pep.xml file exists" );
}

# Try a upload
SKIP: {
   my @keys = credentials();
   skip "skipping due to no AWS credentials", 1 if ( !$keys[0] && !$keys[1]);
   $keys[3] = lc "tpp-$keys[0]-test";
   
   my $s3m = TPP::AWS::S3Manager->new();
   $s3m->open( @keys );
   is( $s->uploadInput( $s3m ), 8, 	"uploaded omssa input files" );
   $s3m->delete();
}

# Remove test files
unlink catfile( $dir, "test.fasta.index" );
unlink catfile( $dir, "test.omssa.log" );
unlink catfile( $dir, "test.mgf" );
unlink catfile( $dir, "test.pep.xml" );
