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
# $Id: SQSManager.pm 6003 2013-01-11 19:52:16Z slagelwa $
#
package TPP::AWS::SQSManager;
use strict;
use warnings;

use Amazon::SQS::Simple;
use Class::InsideOut qw( :std );
use LWP::UserAgent;
use Module::Load;
use Sys::Hostname;
use YAML ();

use TPP::AWS::Logger qw( $log );


#-- @GLOBALS ----------------------------------------------------------------#

our $REVISION = (q$Revision: 6003 $ =~ /(\d+)/g)[0] || '???'; 

use constant SQS_MSG_MAX_LEN     => 65536;
use constant SQS_MAX_TIMEOUT_SEC => 43200;              # 12 hours
use constant SQS_MAX_MSG_RETENT  => 1209600;            # 14 days



#-- @ATTRIBUTES -------------------------------------------------------------#

readonly sqs        => my %sqs;		# Reference to Amazon::SQS
readonly upQueue    => my %upQueue;	# AWS queue for upload messages
readonly upCounts   => my %upCounts;	# counts of visible/invisible msgs 
readonly srvQueue   => my %srvQueue;	# AWS queue for service messages
readonly srvCounts  => my %srvCounts;	# counts of visible/invisible msgs 
readonly downQueue  => my %downQueue;	# AWS queue for download messages
readonly downCounts => my %downCounts;	# counts of visible/invisible msgs 
readonly doneQueue  => my %doneQueue;	# AWS queue for done messages
readonly doneCounts => my %doneCounts;	# counts of visible/invisible msgs
readonly rcvdIds    => my %rcvdIds;	# Hash of message ids received

private  active     => my %active;	# Active count
private  prefix     => my %prefix;	# Prefix for name of queues


#-- @PUBLIC -----------------------------------------------------------------#

sub new
   {
   my ( $class, $accessKey, $secretKey, $region, $service ) = @_;
   $region  ||= 'us-west-2';
   $service ||= 'AMZTPP';
   
   # Create object
   my $self = Class::InsideOut::new( $class );
   my $id   = id $self;
   $rcvdIds{$id} = {};
   $active{$id}  = 0;
   $prefix{$id}  = "$service-$accessKey";

   # Setup SQS
   my $sqs  = $sqs{$id} = Amazon::SQS::Simple->new( $accessKey, $secretKey );
   # FIXME: force endpoint based on region as SQS::Simple doesn't support
   #        it (yet)
   $sqs->{Endpoint} = "http://$region.queue.amazonaws.com";
   my $list = $sqs->ListQueues( QueueNamePrefix => $prefix{$id} ) || [];
   foreach my $q ( @$list )
      {
      $log->debug( "found queue endpoint: " . $q->Endpoint() );
      $upQueue{$id}   = $q if ( $q->Endpoint() =~ /-upload$/ );
      $srvQueue{$id}  = $q if ( $q->Endpoint() =~ /-service$/ );
      $downQueue{$id} = $q if ( $q->Endpoint() =~ /-download$/ );
      $doneQueue{$id} = $q if ( $q->Endpoint() =~ /-done$/ );
      }

   $upCounts{$id}   = [ undef, undef ],
   $srvCounts{$id}  = [ undef, undef ],
   $downCounts{$id} = [ undef, undef ],
   $doneCounts{$id} = [ undef, undef ],
   
   $log->debug( "SQSManager initialized" );
   return $self;
   }

#
# Create the necessary queues (unless they already exist)
#
sub createQueues
   {
   my ( $self ) = @_;
   
   # Create the SQS queues. If the queues have recently been deleted it may 
   # take up to 60 secs before they can be recreated, so retry on failure
   for ( 1..3 )
      {
      eval { $self->_createQueues() };
      last unless ( $@ );       
      $log->error( "$@\n" );
      ( $_ >= 3 ) ? $log->logdie( "unable to create SQS queues, giving up" )
                  : $log->error( "unable to create SQS queues, retrying in 30 secs" );
      sleep(30);
      }
      
   return 1;
   }
   
