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
# $Id: Report.pm 6285 2013-09-20 16:50:03Z slagelwa $
#
package TPP::AWS::Report;
use strict;
use warnings;

use Class::InsideOut qw( :std );
use File::Basename;
use POSIX qw(strftime);

use TPP::AWS::Service;
use TPP::AWS::Logger qw( $log );


#-- @GLOBALS ----------------------------------------------------------------#

our $REVISION = (q$Revision: 6285 $ =~ /(\d+)/g)[0] || '???'; 


#-- @ATTRIBUTES -------------------------------------------------------------#

private  hosts		=> my %hosts;
private  ids		=> my %ids;
readonly services       => my %services;
readonly serviceCount	=> my %serviceCount;
readonly errors 	=> my %errors;
readonly runTime	=> my %runTime;


#-- @PUBLIC -----------------------------------------------------------------#

sub new
   {
   my ( $class ) = @_;
   
   # Create object
   my $self = Class::InsideOut::new( $class );
   my $id   = id $self;
   
   $services{$id}       = [];
   $hosts{$id}          = {};
   $ids{$id}            = {};
   $serviceCount{$id}   = 0;
   $errors{$id}         = [];           # List of services with errors
   $runTime{$id}        = 0; 
   
   return $self;
   }

sub add
   {
   my ( $self, $srv ) = @_;	
   my $id = id $self;
   
   $serviceCount{$id}            += 1;
   $hosts{$id}->{$srv->{SERVER}} += 1;
   $ids{$id}->{$srv->{INSTANCE_ID}} += 1 if ( $srv->{INSTANCE_ID} );
   
   if ( $srv->{CLIENT_SUBMIT} && $srv->{CLIENT_RECEIVE} ) {
      $runTime{$id} += ($srv->{CLIENT_RECEIVE} - $srv->{CLIENT_SUBMIT});
   }
   push @{$errors{$id}}, $srv if ( @{$srv->{ERRORS}} );
      
   if ( $log->is_debug() )
      {
      my $out = sprintf "%-4d %s %-10.10s %18s %18s %18s %18s %4d %s\n", 
         $srv->{NUM},  $srv->{SERVER}, $srv->{INSTANCE_ID},
         strftime( "%Y-%m-%d %H:%M:%S", localtime($srv->{CLIENT_SUBMIT}) ),
         strftime( "%Y-%m-%d %H:%M:%S", localtime($srv->{SERVER_RECEIVE}) ),
         strftime( "%Y-%m-%d %H:%M:%S", localtime($srv->{SERVER_SUBMIT}) ),
         strftime( "%Y-%m-%d %H:%M:%S", localtime($srv->{CLIENT_RECEIVE}) ),
         $srv->{SERVER_WTIME}, $srv->{INPUT_FILES}->{INPUT};
      $log->debug( "reporting $out" );
      }
      
   push @{$services{$id}}, $srv;
   }
   
sub ec2Count
   {
   my ( $self ) = @_;	
   return scalar(keys %{$ids{id $self}});
   }
   
sub avgRunTime
   {
   my ( $self ) = @_;	
   return 'NaN' unless $self->serviceCount();
   return( $self->runTime() / $self->serviceCount() );
   }

sub avgUploadSpeed
   {
   my ( $s ) = @_;	
   return 'NaN' unless $s->{UPSECS};
   return( ($s->{UPBYTES}/1_048_576) / $s->{UPSECS} );
   }
   
sub avgDownloadSpeed
   {
    my ( $s ) = @_;	
   return 'NaN' unless $s->{DOWNSECS};
   return( ($s->{DOWNBYTES}/1_048_576) / $s->{DOWNSECS} );
   }
   
sub statistics
   {
   my ( $self, $src ) = @_;	
   my $id = id $self;
   
   my %stats = ( UPCOUNT => 0, 
                 UPBYTES => 0,
                 UPSECS => 0,
                 DOWNCOUNT => 0,
                 DOWNBYTES => 0,
                 DOWNSECS => 0 );
   foreach my $s ( @{ $services{id $self} } )
      {
      $stats{$_} += $s->{"${src}_$_"} foreach ( keys %stats );
      } 
   return \%stats;
   }
   
