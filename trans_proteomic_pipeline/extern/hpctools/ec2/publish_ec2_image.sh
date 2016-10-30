#!/bin/bash 
#
# Script for publishing a TPP Amazon EC2 image. 
#

set -e               # Exit on error

export BUCKET=spctools-images-us-west-2

export EC2_URL=${EC2_URL:-http://ec2.us-west-2.amazonaws.com}
export AWS_ACCESS_KEY_ID=$AWS_ACCESS_KEY_ID
export AWS_SECRET_ACCESS_KEY="$AWS_SECRET_ACCESS_KEY"

export EC2_HOME=$(ls -d /tmp/ec2-ami-tools-*)
export PATH=$PATH:${EC2_HOME}/bin


DESC=`/opt/tpp/bin/tpp_hostname 'versionInfo!'`
NAME=`echo "$DESC" | perl -n -e 'print "TPP-$1.$2" if /TPP v(\S+) .* rev (\d+)/'`
YYYYMMDD=`date  +"%Y%m%d"`
FOLDER="$NAME-$YYYYMMDD"

echo "               BUCKET = $BUCKET"
echo "              EC2_URL = $EC2_URL"
echo "    AWS_ACCESS_KEY_ID = $AWS_ACCESS_KEY_ID"
echo "AWS_SECRET_ACCESS_KEY = $AWS_SECRET_ACCESS_KEY"
echo
echo "  NAME = $NAME"
echo "  DESC = $DESC"
echo "FOLDER = $FOLDER"

source /etc/profile.d/java.sh

if [ "$AWS_ACCESS_KEY_ID" == "" ]; then
   echo "Error: AWS_ACCESS_KEY_ID not set cannot publish image!!!"
   exit 1
fi
if [ "$AWS_SECRET_ACCESS_KEY" == "" ]; then
   echo "Error: AWS_SECRET_ACCESS_KEY not set cannot publish image!!!"
   exit 1
fi
if [ "$EC2_CERT" == "" ]; then
   echo "Error: EC2_CERT not set cannot publish image!!!"
   exit 1
fi
if [ "$EC2_PRIVATE_KEY" == "" ]; then
   echo "Error: EC2_PRIVATE_KEY not set cannot publish image!!!"
   exit 1
fi

echo "=== Uploading image..."
ec2-upload-bundle -b "$BUCKET/$FOLDER" -m image.manifest.xml \
   --acl 'public-read' -a $AWS_ACCESS_KEY_ID -s "$AWS_SECRET_ACCESS_KEY" \
   --region us-west-2
echo "=== Done uploading image"
echo

echo "=== Registering image..."
apt-get --force-yes -y install ec2-api-tools
ec2-register -C $EC2_CERT -K $EC2_PRIVATE_KEY \
   $BUCKET/$FOLDER/image.manifest.xml -n $FOLDER -d "$DESC"
echo "=== Done registering image"
echo
