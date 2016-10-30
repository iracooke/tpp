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
# $Id: Status.pm 6210 2013-06-17 19:46:48Z slagelwa $
#
package TPP::AWS::Status;
use strict;
use warnings;

use File::Basename;

use TPP::AWS::Logger qw( $log );


#-- @GLOBALS ----------------------------------------------------------------#

our $REVISION = (q$Revision: 6210 $ =~ /(\d+)/g)[0] || '???'; 


#-- @PUBLIC -----------------------------------------------------------------#

#
# Print a status report to the console
#
sub printText
   {
   my ( $pid, $ec2m, $sqsm, $s3m ) = @_;;
   
   print "Client background process: ";
   print $pid ? "is running (PID:$pid)\n" : "is stopped\n";
   print "\n";
   
   print "EC2\n";
   print
"Instance      S  AMI ID        Type       Started             Public DNS\n";
   print
"------------  -  ------------  ---------  ------------------- ----------\n";
   foreach ( $ec2m->getSpotRequests() )
      {
      my $state = uc $_->state();
      printf "%-12s  %1.1s  %-12s  %-9s  %19.19s %s\n", 
         $_->spotInstanceRequestId(), $state, 
         $_->launchSpecification()->imageId(),
         $_->launchSpecification()->instanceType(), 
         $_->createTime(), $_->type();
      }
   foreach ( $ec2m->getInstances() )
      {
      # uc/substr needed else a memory panic !?!?
      my $status = uc substr( $_->instance_state()->name(), 0 );
      ( my $time = $_->launch_time() ) =~ s/T/ /;
      printf "%-12s  %1.1s  %-12s  %-9s  %19.19s %s\n", $_->instance_id(),
         $status, $_->image_id, $_->instance_type(), $time,
         ( $_->dns_name() || '' );
      }
   print "\n";

   print "SQS\n";
   print "Queue name                             ~ #      Timeout\n";
   print "-----------------------------------  ---------  -------------\n";
   foreach ( @{$sqsm->list()} )
      {
      ( my $time = $_->{VisibilityTimeout} ) =~ s/([\d-]+)T(.*)\.\dT/$1 $2/;
      printf "%-35.35s  %4d/%-4d  %d\n", $_->{Name},
         $_->{ApproximateNumberOfMessages}, 
         $_->{ApproximateNumberOfMessagesNotVisible}, 
         $_->{VisibilityTimeout};
      }
   print "\n";

   my $bucket = $s3m->bucket ? $s3m->bucket->bucket() : '-.-';
   my $i = 0;
   print "S3 (Bucket $bucket)\n";
   print "File name                            Size      Last Modified\n";
   print "-----------------------------------  --------  -------------\n";
   foreach ( @{$s3m->list( 10 )} )         
      {
      ( my $time = $_->{last_modified} ) =~ s/T/ /;
      printf "%-35.35s  %8.8s  %s\n", basename($_->{key}), human_size( $_->{size} ),
         $time;
      $i++;
      }
   print "...(only the first 10 files are shown)\n\n" if ( $i >= 9 );
   
   return 0;
   }
   
#
# Print a status report in XML format
#
sub printXML
   {
   my ( $pid, $ec2m, $sqsm, $s3m ) = @_;;
   $pid ||= '';
   
   print qq{<amztpp>\n};
   print qq{<client pid="$pid" />\n};
   
   print qq{<ec2>\n};
   foreach ( $ec2m->getSpotRequests() )
      {
      ( my $time = $_->createTime() ) =~ s/T/ /;
      print '<spot id="', $_->spotInstanceRequestId(), '"';
      print ' state="', $_->state(), '"';
      print ' instance_id="', ($_->instanceId() || ''), '"';
      print ' image_id="', $_->launchSpecification()->imageId(), '"';
      print ' type="', $_->launchSpecification()->instanceType(), '"';
      print ' created="', $time, '"' if ( $time );
      print " />\n";
      }
   foreach ( $ec2m->getInstances() )
      {
      # uc/substr needed else a memory panic !?!?
      my $status = $_->instance_state()->name();
      ( my $time = $_->launch_time() ) =~ s/T/ /;
      print '<instance id="', $_->instance_id(), '"';
      print ' status="', $status, '"';
      print ' image_id="', $_->image_id, '"';
      print ' type="', $_->instance_type, '"';
      print ' started="', $time, '"' if ( $time );
      print ' dns="', ($_->dns_name() || ''), '"';
      print " />\n";
      }
   print "</ec2>\n"; 
   
   print "<sqs>\n";
   foreach ( @{$sqsm->list()} )
      {
      ( my $time = $_->{VisibilityTimeout} ) =~ s/([\d-]+)T(.*)\.\dT/$1 $2/;
      print qq{<queue name="$_->{Name}" };
      print qq{ ApproximateNumberOfMessages="$_->{ApproximateNumberOfMessages}"}; 
      print qq{ ApproximateNumberOfMessagesNotVisible="$_->{ApproximateNumberOfMessagesNotVisible}"}; 
      print qq{ VisibilityTimeout="$_->{VisibilityTimeout}" }; 
      print qq{ />\n};
      }
   print "</sqs>\n"; 
   
   my $bucket = $s3m->bucket ? $s3m->bucket->bucket() : '-.-';
   print qq{<s3 bucket="$bucket">\n};
   foreach ( @{$s3m->list()} )
      {
      ( my $time = $_->{last_modified} ) =~ s/T/ /;
      print qq{<file key="$_->{key}" };
      print qq{ size="$_->{size}"};
      print qq{ last_modified="$time"};
      print qq{ />\n};
      }
   print "</s3>\n";
   
   print "</amztpp>\n";
   return 0;
   }

#
# Convert an numeric size into a human readable value 
#
sub human_size
   {
   my $val = shift;

   # 2**10 (binary) multiplier by default
   my $multiplier = @_ ? shift: 1024;

   my $magnitude = 0;
   my @suffixes  = qw/B KB MB GB TB PB EB/;

   my $rval;
   while ( ( $rval = sprintf( "%.1f", $val ) ) >= $multiplier )
      {
      $val /= $multiplier;
      $magnitude++;
      }

   # Use Perl's numeric conversion to remove trailing zeros
   # in the fraction and the decimal point if unnecessary
   $rval = 0 + $rval;

   return sprintf( "%s %2.2s",  $rval, $suffixes[$magnitude] );
   }
   
1;