sub printText
   {
   my ( $self, $out ) = @_;
   $out ||= \*STDOUT;
   
   my $id = id $self;
   
   printf $out " # Instances: %d\n",		$self->ec2Count();
   printf $out "  # Services: %d\n",		$self->serviceCount();
   printf $out "    # Errors: %d\n",		scalar( @{$self->errors} );
   printf $out "    Avg Time: %.1f (sec)\n",	$self->avgRunTime();
   
   my $s = $self->statistics( 'CLIENT' );
   printf $out "\nClient Bandwidth\n";
   printf $out "Upload Summary\n";
   printf $out "   # Uploads: %d\n",		$s->{UPCOUNT};
   printf $out "  Total (MB): %.2f\n",		$s->{UPBYTES}/1_048_576;
   printf $out "  Avg (MB/s): %.2f\n",		avgUploadSpeed( $s );
   printf $out "Download Summary\n";
   printf $out " # Downloads: %d\n",		$s->{DOWNCOUNT};
   printf $out "  Total (MB): %.2f\n",		$s->{DOWNBYTES}/1_048_576;
   printf $out "  Avg (MB/s): %.2f\n",		avgDownloadSpeed( $s );
   
   $s = $self->statistics( 'SERVER' );
   printf $out "\nAmazon Bandwidth\n";
   printf $out "Upload Summary\n";
   printf $out "   # Uploads: %d\n",		$s->{UPCOUNT};
   printf $out "  Total (MB): %.2f\n",		$s->{UPBYTES}/1_048_576;
   printf $out "  Avg (MB/s): %.2f\n",		avgUploadSpeed( $s );
   printf $out "Download Summary\n";
   printf $out " # Downloads: %d\n",		$s->{DOWNCOUNT};
   printf $out "  Total (MB): %.2f\n",		$s->{DOWNBYTES}/1_048_576;
   printf $out "  Avg (MB/s): %.2f\n",		avgDownloadSpeed( $s );
   print  $out "\n";
   
   #printf $out "%-40.40s %-30.30s\n", "Input File", "Error";
   print $out "Input File\tError\n";
   foreach my $s ( @{ $self->errors() } )
      {
      foreach my $e ( @{ $s->{ERRORS} } )
         {
         chomp( $e );
         print $out ($s->{NAME} || '<unknown>') . "\t$e\n";
         }
      }
   print $out "...no errors to report\n" unless ( @{ $self->errors() } );
   print $out "\n";
      
   print $out "Num  Instance ID  Req Submit          Req Receive         Res Submit          Res Receive         Wall time Service   Input File\n";
   foreach my $s ( @{ $services{id $self} } )
      {
      my ( $name, $dir, $suffix ) = fileparse( $s->{NAME}, qr/\.[^.]*$/ );
      my $type = ref($s); $type =~ s/TPP::AWS:://;
      $type =~ s/Service$//;
      printf $out "%-4d %-10.10s   %18s %18s %18s %18s %-6d    %-9.9s %s\n", 
         $s->{NUM},  ($s->{INSTANCE_ID} || ''), 
         strftime( "%Y-%m-%d %H:%M:%S", localtime($s->{CLIENT_SUBMIT}) ),
         strftime( "%Y-%m-%d %H:%M:%S", localtime($s->{SERVER_RECEIVE}) ),
         strftime( "%Y-%m-%d %H:%M:%S", localtime($s->{SERVER_SUBMIT}) ),
         strftime( "%Y-%m-%d %H:%M:%S", localtime($s->{CLIENT_RECEIVE}) ),
         $s->{SERVER_WTIME}, $type, $name;
      }
    printf $out "\n";
      
#   printf $out "Summary SQS Queue Performance\n";
#   printf $out " Avg Request Queue Wait Time: \n";
#   printf $out " Avg Respose Queue Wait Time: \n";
   }
   
sub printXML
   {
   my ( $self, $out ) = @_;
   $out ||= \*STDOUT;
   
   my $id = id $self;
   
   print $out qq{<amztpp>\n};
   
   print $out qq{<services>\n};
   foreach my $s ( @{ $services{id $self} } )
      {
      my $type = ref($s); $type =~ s/TPP::AWS:://;
      print $out qq{ <service};
      print $out qq{ num="$s->{NUM}"};
      print $out qq{ name="$s->{NAME}"};
      print $out qq{ type="$type"};
      print $out qq{ instance_id="$s->{INSTANCE_ID}"};
      print $out qq{ server_wtime="$s->{SERVER_WTIME}"};
      print $out qq{ />\n};
      }
   print $out qq{</services>\n};
      
   print $out qq{</amztpp>\n};
   
   return 0;
   }
    
1;
