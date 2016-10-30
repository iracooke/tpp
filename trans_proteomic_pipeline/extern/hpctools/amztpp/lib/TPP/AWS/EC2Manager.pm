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
# $Id: EC2Manager.pm 6739 2014-11-12 22:38:49Z slagelwa $
#
package TPP::AWS::EC2Manager;
use strict;
use warnings;

use Class::InsideOut qw( :std );
use VM::EC2 1.23;

use TPP::AWS::Logger qw( $log );


#-- @GLOBALS ----------------------------------------------------------------#

our $REVISION = (q$Revision: 6739 $ =~ /(\d+)/g)[0] || '???'; 


#-- @ATTRIBUTES -------------------------------------------------------------#

readonly ec2     => my %ec2;            # VM::EC2 object
readonly spots   => my %spots;          # EC2 Spot Instance Requests
readonly cancels => my %cancels;        # EC2 Spot Instance Cancels
readonly region  => my %region;
readonly bucket  => my %bucket;


#-- @PUBLIC -----------------------------------------------------------------#

#
# Constructor
#
# @arg   Access key for authorized user
# @arg   Secret key corresponding to the access key
# @arg   EC2 region or endpoint (optional defaults to AWS us-west-2 region)
# @arg   S3 bucket to use
#
sub new
   {
   my ( $class, $accessKey, $secretKey, $region, $bucket ) = @_;

   # Create object
   my $self = Class::InsideOut::new( $class );
   my $id   = id $self;

   # Pick region
   $region ||= 'us-west-2';
   my $endpoint = ($region =~ /^http/) ? $region 
                                       :  "http://ec2.${region}.amazonaws.com";
   $region{$id} = $region;

   # Pick bucket (needed to pass to EC2 instances)
   $bucket ||= lc("TPP-$accessKey");
   $bucket{$id} = $bucket;


   # Initialize EC2 interface
   eval 
      {
      my $ec2 = $ec2{$id} = VM::EC2->new( -access_key => $accessKey,
                                          -secret_key => $secretKey,
                                          -endpoint => $endpoint,
                                          -raise_error => 1, );
      unless ( grep { $_->groupName eq 'AMZTPP' } $ec2->describe_security_groups() )
         {
         $ec2->create_security_group( '-group_name' => 'AMZTPP', 
                                      '-group_description' => 'AMZTPP security group (firewall rules)' );
         }
      };
   $log->logdie( "unable to initialize EC2 Manager: $@" ) if $@;
   $log->debug( "EC2 Manager initialized" );

   # Since spot requests don't immediately show when requested keep track 
   # of the ones that we have requested or canceled
   $spots{$id} = {};
   $cancels{$id} = {};

   return $self;
   }