#
# List queues created
#
sub list
   {
   my ( $self ) = @_;
   my $id = id $self;
   my $sqs = $sqs{$id};
   
   my @s;
   eval
      {
      my $list =  $sqs->ListQueues( QueueNamePrefix => $prefix{$id} ) || [];
      #my $list =  $sqs->ListQueues() || [];
      foreach my $q ( @$list )
         {
         my $a = $q->GetAttributes(); 
         ( $a->{Name} ) = ( $q->Endpoint() =~ /\/([^\/]*?)$/ );
         push @s, $a if ( $a->{Name} =~ /upload|download|service|done/ );
         }
      };
   if ( my $e = $@ )
      {
      return [] if ( $e =~ /specified queue does not exist/ );
      $log->logdie( $e );
      }
      
   return \@s;
   }

#
# Queue an upload request for a service
#
sub queueUpload
   {
   my ( $self, $srv ) = @_;
   my $id = id $self;
   
   $active{$id}++;
   $srv->{CLIENT}  = hostname();
   $srv->{UID}     = "$active{$id}::$srv->{CLIENT}::$$";
   $srv->{CLIENT_SUBMIT} = time();
   my $buf = YAML::Dump($srv);
   (length $buf <= SQS_MSG_MAX_LEN)
      or $log->logdie( "Error: maximum length of SQS message exceeded\n" );
      
   $log->debug( "queued service upload" );
   return $self->upQueue()->SendMessage( $buf );
   }

sub dequeueUpload
   {
   my ($self) = @_;
   
   my $srv = $self->_rcvmsg( $self->upQueue() );
   if ( $srv )
      {
      $log->debug( "dequeued service upload" );
      }
   return $srv;
   }
   
#
# Queue an run for a service
#
# @arg  reference to service to queue
# @arg  optional delay in seconds (max 900)
#
sub queueService
   {
   my ( $self, $srv, $delay ) = @_;
   my $id = id $self;
   
   $active{$id}++;
   my $buf = YAML::Dump($srv);
   (length $buf <= SQS_MSG_MAX_LEN)
      or $log->logdie( "Error: maximum length of SQS message exceeded\n" );
      
   $log->debug( "queueing service run" );
   if ( defined $delay )
      {
      # FIX: until Amazon::SQS::Simple is updated to the latest API version
      # override what version we are so that this will work
      return $self->srvQueue()->SendMessage( $buf, 'DelaySeconds' => $delay,
                                                   'Version' => '2011-10-01' );
      }
   else
      {
      return $self->srvQueue()->SendMessage( $buf );
      }
   }

#
# Updates (extends) service's visibility
#
sub extendService
   {
   my ( $self, $srv, $n ) = @_;     
   
   $log->debug( "extending visibility of service message" );
   my $retry = 3;
   do {
      eval { 
         $self->srvQueue()->ChangeMessageVisibility( $srv->{MSG_HANDLE}, $n );
         };
      return unless ( $@ );
         
      $log->warn( "message change visibility failed (${retry}x retries): $@" );
      sleep(2);                                 # ...wait a bit
      } while ( --$retry );
   }

sub dequeueService
   {
   my ($self) = @_;
   
   my $srv = $self->_rcvmsg( $self->srvQueue() );
   if ( $srv )
      {
      $srv->{SERVER}         = hostname();
      $srv->{INSTANCE_ID}    = _getInstanceID();
      $srv->{SERVER_RECEIVE} = time();
      $log->debug( "dequeued service run" );
      }
   return $srv;
   }
  
#
# Queue download of service
#
sub queueDownload
   {
   my ( $self, $srv ) = @_;
   
   $srv->{SERVER_SUBMIT} = time();
   my $buf = YAML::Dump($srv);
   (length $buf <= SQS_MSG_MAX_LEN)
      or $log->logdie( "Error: maximum length of SQS message exceeded\n" );
      
   $log->debug( "queueing service download" );
   return $self->downQueue()->SendMessage( $buf );
   }
   
sub dequeueDownload
   {
   my ($self) = @_;
   
   my $srv = $self->_rcvmsg( $self->downQueue() );
   if ( $srv )
      {
      $srv->{CLIENT_RECEIVE} = time();
      $log->debug( "dequeued service download" );
      }
   return $srv;
   }
  
#
# Queue service done
#
sub queueDone
   {
   my ( $self, $srv ) = @_;
   my $id = id $self;
   
   $active{$id}++;
   my $buf = YAML::Dump($srv);
   (length $buf <= SQS_MSG_MAX_LEN)
      or $log->logdie( "Error: maximum length of SQS message exceeded\n" );
      
   $log->debug( "queueing service done" );
   return $self->doneQueue()->SendMessage( $buf );
   }

