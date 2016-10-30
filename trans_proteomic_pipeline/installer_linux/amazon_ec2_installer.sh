#!/bin/bash 
#
# script for getting TPP built and serving web pages on Amazon EC2.
# see amazon_ec2_notes for further details.
#
# args are:
#    your_AWS_ACCESS_KEY_ID your_AWS_SECRET_ACCESS_KEY your_s3_bucket_name
#
# I used AMI  	ami-7806e211, which is an Ubuntu 8.10 linux image, EC2 "small" size.


# a little helper function
append_apache_config() {
	echo $1 >> /etc/apache2/sites-enabled/000-default
}

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
    echo "Centos 5.2, Ubuntu 8.04, Ubuntu 8.10, Ubuntu 9.04, or Ubuntu 9.10"
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
	echo "attempting to use Ubuntu 9.x config"
	DISTRIB_FULL="ubuntu-9_x"
    fi
elif [ $DISTRIB_ID = "CentOS" ]; then
    if [ $DISTRIB_RELEASE = "5.2" ]; then
	DISTRIB_FULL="centos-5_2"
    else
	echo "Centos distribution not recogized; should be"
	echo "Centos 5.2"
	echo "attempting to use Centos 5.2 config"
	DISTRIB_FULL="centos-5_2"
    fi
else
    #sanity check
    echo "Distribution not recogized; should be one of"
    echo "Centos 5.2, Ubuntu 8.04, Ubuntu 8.10, Ubuntu 9.04, or Ubuntu 9.10"
fi


echo "system recognized as $DISTRIB_ID $DISTRIB_RELEASE"




#
# install prerequisites for building the TPP on this system
#   
echo "installing build prerequisites for $DISTRIB_FULL..."

echo ./install-prerequisites-$DISTRIB_FULL.sh

./install-prerequisites-$DISTRIB_FULL.sh

echo "done installing build prerequisites for $DISTRIB_FULL"




#
# check out the TPP from sashimi trunk
#
mkdir -p sashimi
cd sashimi
mkdir -p trans_proteomic_pipeline
svn checkout https://sashimi.svn.sourceforge.net/svnroot/sashimi/trunk/trans_proteomic_pipeline trans_proteomic_pipeline




#
# these commands build s3fs, which we'll use to hook your S3 bucket up to EC2 as a filesystem:
#
cd ~
STABLE_S3FS=s3fs-r203.tar.gz
wget http://s3fs.googlecode.com/files/$(STABLE_S3FS)
tar -xzf $(STABLE_S3FS)
cd s3fs
make
make install




#
# now to hook up your S3 bucket (using a password file so keys don't appear in "ps -AF"),
# (Note: to be safe, don't forget to "umount /var/www/s3" before you shutdown your EC2 instance to flush everything)
# 
mkdir -p /var/www/s3
mkdir -p /mnt/tmp
echo "$1:$2" > /etc/passwd-s3fs
chmod 600 /etc/passwd-s3fs  
/usr/bin/s3fs $3 /var/www/s3 -oallow_other -ouse_cache=/mnt/tmp
rm /etc/passwd-s3fs




#
# Now to build TPP - we tweak the config slightly, so that user password info resides on S3 where it won't be 
# lost in an EC2 restart:
#
cd ~/sashimi/trans_proteomic_pipeline/src
echo "CGI_USERS_DIR=/var/www/s3/users/" > Makefile.config.incl
make all install