#
# Start another EC2 image
#
sub start
   {
   my ( $self, $cnt, $image, $type, $key, $grp, $spot ) = @_;
   my $id = id $self;

   # The EC2 instance is expected to have Cloud-Init installed, which is
   # a way to pass some work to be done first boot of the image.
   # Here we pass a bash script in the userdata to do some setup.  
   my $accessKey = $self->ec2()->access_key();
   my $secretKey = $self->ec2()->secret();
   my $region    = $self->region();
   my $bucket    = $self->bucket();
   my $userData  = <<BOOTSTRAP;
#!/bin/bash

chmod a+rwxt /tmp

apt-get update
apt-get upgrade -y

echo '$accessKey' >  /root/.awssecret
echo '$secretKey' >> /root/.awssecret
echo '$region'    >> /root/.awssecret
echo '$bucket'    >> /root/.awssecret
chmod 600 /root/.awssecret

echo '[default]'               >  /root/.s3cfg
echo 'access_key = $accessKey' >> /root/.s3cfg
echo 'bucket_location = US'    >> /root/.s3cfg
echo 'secret_key = $secretKey' >> /root/.s3cfg
echo 'use_https = True'        >> /root/.s3cfg
chmod 600 /root/.s3cfg

cd /tmp
wget -t 30 --waitretry=3 http://s3.amazonaws.com/spctools/AMZTPP.tar.gz || true
if [ -e AMZTPP.tar.gz ]; then
   stop amztppd || true
   tar xvzf AMZTPP.tar.gz
   cd AMZTPP-*
   perl Makefile.PL INSTALLSITESCRIPT=/opt/tpp/bin
   make
   make install
   cp init/amztppd.conf /etc/init
fi

# Add logging links to Apache config
sed -i '/# ISB-/a Alias /amztppd-service.log /var/log/amztppd-service.log' /etc/apache2/sites-available/default
sed -i '/# ISB-/a Alias /amztppd.log /var/log/amztppd.log' /etc/apache2/sites-available/default
/etc/init.d/apache2 restart

start amztppd
BOOTSTRAP

   $image ||= ($self->getImages())[0];

   my @i;
   my %args = ();
   $args{-image_id}       = $image;
   $args{-instance_type}  = $type || 'm1.xlarge';
   $args{-key_name}       = $key if ( $key );
   $args{-security_group} = ($grp ? [ "AMZTPP", $grp ] : "AMZTPP" );
   $args{-user_data} = $userData;
   if ( $spot )
      {
      $args{-instance_count} = $cnt || 1; 
      $args{-type}           = 'one-time';
      $args{-spot_price}     = $spot;

      $log->debug( "requesting $cnt EC2 spot instances of $image type $args{-instance_type}");
      eval {  @i = $self->ec2()->request_spot_instances( %args );  };
      $log->error( "requesting EC2 spot instance(s): $@" ) if ( $@ );
      foreach ( @i ) 
         { 
         $log->debug( $_->spotInstanceRequestId . " spot instance requested" );
         $spots{$id}->{$_->spotInstanceRequestId} = $_; 
         }
      }
   else
      {
      $args{-min_count} = 1; 
      $args{-max_count} = $cnt; 
      $args{-shutdown_behaviour} = 'terminate';

      $log->debug( "starting $cnt EC2 instances of $image type $args{-instance_type}");
      eval { @i = $self->ec2()->run_instances( %args ); };
      $log->error( "starting EC2 instance(s): $@" ) if ( $@ );
      foreach ( @i ) 
         { 
         $log->debug( $_->instanceId . " instance started" );
         }
      }

   # Tag new instances as an AMZTPP instance
   # NOTE: spot instances don't get tagged when they start.  So you can't
   #       use this to identify them.  Use security groups instead.
   foreach ( @i ) 
      { 
      $_->add_tags( 'AMZTPP' => '1' ); 
      }

   return scalar @i;
   }

#
# Return the number of started (pending + open spot requests) and 
# running EC2 instances 
#
sub counts
   {
   my ( $self )  = @_;

   my @i = $self->getInstances();
   my @s = $self->getSpotRequests();

   # Find running EC2 instances
   my $r = grep { $_->current_state() eq 'running' }  @i;

   # Find pending ec2 and open spot requests
   my $p = grep { $_->current_state() eq 'pending' }  @i;
   my $o = grep { !$_->instanceId() && $_->state() eq 'open' }  @s;

   return( $p + $o, $r );
   }

#
# Terminate one or more EC2 instances/spot requests associated with AMZTPP.
#
# @arg   Number of instances to terminate.  If undef it will request that all
#        AMZTPP EC2 pending or running instances be terminated.  
#
sub terminate
   {
   my ( $self, $cnt ) = @_;
   my $id = id $self;

   # Find AMZTPP instances that are pending(0) or running(16)
   my @i = grep { $_->current_state =~ /running|pending/ } $self->getInstances();
   @i = @i[0..--$cnt] if ( $cnt && $cnt < @i );  # all, or some?
   if ( @i )
      {
      $log->debug( "terminating " . scalar(@i) . " instances" );
      my @s = eval { $self->ec2()->terminate_instances( -instance_id => \@i ) };
      $log->error( "unable to terminate instance(s): $@" ) if ( $@ );
      $cnt -= scalar(@s);
      }
   return scalar(@i) if ( defined $cnt && $cnt == 0 );

   # Find AMZTPP spot instances that we haven't potentially terminated
   my @s = grep { $_->state() =~ /open|active|pending/ } $self->getSpotRequests();
   @s = @s[0..--$cnt] if ( $cnt && $cnt < @s );  # all, or some?
   if ( @s )
      {
      $log->debug( "canceling " . scalar(@s) . " spot instances" );
      eval { $self->ec2()->cancel_spot_instance_requests( @s ) };
      $log->error( "unable to cancel spot instances: $@" ) if ( $@ );
      }

   return scalar(@i) + scalar(@s);
   } 

