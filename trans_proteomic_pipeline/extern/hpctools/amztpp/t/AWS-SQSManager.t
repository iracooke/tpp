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
# $Name:  $
#
use Test::More tests => 16;
use TPP::AWS::Credentials qw( credentials );
use FindBin;

BEGIN { 
      use_ok( 'TPP::AWS::SQSManager' );
      }

$| = 1;


# Initialize
chdir( "$FindBin::Bin/data" );

my @keys;

SKIP: {
   @keys = credentials();
   skip "skipping due to no AWS credentials", 15 if ( !$keys[0] || !$keys[1] );

   my $router = new_ok( TPP::AWS::SQSManager => [ @keys[0..2], 'AMZTPPTest' ] );
   ok( $router->createQueues(), 'Creating queues' );

   my $srv = TestService->new();
   $INC{TestService} = 'TestService.pm';       # Fool perl into thinking its loaded
   
   ok( $router->queueUpload( $srv ),	"Sending upload" );
   for ( my $i = 5; $i; $i--) { $srv = $router->dequeueUpload(); last if $srv; sleep 2 }
   ok( $srv,	   			"Received upload" );
   isa_ok( $srv, 'TestService' );
   
   ok( $router->queueService( $srv ),	"Sending service" );
   for ( my $i = 5; $i; $i--) { $srv = $router->dequeueService(); last if $srv; sleep 2 }
   ok( $srv,	   			"Received service" );
   isa_ok( $srv, 'TestService' );
   
   ok( $router->queueDownload( $srv ),	"Sending download" );
   for ( my $i = 5; $i; $i--) { $srv = $router->dequeueDownload(); last if $srv; sleep 2 }
   ok( $srv,	   			"Received download" );
   isa_ok( $srv, 'TestService' );
   
   ok( $router->queueDone( $srv ),	"Sending done" );
   for ( my $i = 5; $i; $i--) { ($srv) = $router->dequeueDone(); last if $srv; sleep 2 }
   ok( $srv,	   			"Received done" );
   isa_ok( $srv, 'TestService' );
   
   ok( $router->delete(), 'Delete any queues (need to wait 60 secs afterwards)' );
#  sleep(60);   # can't recreate a queue for 60 secs so might as well wait
   }

package TestService;

use base 'TPP::AWS::Service';

sub run
   {
   my ( $this ) = @_;
   $this->{MSG} = 'Hello world';
   $this->{UID} = 'test';
   }

1;