sub dequeueDone
   {
   my ( $self, $cnt ) = @_;
   
   my @srvs = $self->_rcvmsg( $self->doneQueue(), $cnt );
   $log->debug( "dequeued " . scalar(@srvs) . " done services" );
   return @srvs;
   }

#
# Requeue a done message by changing its visibility 
#
sub requeueDone
   {
   my ( $self, $srv ) = @_;     
   
   $log->debug( "requeuing done service $srv->{UID}" );
   
   my $retry = 3;
   do {
      eval { 
         $self->doneQueue()->ChangeMessageVisibility( $srv->{MSG_HANDLE}, 0 );
         };
      return unless ( $@ );
         
      $log->warn( "message change visibility failed (${retry}x retries) $@" );
      sleep(2);                                 # ...wait a bit
      } while ( --$retry );
      
   }

#
# Determine if any queue is active (has non-visible message).  Ignores done
# queue. Does this by getting the approximate number of visible and not visible
# messages for the SQS queues and returns count of active (not-visible) 
# messages. Returns undef if counts couldn't be queried.
#
sub hasActiveQueues
   {
   my ($self) = shift;
   my $id = id $self; 
   
   my $cnt = 0;
   eval
      {
      my $up = $self->upQueue()->GetAttributes();
      $upCounts{$id} = [ $up->{ApproximateNumberOfMessages},
                         $up->{ApproximateNumberOfMessagesNotVisible} ];
      map { $cnt += $_ } @{ $upCounts{$id} };
      
      # FIX: lie about version to get newly added attributes
      my $srv = $self->srvQueue()->GetAttributes( 'Version' => '2011-10-01' );
      $srvCounts{$id} = [ $srv->{ApproximateNumberOfMessages},
                          $srv->{ApproximateNumberOfMessagesNotVisible} ];
      map { $cnt += $_ } @{ $srvCounts{$id} };
      $cnt += ($srv->{ApproximateNumberOfMessagesDelayed} || 0);
      
      my $down = $self->downQueue()->GetAttributes();
      $downCounts{$id} = [ $down->{ApproximateNumberOfMessages},
                           $down->{ApproximateNumberOfMessagesNotVisible} ];
      map { $cnt += $_ } @{ $downCounts{$id} };
      
      my $done = $self->doneQueue()->GetAttributes();
      $doneCounts{$id} = [ $done->{ApproximateNumberOfMessages},
                           $done->{ApproximateNumberOfMessagesNotVisible} ];
      # We DON'T want to count done messages
      
      if ( $log->is_debug )
         {
         $log->debug( sprintf("approximately %d/%-d messages in upload queue",
                      @{ $upCounts{$id} } ) );
         $log->debug( sprintf("approximately %d/%-d messages in srv queue",
                      @{ $srvCounts{$id} } ) );
         $log->debug( sprintf("approximately %d/%-d messages in download queue",
                      @{ $downCounts{$id} } ) );
         $log->debug( sprintf("approximately %d/%-d messages in done queue",
                      @{ $doneCounts{$id} } ) );
         $log->debug( "approximately $_ messages delayed" )
            if ( $_ = $srv->{ApproximateNumberOfMessagesDelayed} );
         }
      };
   if ( $@ )
      {
      $log->error( "error querying SQS queues: $@" );
      return undef;    	
      }

   return( $cnt );
   }   
   
#
# Delete a message from a queue
#
sub deleteMessage
   {
   my ( $self, $q, $srv ) = @_;
   $log->debug( "deleting message from queue" );
   
   my $retry = 3;
   do {
      eval { 
         $q->DeleteMessage( $srv->{MSG_HANDLE} );
         $srv->{MSG_HANDLE} = undef;
         }; 
      return unless ( $@ );
         
      $log->warn( "message deletion failed (${retry}x retries) $@" );
      sleep(2);                                 # ...wait a bit
      } while ( --$retry );
      
   $log->logdie( "message deletion failed too many times, giving up $@" );
   }