#
# Now configure Apache in /etc/apache2/sites-enabled/000-default :
apt-get -y install apache2
cp /etc/apache2/sites-enabled/000-default /etc/apache2/000-default.bak
# remove closure
sed "s/<\/VirtualHost>//g" /etc/apache2/000-default.bak > /etc/apache2/sites-enabled/000-default
# and add
append_apache_config "###############################################"
append_apache_config "#"
append_apache_config "# ISB-Tools Trans Proteomic Pipeline directives"
append_apache_config "#"
append_apache_config ""
append_apache_config "###"
append_apache_config "# enable SSI"
append_apache_config "AddType text/html .shtml"
append_apache_config "AddHandler server-parsed .shtml"
append_apache_config ""
append_apache_config ""
append_apache_config "###"
append_apache_config "# environmental variable passed to scripts"
append_apache_config "SetEnv WEBSERVER_ROOT /var/www/s3"
append_apache_config "# you may want to define WEBSERVER_TMP to keep tempfiles out of your data "
append_apache_config "# directories, or in RAMdisk, or somesuch"
append_apache_config "SetEnv WEBSERVER_TMP /var/www/tmp"
append_apache_config ""
append_apache_config ""
append_apache_config "###"
append_apache_config "# directory for tpp's html resources (css, js, images, etc)"
append_apache_config "Alias /tpp/html "/usr/local/tpp/html""
append_apache_config "<Directory "/usr/local/tpp/html">"
append_apache_config "    AllowOverride None"
append_apache_config "    Options Includes Indexes FollowSymLinks MultiViews"
append_apache_config "    Order allow,deny"
append_apache_config "    Allow from all"
append_apache_config "</Directory>"
append_apache_config ""
append_apache_config "###"
append_apache_config "# directory for tpp's schema resources"
append_apache_config "<Directory "/usr/local/tpp/schema">"
append_apache_config "    AllowOverride None"
append_apache_config "    Options Includes Indexes FollowSymLinks MultiViews"
append_apache_config "    Order allow,deny"
append_apache_config "    Allow from all"
append_apache_config "</Directory>"
append_apache_config ""
append_apache_config ""
append_apache_config "###"
append_apache_config "# directory for tpp's executable files"
append_apache_config "Alias /tpp/cgi-bin/ "/usr/local/tpp/cgi-bin/""
append_apache_config "<Directory "/usr/local/tpp/cgi-bin">"
append_apache_config "    # reset the handler (thx Marius on spctools-discuss)"
append_apache_config "    AddHandler default-handler .jpg .png .css .ico .gif "
append_apache_config "    AddHandler cgi-script cgi pl"
append_apache_config "    <Files ~ "\.pl$">"
append_apache_config "        Options +ExecCGI"
append_apache_config "    </Files>"
append_apache_config "    <Files ~ "\.cgi$">"
append_apache_config "        Options +ExecCGI"
append_apache_config "    </Files>"
append_apache_config "    AllowOverride AuthConfig Limit"
append_apache_config "    Options ExecCGI"
append_apache_config "    AddHandler cgi-script .cgi .pl"
append_apache_config "    Order allow,deny"
append_apache_config "    Allow from all"
append_apache_config "</Directory>"
append_apache_config ""
append_apache_config "###"
append_apache_config "# handle shtml references"
append_apache_config "Alias /ISB "/var/www/s3/ISB""
append_apache_config "<Directory "/var/www/s3/ISB">"
append_apache_config "    Options Indexes MultiViews Includes"
append_apache_config "    AllowOverride None"
append_apache_config "    Order allow,deny"
append_apache_config "    Allow from all"
append_apache_config "    AddType text/html .shtml"
append_apache_config "    AddHandler server-parsed .shtml"
append_apache_config "    # S3FS and sendfile don't get along"
append_apache_config "    EnableSendfile Off"
append_apache_config "</Directory>"
append_apache_config ""
append_apache_config "</VirtualHost>"
append_apache_config ""




# now set the PATH within Apache so it can find xinteract etc: add this line to /etc/apache2/envvars:
echo "PATH=$PATH:/usr/local/tpp/bin;" >> /etc/apache2/envvars




#Now create the tmpdir for TPP then restart apache:
mkdir -p /var/www/tmp
chmod 777 /var/www/tmp
/etc/init.d/apache2 restart




#Now enable server side includes:
a2enmod include
/etc/init.d/apache2 reload




# and run it in your browser!
# http://[your_ec2_instance_public_dns]/tpp/cgi-bin/tpp_gui.pl
