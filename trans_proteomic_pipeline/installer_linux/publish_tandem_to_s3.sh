#!/bin/bash

#
# a little script to build tandem (and x!!tandem if possible) and put it out on S3
# intended for use with Hadoop and EC2, probably useful for other scenarios too
# Apache Licensed, and Copyright(C) 2010 Insilicos LLC all rights reserved
#
if [ "" == "$AWS_ACCESS_KEY_ID" ] ; then
	echo "you need to export AWS_ACCESS_KEY_ID=<your AWS ID>"
fi
if [ "" == "$AWS_SECRET_ACCESS_KEY" ] ; then
	echo "you need to export AWS_SECRET_ACCESS_KEY=<your AWS secret key>"
fi
if [ "" == "$S3BUCKET" ] ; then
	echo "you need to export S3BUCKET=<your S3 bucket for public access>"
fi

current=$(pwd)
echo $current
cd $(dirname $0)
wrkdir=$(pwd)
echo $wrkdir
cd ~
if  [ ! -e "s3sync" ]; then
wget http://s3.amazonaws.com/ServEdge_pub/s3sync/s3sync.tar.gz
--18:31:18--  http://s3.amazonaws.com/ServEdge_pub/s3sync/s3sync.tar.gz
tar -xzf s3sync.tar.gz
fi

# determine system architecture
. $wrkdir/tandem_arch.sh

BUILD_DIR=$(cd $wrkdir/../src ; make tell_obj_arch)

export PATH=$PATH:/usr/local/mpich2/bin

cd $wrkdir/../src ; make xtandem$1 ; make bbtandem$1

cd ~

echo "publishing mrtandem to $S3BUCKET:$TANDEM_ARCH"
s3sync/s3cmd.rb put $S3BUCKET:$TANDEM_ARCH/mrtandem $BUILD_DIR/tandem x-amz-acl:public-read
if [ -e $BUILD_DIR/bbtandem ] ; then
	echo "publishing bbtandem to $S3BUCKET:$S3DIR/$(uname -m)"
	. $wrkdir/tandem_arch.sh mpi
	s3sync/s3cmd.rb put $S3BUCKET:$TANDEM_ARCH/bbtandem $BUILD_DIR/bbtandem x-amz-acl:public-read
else
	echo "bbtandem not built"
fi
# and also post the code that helps clients decide which version to download
echo "publishing tandem_arch.sh installation aid to $S3BUCKET"
s3sync/s3cmd.rb put $S3BUCKET:tandem_arch.sh $wrkdir/tandem_arch.sh x-amz-acl:public-read
cd $current
