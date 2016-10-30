#!/bin/bash 
#
# Script for preparing a TPP Amazon EC2 image from a stock ubutunu based EC2 
# image.  Much of this script was taken from the original work done by
# Insilicos to run the TPP on the cloud.
#

TPP_VERSION=${TPP_VERSION:-4.6.2}
TPP_DIR=/opt/tpp
TPP_DATA=/mnt/tppdata
TPP_USERS=$TPP_DIR/users
APACHE_CONF=/etc/apache2/sites-enabled/000-default.conf

set -e		# Exit on error

ROOTDIR=`pwd`

# a little helper function to append output to the apache config files
append_apache_config() {
	echo $1 >> $APACHE_CONF
}

#
# Add multiverse support in /etc/apt/sources.list  In order to install perl 
# modules and other unsupported third party software
#
echo "=== Enabling multiverse repository..."
sed -i "/^# deb.*multiverse/ s/^# //" /etc/apt/sources.list
echo "=== Done enabling multiverse"
echo

#
# Install prerequisites for building the TPP on this system
#   
echo "=== Installing build prerequisites..."

apt-get -y --force-yes update
apt-get -y --force-yes install \
    zip \
    g++ \
    subversion \
    libbz2-dev \
    swig \
    libexpat1 \
    libexpat1-dev \
    libpng12-dev \
    gnuplot \
    libperl-dev \
    build-essential \
    libgd2-xpm-dev \
	libfuse-dev \
	libcurl4-openssl-dev \
	libxml2-dev \
    libgd2-xpm-dev \
    libxml-parser-perl \
  cabextract \
  apache2 \
  ant \
  ruby \
  libio-compress-perl \

apt-get -y --force-yes install openjdk-6-jdk
apt-get -y --force-yes install xsltproc
apt-get -y --force-yes install s3cmd

export JAVA_HOME=$(readlink -f /usr/bin/java | sed "s:bin/java::")
echo "export JAVA_HOME=$JAVA_HOME" | tee -a /etc/profile.d/java.sh

echo "=== Done installing build prerequisites"
echo

#
# Now to build TPP
#
echo "=== Building and installing TPP ${TPP_VERSION}..."
export PERL_BIN=$(which perl)
cd $ROOTDIR
if [ "$TPP_VERSION" = "trunk" ]; then
   svn export svn://svn.code.sf.net/p/sashimi/code/trunk/trans_proteomic_pipeline tpp
else
   svn export svn://svn.code.sf.net/p/sashimi/code/tags/release_${TPP_VERSION//./-}/trans_proteomic_pipeline tpp
fi
cd tpp/src
echo "TPP_ROOT=$TPP_DIR/"         > Makefile.config.incl
echo "CGI_USERS_DIR=$TPP_USERS/" >> Makefile.config.incl

# Update version to indicate its a AMI
perl -pi -e 's/^(#define\s+TPP_RELEASE_NAME\s+")([^"]+)(".*)$/$1$2 (AMI)$3/' common/TPPVersion.h

# Fix 4.5.0 for ubuntu 
if [ "$TPP_VERSION" == "4.5.0" ]; then
#   perl -pi -e 's/\#include/#include <stdlib.h>\n#include/' /tmp/tpp/extern/ProteoWizard/pwiz/pwiz/analysis/common/ZeroSampleFiller.hpp
   echo "fixing $TPP_VERSION"

   perl -pi -e 's/mingw-i686/mingw/' perl_paths/perl_paths.makefile
   perl -pi -e 's/ARCH/ARCH_FAMILY/' perl_paths/perl_paths.makefile

   sed -i -e '113s/$/ -Wno-unused-result/' Makefile
   sed -i -e '14s/$/ -Wno-unused-result/' util/Makefile
   sed -i -e'33i#include <stdlib.h>\n' Parsers/mzParser/mzParser.h
   sed -i -e'34i#include <stdint.h>\n' Parsers/mzParser/mzParser.h

   echo "done"
fi

make all 
make install SRC_ROOT=$PWD/	# '/' is important
# Make a cache directory for petunia
mkdir -p $TPP_USERS
chown www-data:www-data $TPP_USERS
chmod 775 $TPP_USERS

# Make local data directories for tpp
# but do this in rc.local script so it
# happens every boot
sed -i -e "s/exit 0//" /etc/rc.local
echo "mkdir -p $TPP_DATA/local" >> /etc/rc.local
echo "chown -R www-data:www-data $TPP_DATA" >> /etc/rc.local
echo "chmod -R 755 $TPP_DATA" >> /etc/rc.local
echo "exit 0" >> /etc/rc.local
/etc/rc.local

# Add profile to set path
echo "export PATH=\$PATH:$TPP_DIR/bin" | tee -a /etc/profile.d/tpp.sh
echo "=== Done installing TPP"