#
# Delete all of the queues
#
sub delete
   {
   my ($self) = shift;
   my $name;
    
   my $cnt = 0;
   if ( $self->upQueue() )
      {
      ($name = $self->upQueue()->Endpoint()) =~ s/.*\///;
      $log->debug( "deleting AWS Queue $name" );
      $self->upQueue()->Delete();
      $upQueue{ id $self } = undef;
      $cnt++;
      }
   
   if ( $self->srvQueue() )
      {
      ($name = $self->srvQueue()->Endpoint()) =~ s/.*\///;
      $log->debug( "deleting AWS Queue $name" );
      $self->srvQueue()->Delete();
      $srvQueue{ id $self } = undef;
      $cnt++;
      }
      
   if ( $self->downQueue() )
      {
      ($name = $self->downQueue()->Endpoint()) =~ s/.*\///;
      $log->debug( "deleting AWS Queue $name" );
      $self->downQueue()->Delete();
      $downQueue{ id $self } = undef;
      $cnt++;
      }
      
   if ( $self->doneQueue() )
      {
      ($name = $self->doneQueue()->Endpoint()) =~ s/.*\///;
      $log->debug( "deleting AWS Queue $name" );
      $self->doneQueue()->Delete();
      $doneQueue{ id $self } = undef;
      $cnt++;
      }
      
   return $cnt;
   }


#-- @PRIVATE ----------------------------------------------------------------#
   
#
# Create the queues
#
sub _createQueues
   {
   my ( $self ) = @_;
   my $id = id $self;
   
   # Get/create queues w/max timeout(expect long service times)
   my %opts = ( DefaultVisibilityTimeout => SQS_MAX_TIMEOUT_SEC,
	        MessageRetentionPeriod => SQS_MAX_MSG_RETENT );
   if ( !$self->upQueue() )
      {
      $log->debug( "creating AMZTPP upload queue" );
      $upQueue{$id} = $self->sqs()->CreateQueue( "$prefix{$id}-upload", %opts );
      }
   if ( !$self->srvQueue() )
      {
      $log->debug( "creating AMZTPP service queue" );
      $srvQueue{$id} = $self->sqs()->CreateQueue( "$prefix{$id}-service", %opts,
         DefaultVisibilityTimeout => (60 * 5),
         );
      }
   if ( !$self->downQueue() )
      {
      $log->debug( "creating AMZTPP download queue" );
      $downQueue{$id} = $self->sqs()->CreateQueue( "$prefix{$id}-download", %opts );
      }
   if ( !$self->doneQueue() )
      {
      $log->debug( "creating AMZTPP done queue" );
      $doneQueue{$id} = $self->sqs()->CreateQueue( "$prefix{$id}-done",
         DefaultVisibilityTimeout => (60 * 10),
         MessageRetentionPeriod => SQS_MAX_MSG_RETENT,
         );
      }
   } 

#
# Retrieve a message from a queue
#
sub _rcvmsg
   {
   my ( $self, $queue, $cnt ) = @_;
   $cnt ||= 1;
   
   # Loop receiving messages
   my @srvs = ();
   while ( !@srvs )
      {
      my @msgs = eval { $queue->ReceiveMessage( MaxNumberOfMessages => $cnt ) };
      $log->error( $@ ) if ( $@ );
      last unless @msgs;
      
      foreach my $msg ( @msgs )
         {
         # Inflate msg into object
         my $msgID = $msg->MessageId();
         $log->debug( "received msg id: $msgID" );
      
         # Skip if duplicate 
         if ( my $i = $self->rcvdIds()->{$msgID}++ ) 
            {
            $log->debug( "skipping duplicate message received (x$i)" );
            next;  
            }
      
         # Dynamically load module ("use") to ensure object's module is loaded
         my $srv = YAML::Load( $msg->MessageBody() );
         $srv->{MSG_HANDLE} = $msg->ReceiptHandle();
      
         my $class = ref($srv);
         load $class;
         $log->logdie($@) if ( $@ );
         push @srvs, $srv;
         
         # MessageID's should be unique but check again
#        if ( my $i = $self->rcvdIds()->{$srv->{UID}}++ ) 
#           {
#           $log->warn( "duplicate message received (x$i) $srv->{UID}" );
#           next;
#           }
         }
      }
      
   return wantarray ? @srvs : $srvs[0];
   }

#
# Get the EC2 instance's unique ID
#
sub _getInstanceID
   {
   my $ua  = LWP::UserAgent->new( timeout => 2 );
   my $res =  $ua->get("http://169.254.169.254/latest/meta-data/instance-id");
   return $res->decoded_content;
   }

1;
