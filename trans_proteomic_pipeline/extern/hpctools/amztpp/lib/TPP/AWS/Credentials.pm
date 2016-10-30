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
# $Id: Credentials.pm 5877 2012-05-17 20:40:41Z slagelwa $
#
package TPP::AWS::Credentials;
use strict;
use warnings;

use base qw( Exporter );
use File::HomeDir qw( home );
use File::Spec::Functions qw( catfile );

use TPP::AWS::Logger qw( $log );

our $REVISION = (q$Revision: 5877 $ =~ /(\d+)/g)[0] || '???'; 
our @EXPORT_OK = qw( credentials );


#-- @SUBROUTINES ------------------------------------------------------------#

#
# Try to determine the AWS credentials access key, secret key, and region to use
# via a variety of ways:
#
#   1) Provided in the parameters
#   2) Environment variables (EC2_ACCESS_KEY, EC2_SECRET_KEY, EC2_REGION,
#      and S3_BUCKET)
#   3) Reading from the .awssecret file in your local home directory
#
sub credentials
   {
   my ( $accessKey, $secretKey, $ec2region, $s3bucket ) = @_;
   
   # Provided or Enviroment values?
   $accessKey ||= $ENV{EC2_ACCESS_KEY};
   $secretKey ||= $ENV{EC2_SECRET_KEY};
   $ec2region ||= $ENV{EC2_REGION};
   $s3bucket  ||= $ENV{S3_BUCKET};
   if ( $accessKey && $secretKey )
      {
      return( $accessKey, $secretKey, $ec2region, $s3bucket )
      }
   
   # Try reading .awssecret in case the keys are there.
   my @keys = TPP::AWS::Credentials::read();
   if ( $keys[0] && $keys[1] )
      {
      return( $keys[0], $keys[1], 
              $ec2region || $keys[2], $s3bucket || $keys[3] );
      }
      
   return();
   }

#
# Read AWS credentials from a file
# 
# @arg   File to write credentials to (optional)
#
sub read
   {
   my ( $awsfile ) = @_;
   $awsfile = '' unless defined $awsfile;
   $awsfile ||= $ENV{AWS_CREDENTIAL_FILE};
   $awsfile ||= catfile( File::HomeDir->my_data(), '.awssecret' );
   return() unless ( -f $awsfile );
   
   local *AWSFILE;
   open( AWSFILE, "< $awsfile" ) 
      or $log->logdie( "can't read credentials from file $awsfile: $!" );
      
   my @fields;
   while ( my $line = <AWSFILE> )
      {
      next if ( $line =~ /\s*#/ );
      chomp( $line );
      push @fields, $line;
      last if ( $#fields == 3 );
      }
   close( AWSFILE );
   
   # Check permissions on file
   my $stat = (stat $awsfile)[2] & 0777;
   if ( $^O !~ /MSWin/ && ($stat & 0477) != 0400 )
      {
      $log->warn( "$awsfile file permissions are too open. Should be -rw-------" );
      }
      
   return( @fields );           # access key, secret key, and optionally region 
   }

#
# Write AWS credentials to a file
# 
# @arg   AWS access key
# @arg   AWS secret key 
# @arg   AWS region id to use
# @arg   AWS S3 bucket to use
# @arg   File to write credentials to (optional)
#
sub write
   {
   my ( $accessKey, $secretKey, $region, $s3bucket, $awsfile ) = @_;
   
   $accessKey = '' unless defined $accessKey;
   $secretKey = '' unless defined $secretKey;
   $region    = '' unless defined $region;
   $awsfile   = '' unless defined $awsfile;
   $s3bucket  = '' unless defined $s3bucket;
   $awsfile ||= catfile( File::HomeDir->my_data(), '.awssecret' );
   
   local *AWSFILE;
   open( AWSFILE, "> $awsfile" ) 
      or $log->logdie( "can't save credentials to file $awsfile: $!" );
   print AWSFILE "$accessKey\n";
   print AWSFILE "$secretKey\n";
   print AWSFILE "$region\n"   if ( $region );
   print AWSFILE "$s3bucket\n" if ( $s3bucket );
   close( AWSFILE );
   
   # Set permissions
   chmod 0400, $awsfile 
      or $log->logwarn( "unable to set permissions on credentials file: $!");
   }
   
#
# Just removes the credentials file
# 
sub remove
   {
   my ( $awsfile ) = @_;
   $awsfile   = '' unless defined $awsfile;
   $awsfile ||= catfile( File::HomeDir->my_data(), '.awssecret' );
   
   unlink $awsfile 
      or $log->logdie( "can't remove credentials file $awsfile: $!" );
   }
   
1;
__END__
