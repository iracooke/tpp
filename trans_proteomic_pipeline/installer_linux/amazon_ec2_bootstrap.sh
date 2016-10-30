#!/bin/bash 


# quickstart:
#
# if you already have wget on your EC2 instance, you can start the
# build and installation process by executing the following line:
#
# wget  http://sashimi.svn.sourceforge.net/viewvc/sashimi/trunk/trans_proteomic_pipeline/installer_linux/amazon_ec2_bootstrap.sh; sh amazon_ec2_bootstrap.sh your_AWS_ACCESS_KEY_ID your_AWS_SECRET_ACCESS_KEY your_s3_bucket_name
# (substituting your actual values for "your_AWS_ACCESS_KEY_ID", 
#  "your_AWS_SECRET_ACCESS_KEY:", and "your_s3_bucket_name")





#
# script for boostrapting the Amazon EC2 TPP build process

# copy this script to your EC2 instance and run it to begin the build
# and installation process

# see amazon_ec2_notes for further details.

# args are:
#    your_AWS_ACCESS_KEY_ID your_AWS_SECRET_ACCESS_KEY your_s3_bucket_name
#


#
# sanity check
#
if [ $# -ne 3 ]
then
  echo "Usage: `basename $0` your_AWS_ACCESS_KEY_ID your_AWS_SECRET_ACCESS_KEY your_s3_bucket_name"
  exit $WRONG_ARGS
fi




#
# identify the distribution name and version, and make sure that we
# support it
#

if [ -e /etc/lsb-release ]; then
    # we might be on ubuntu
    . /etc/lsb-release
    # DISTRIB_ID and DISTRIB_RELEASE should now be set
elif [ -e /etc/redhat-release ]; then
    grep "CentOS" /etc/redhat-release > /dev/null
    if [ $? -eq 0 ]; then
	DISTRIB_ID="CentOS"
        grep "Centos release 5.2 (Final)" /etc/redhat-release > /dev/null
	if [ $? -eq 0 ]; then
	    DISTRIB_RELEASE="5.2"
	fi
    fi
elif [ -e /etc/debian_version ]; then
	# Debian - just pretend its ubuntu
	DISTRIB_ID="Ubuntu"
	DISTRIB_RELEASE="8.04"
else
    #sanity check
    echo "Distribution not recogized; should be one of"
    echo "Centos 5.2, Debian 5, Ubuntu 8.04, Ubuntu 8.10, Ubuntu 9.04, or Ubuntu 9.10"
fi

    

if [ $DISTRIB_ID = "Ubuntu" ]; then
    if [ $DISTRIB_RELEASE = "8.04" ]; then
	DISTRIB_FULL="ubuntu-8_04"
    elif [ $DISTRIB_RELEASE = "8.10" ]; then
	DISTRIB_FULL="ubuntu-8_10"
    elif [ $DISTRIB_RELEASE = "9.04" ]; then
	DISTRIB_FULL="ubuntu-9_x"
    elif [ $DISTRIB_RELEASE = "9.10" ]; then
	DISTRIB_FULL="ubuntu-9_x"
    else
	echo "Ubuntu distribution not recognized; should be one of"
	echo "Ubuntu 8.04, Ubuntu 8.10, Ubuntu 9.04, or Ubuntu 9.10"
	echo "Attempting use of Ubuntu 9.xx config"
	DISTRIB_FULL="ubuntu-9_x"
    fi
elif [ $DISTRIB_ID = "CentOS" ]; then
    if [ $DISTRIB_RELEASE = "5.2" ]; then
	DISTRIB_FULL="centos-5_2"
    else
	echo "Centos distribution not recogized; should be"
	echo "Centos 5.2"
	echo "But we'll push on as if it was 5.2 and hope for the best."
	DISTRIB_FULL="centos-5_2"
    fi
else
    #sanity check
    echo "Distribution not recogized; should be one of"
    echo "Centos 5.2, Debian 5.0.3, Ubuntu 8.04, Ubuntu 8.10, Ubuntu 9.04, or Ubuntu 9.10"
fi


echo "system recognized as $DISTRIB_ID $DISTRIB_RELEASE"



#
# install wget, so the we can retrive the appropriate build scripts
#

if [ "$(id -u)" = "0" ] ; then # we're already root
   SUDO=""  
else
   SUDO="sudo"
 fi


if [ $DISTRIB_ID = "Ubuntu" ]; then
    $SUDO apt-get update
    $SUDO apt-get -y install wget
elif  [ $DISTRIB_ID = "CentOS" ]; then
    $SUDO yum -y install wget
fi



#
# download main amazon ec2 build script and make executable
#

wget http://sashimi.svn.sourceforge.net/viewvc/sashimi/trunk/trans_proteomic_pipeline/installer_linux/amazon_ec2_installer.sh

chmod u+x amazon_ec2_installer.sh



#
# download TPP build prerequisite script for building the TPP on this system
# and make executable
#   

wget http://sashimi.svn.sourceforge.net/viewvc/sashimi/trunk/trans_proteomic_pipeline/installer_linux/install-prerequisites-$DISTRIB_FULL.sh

chmod u+x install-prerequisites-$DISTRIB_FULL.sh



#
# run the TPP AWS EC2 install script
#

./amazon_ec2_installer.sh $1 $2 $3