#
# Cancel one or more spot requests
#
# @arg   Number of instances to cancel.  If undef it will request that all
#        AMZTPP EC2 pending instances or spot requests be cancelled.  
#
sub cancel
   {
   my ( $self, $cnt ) = @_;
   my $id = id $self;

   # Find AMZTPP instances that are pending(0)
   my @i = grep { $_->current_state =~ /pending/ } $self->getInstances();
   @i = @i[0..--$cnt] if ( $cnt && $cnt < @i );  # all, or some?
   if ( @i )
      {
      $log->debug( "canceling " . scalar(@i) . " instances" );
      my @s = eval { $self->ec2()->terminate_instances( -instance_id => \@i ) };
      $log->error( "unable to cancel instance(s): $@" ) if ( $@ );
      $cnt -= scalar(@s);
      }
   return scalar(@i) if ( defined $cnt && $cnt == 0 );

   # Find AMZTPP spot instances that haven't started
   my @s = grep { $_->state() =~ /open|pending/ } $self->getSpotRequests();
   @s = @s[0..--$cnt] if ( $cnt && $cnt < @s );  # all, or some?
   if ( @s )
      {
      $log->debug( "canceling " . scalar(@s) . " spot instances" );
      foreach ( @s ) { $cancels{$id}->{$_->spotInstanceRequestId()} = $_ } 
      eval { $self->ec2()->cancel_spot_instance_requests( map { $_->spotInstanceRequestId() } @s ) };
      $log->error( "unable to cancel spot instances: $@" ) if ( $@ );
      }

   return scalar(@i) + scalar(@s);  	
   }

#
# Returns a list of available TPP images, sorted by their path.  This 
# should mean the most recent/best is the first.
#
sub getImages
   {
   my ( $self ) = @_;

   # Locate images by owner and location
   # FIXME: owner of images should be a spctools@systemsbiology user not me
   # TODO: use tags instead?
   my $filters = { 'owner-id'          => '178066177892',
                   'manifest-location' => 'spctools-images*',
      };
   my @i = $self->ec2()->describe_images( -filter => $filters );
   @i || $log->logdie( "Unable to locate any TPP images" );

   # TPP images are named with a date so sort them to get the most recent first
   return ( map { $_->imageId() } 
            reverse sort { $a->imageLocation() cmp $b->imageLocation() } @i );
   }

#
# Returns a list of our "active" EC2 instances/spot requests
#
sub getInstances
   {
   my ( $self ) = @_;

   my $filters = { 'instance-state-name' =>  [ 'running', 'pending', 'failed' ],
# Issue with spot instances and tags not propagating, so just use security group
#   	           'tag:AMZTPP' => '*',
                   'group-name' => 'AMZTPP', 
      };

   my @i = $self->ec2()->describe_instances( -filter => $filters );
#  $log->debug( scalar(@i) . " EC2 instances reported" );
#   foreach ( @i )
#      {
#      $log->debug( "recognized instance " . $_->instanceId() . 
#                   " state " . $_->instanceState() );
#      }

   return ( @i );
   }

#
# Returns a list of our "active" EC2 spot instance requests
#
sub getSpotRequests
   {
   my ( $self ) = @_;

   # Find AMZTPP open spot requests. Remember tags on spot requests don't
   # propagate onto the instances
   my $filter = {  'state' => [ 'open', 'active', 'failed' ],
                   'tag:AMZTPP' => '*', 
   };

   my @s = $self->ec2()->describe_spot_instance_requests( -filter => $filter );
   my %i = map { $_->spotInstanceRequestId() => $_ } @s;
#  $log->debug( scalar(keys %i) . " EC2 spot instances reported" );

   # Add any additional spot instances that we've requested and aren't listed
   while ( my ( $id, $spot ) =  each %{ $self->spots() } )
      {
      next if ( $i{$id} );
      eval 
         { 
         $spot->refresh();
         if ( $spot->state() =~ /open|active|failed/ )
            {
#           $log->debug( "adding unreported spot request $id to list" );
            $i{$id} = $spot;
            }
         };
      $@ && $log->error( "couldn't refresh spot request $id: $@" );
      }
   $spots{id $self} = \%i;

   # Remove any spot instances that we've cancelled
   while ( my ( $id, $spot ) =  each %{ $self->cancels() } )
      {
      delete $i{$id} if ( $i{$id} );
      }

   return values %i;
   }

1;