echo "=== Fixing TPP..."
# fix petunia default user permissions/files
rm -rf $TPP_USERS/guest/.svn
chown -R www-data:www-data $TPP_USERS/guest
chmod 750 $TPP_USERS/guest
# change petunia top location
sed -i -e 's/${www_root}ISB\/data\//${www_root}/' $TPP_DIR/cgi-bin/tpp_gui.pl
sed -i "/dummy/i 'www_root' => '/mnt/'," $TPP_DIR/cgi-bin/tpp_gui_config.pl
sed -i "/dummy/i 'data_dir' => '/mnt/tppdata/'," $TPP_DIR/cgi-bin/tpp_gui_config.pl
# add s3cmd functionality
export S3=$(which s3cmd)
sed -i "/dummy/i 's3cmd' => '$S3'," $TPP_DIR/cgi-bin/tpp_gui_config.pl
# fix echo path
export ECHO=$(which echo)
perl -pi -e "s#/usr/bin/echo/#$ECHO/#" $TPP_DIR/cgi-bin/tpp_gui.pl
# add ISB alias
sed -i '/Alias \/tppdata/i Alias /ISB /opt/tpp' /etc/apache2/sites-available/*default*
# fix document root
perl -pi -e 's#DocumentRoot .*#DocumentRoot /mnt/tppdata#' /etc/apache2/sites-available/*default*
echo "=== Done fixing TPP"

echo "=== Configuring Apache..."
a2enmod cgi
a2enmod rewrite
#
# Now configure Apache in /etc/apache2/sites-enabled/000-default :
cp $APACHE_CONF ${APACHE_CONF}.bak
# remove closure
sh -c "sed 's/<\/VirtualHost>//g' ${APACHE_CONF}.bak > $APACHE_CONF"
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
append_apache_config "SetEnv WEBSERVER_ROOT $TPP_DATA"
append_apache_config "# you may want to define WEBSERVER_TMP to keep tempfiles out of your data "
append_apache_config "# directories, or in RAMdisk, or somesuch"
append_apache_config "SetEnv WEBSERVER_TMP $TPP_DIR/tmp"
append_apache_config ""
append_apache_config ""
append_apache_config "###"
append_apache_config "# directory for tpp's html resources (css, js, images, etc)"
append_apache_config "Alias /tpp/html "$TPP_DIR/html""
append_apache_config "<Directory "$TPP_DIR/html">"
append_apache_config "    AllowOverride None"
append_apache_config "    Options Includes Indexes FollowSymLinks MultiViews"
append_apache_config "    Require all granted"
append_apache_config "</Directory>"
append_apache_config ""
append_apache_config "###"
append_apache_config "# directory for tpp's schema resources"
append_apache_config "<Directory "$TPP_DIR/schema">"
append_apache_config "    AllowOverride None"
append_apache_config "    Options Includes Indexes FollowSymLinks MultiViews"
append_apache_config "    Require all granted"
append_apache_config "</Directory>"
append_apache_config ""
append_apache_config "###"
append_apache_config "# directory for tpp's executable files"
append_apache_config "Alias /tpp/cgi-bin/ "$TPP_DIR/cgi-bin/""
append_apache_config "<Directory "$TPP_DIR/cgi-bin">"
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
append_apache_config "    Require all granted"
append_apache_config "</Directory>"
append_apache_config ""
append_apache_config "###"
append_apache_config "# handle shtml references"
append_apache_config "Alias /tppdata "$TPP_DATA""
append_apache_config "<Directory "$TPP_DATA">"
append_apache_config "    Options Indexes MultiViews Includes"
append_apache_config "    AllowOverride None"
append_apache_config "    Require all granted"
append_apache_config "    AddType text/html .shtml"
append_apache_config "    AddHandler server-parsed .shtml"
append_apache_config "    # S3FS and sendfile don't get along"
append_apache_config "    EnableSendfile Off"
append_apache_config "</Directory>"
append_apache_config ""
append_apache_config "#"
append_apache_config "# Use mod_rewrite to rewrite http GET requests to retrieve pep.xml"
append_apache_config "# files and instead view them in pepXMLViewer.  Used when browsing the"
append_apache_config "# directory tree at /proteomics and a user clicks on any *.pep.xml file."
append_apache_config "#"
append_apache_config "# Works by examining THE_REQUEST variable (the actual http header"
append_apache_config "# line, eg. "GET /somefile HTTP/1.1") to see if its a request for a"
append_apache_config "# *.pep.xml file in /proteomics.  If so then use the path to redirect to"
append_apache_config "# a URL that launches the pepXMLViewer to look at the file."
append_apache_config "#"
append_apache_config "# [R]   Send redirect back to browser instead of rewriting URL"
append_apache_config "# [NE]  Don't escape the url (already done in THE_REQUEST)"
append_apache_config "# %1    RewriteCond regex capture"
append_apache_config "# $1    RewriteRule regex capture"
append_apache_config "#"
append_apache_config "    RewriteEngine on"
append_apache_config "    RewriteCond %{THE_REQUEST} ^GET\ (/proteomics.*\.pep.xml(\.gz)?\s)"
append_apache_config "    RewriteRule ^.*$ http://%{SERVER_NAME}/tpp/cgi-bin/PepXMLViewer.cgi?xmlFileName=%1 [R,NE]"
append_apache_config ""
append_apache_config "#"
append_apache_config "# Use mod_rewrite to rewrite http GET requests to retrieve prot.xml files"
append_apache_config "# and instead view them in protXMLViewer.  Used when browsing the directory"
append_apache_config "# tree at /proteomics and a user clicks on any *.prob.xml file.  See"
append_apache_config "# previous rule for more details."
append_apache_config "#"
append_apache_config "   RewriteCond %{THE_REQUEST} ^GET\ (/proteomics.*\.prot.xml(\.gz)?\s)"
append_apache_config "   RewriteRule ^.*$ http://%{SERVER_NAME}/tpp/cgi-bin/protxml2html.pl?xmlfile=%1 [R,NE]"
append_apache_config ""
append_apache_config "</VirtualHost>"
append_apache_config ""

# Enable server side includes
a2enmod include

# Enable rewrites
a2enmod rewrite

# Update the PATH within Apache so it can find xinteract etc
echo "PATH=$PATH:$TPP_DIR/bin:" | tee -a /etc/apache2/envvars

# Disable autostart of Apache
update-rc.d apache2 disable
apache2 stop || true
echo "done configuring apache"

# Install hpc AWS tool
echo "### Installing TPP AWS tool..."
cd $ROOTDIR/tpp/extern/hpctools/amztpp
apt-get -y --force-yes install \
   libclass-insideout-perl \
   libfile-pushd-perl \
   libfile-which-perl \
   libyaml-perl \
   libparallel-forkmanager-perl \
   libamazon-sqs-simple-perl \
   libnet-amazon-s3-perl
apt-get -y install --force-yes libfile-homedir-perl libfile-pid-perl
perl Makefile.PL INSTALLSITESCRIPT=$TPP_DIR/bin
make install
cp init/amztppd.conf /etc/init
echo "manual" > /etc/init/amztppd.override
echo "### Done with TPP AWS tool"

echo "### Installing Inspect..."
cd $ROOTDIR
wget proteomics.ucsd.edu/Software/Inspect/Inspect.20120109.zip
unzip -d inspect Inspect*zip
rm inspect/Inspect.exe
cd inspect
# Fix buffer overrun issue
sed -ibak -e'1308s/StrName, 40/StrName, 20/' ParseInput.c
# Fix issue with linking
perl -pi -e 's/$/ \$(LDFLAGS)/ if s/\$\(LDFLAGS\)//' Makefile
make
cd $ROOTDIR
mv inspect /opt/inspect
echo 'export PATH=$PATH:/opt/inspect' | tee -a /etc/profile.d/tpp.sh
echo "### Done with Inspect"

echo "### Installing OMSSA..."
cd $ROOTDIR
wget ftp://ftp.ncbi.nih.gov/pub/lewisg/omssa/CURRENT/omssa-linux.tar.gz
cd /opt
tar xvzf $ROOTDIR/omssa*
mv omssa-* omssa
echo 'export PATH=$PATH:/opt/omssa' | tee -a /etc/profile.d/tpp.sh
echo "### Done with OMSSA"

echo "### Installing Myrimatch..."
cd $ROOTDIR
wget http://teamcity.fenchurch.mc.vanderbilt.edu/guestAuth/repository/download/bt18/7413:id/myrimatch-bin-linux-x86_64-gcc41-release-2_1_138.tar.bz2
tar xvf myrimatch-*.tar.bz2
mv myrimatch /usr/local/bin

echo "### Done with Myrimatch"

#echo "### Wine/msconvert..."
#sudo add-apt-repository -y ppa:ubuntu-wine/ppa
#sudo apt-get -y update
#sudo apt-get -y install wine1.4
#echo "### Done with Wine/msconvert"

echo "### fix tmp..."
chmod a+rwxt /tmp
echo "### Done with fix tmp"

echo "### make tpp tmp..."
mkdir -p /opt/tpp/tmp
chmod a+rwxt /opt/tpp/tmp
echo "### Done with tpp tmp"

echo "### Install deadman..."
cd $ROOTDIR
cp init/deadman.conf /etc/init
chmod 644 /etc/init/deadman.conf
echo "### Done with deadman"

echo "### Install tpp-s3-get/put/ebs..."
cd $ROOTDIR
cp init/tpp-s3-get.conf /etc/init
cp init/tpp-s3-put.conf /etc/init
cp init/tpp-ebs.conf /etc/init
chmod 644 /etc/init/tpp-*
chgrp www-data /etc/init/tpp-*
# disabled by default
echo "manual" > /etc/init/tpp-s3-get.override
echo "manual" > /etc/init/tpp-s3-put.override

#cp init/tpp-s3-put /etc/init.d
#chmod a+x /etc/init.d/tpp-s3-put
#update-rc.d tpp-s3-put stop 01 0 .

echo "### Done with tpp-s3-get/put/ebs"
