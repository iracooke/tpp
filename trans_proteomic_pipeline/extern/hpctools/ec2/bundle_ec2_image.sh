#!/bin/bash 
#
# Script for bundling a TPP Amazon EC2 image. 
#

set -e          # Exit on error

# Use sudo to run commands as root
export EC2_CERT=${EC2_CERT:-`ls /tmp/cert-*.pem`}
export EC2_PRIVATE_KEY=${EC2_PRIVATE_KEY:-`ls /tmp/pk-*.pem`}
export EC2_URL=${EC2_URL:-http://ec2.us-east-1.amazonaws.com}
export AWS_USER_ID="${AWS_USER_ID}"

echo "    AWS_USER_ID = $AWS_USER_ID"
echo "       EC2_CERT = $EC2_CERT"
echo "EC2_PRIVATE_KEY = $EC2_PRIVATE_KEY"
echo "        EC2_URL = $EC2_URL"

source /etc/profile.d/java.sh

if [ ! -f "$EC2_CERT" ]; then
   echo "Error: EC2_CERT not found cannot bundle image!!!"
   echo "Please copy (via scp) your public Amazon EC2 certificate"
   echo "to this instance and set the enviroment variable to the"
   echo "path of this file."
   exit 1
fi
if [ ! -f "$EC2_PRIVATE_KEY" ]; then
   echo "Error: EC2_PRIVATE_KEY not found cannot bundle image!!!"
   echo "Please copy (via scp) your private Amazon EC2 private key"
   echo "to this instance and set the enviroment variable to the"
   echo "path of this file."
   exit 1
fi
if [ "$AWS_USER_ID" == "" ]; then
   echo "Error: AWS_USER_ID not set cannot bundle image!!!"
   echo "Please set the enviroment variable to your Amazon account ID"
   exit 1
fi

echo "=== Install EC2 tools..."
# Ubuntu repository is so out of date...
#sudo apt-get install -y ec2-ami-tools
set -x
cd /tmp
wget http://s3.amazonaws.com/ec2-downloads/ec2-ami-tools.zip
unzip ec2-ami-tools.zip
export EC2_HOME=$(ls -d /tmp/ec2-ami-tools-*)
export PATH=$PATH:${EC2_HOME}/bin
sudo apt-get install -y kpartx
set +x
echo "=== Done installing EC2 tools"
echo

echo "=== Sanitizing image..."
set -x
/etc/init.d/apache2 stop
killall amztppd || :
rm -f /opt/tpp/users/*/* /opt/tpp/users/.s3cfg
rm -f /root/.*awssecret* $HOME/.*awssecret* ~/.*awssecret*
rm -f /root/.*s3cfg $HOME/.*s3cfg* ~/.*s3cfg*
rm -f /root/.*hist* $HOME/.*hist* ~/.*hist*
rm -f /var/log/*.gz
rm -f /var/log/amztpp*
rm -f /var/log/apache2/*
rm -f /var/log/cloud-init.log
rm -f /var/log/messages
rm -f /var/log/lastlog
rm -f /var/log/daemon.log
set +x
echo "=== Done sanitizing image"
echo

echo "=== Bundling image..."
#
# READ THIS:
# https://forums.aws.amazon.com/thread.jspa?messageID=341076#
# https://github.com/mitchellh/packer/pull/256
#
# - ec2-bundle-vol will silently ignore any .gpg and .pem files.  
#   This is done for security purposes, but it is a problem 
#   because it drops all files in /etc/ssl and /use/share/keyrings. 
#   Consequently, an Ubuntu image created with the default command
#   will fail to validate signed packages (because the Ubuntu 
#   public key has disappeared).
#
set -x
ec2-bundle-vol -k $EC2_PRIVATE_KEY -c $EC2_CERT -u "$AWS_USER_ID" \
   -e /tmp -r x86_64 \
   --no-filter --exclude /tmp,/mnt,/home
set +x

# Various attempts to get this to work properly..
#
# KEYS=`find / -name "*.pem" | grep -v "^/tmp" | grep -v "^/mnt" | grep -v "^/home" | tr '\n' ','`
# --include $KEYS

#   --include /etc/ec2/amitools/cert-ec2.pem \

#   -i /etc/ec2/amitools/cert-ec2.pem,/etc/ssl/certs/,/etc/apt/trusted.gpg,/etc/apt/trustdb.gpg \
#   -i /etc/apt/trusted.gpg,/etc/apt/secring.gpg,/etc/apt/trustdb.gpg
#
# sed -i.bak -E '/^.*(\*\.gpg|\*\.pem).*$/d' $EC2_AMITOOL_HOME/lib/ec2/platform/base/constants.rb
#
# -i /home/,/etc/ec2/amitools/cert-ec2.pem,/etc/ssl/certs/,/etc/apt/trusted.gpg,/etc/apt/trusted.gpg~,/etc/apt/trustdb.gpg

echo "=== Done bundling image"
echo
