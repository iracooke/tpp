#!/bin/bash

=== Ubuntu 9.04 Installation procedure after a clean system installation, contribued by A. Quandt ===


sudo bash
apt-get install g++ subversion vim apache2 libbz2-dev swig expat libpng12-dev gnuplot libperl-dev build-essential libgd2-xpm libgd2-xpm-dev


cd /opt
wget -nd http://downloads.sourceforge.net/boost/boost_1_38_0.tar.gz?use_mirror...
tar xzf boost_1_38_0.tar.gz
ln -s boost_1_38_0 boost
cd boost
./configure --prefix=/opt/boost
make -j5
make install
echo "/opt/boost/lib" > /etc/ld.so.conf.d/boost-1_38_0.conf

ldconfig


cd /usr/local/src/
svn co https://sashimi.svn.sourceforge.net/svnroot/sashimi/tags/release_4-2-1/trans_proteomic_pipeline/
mv trans_proteomic_pipeline tpp-4.2.1
cd tpp-4.2.1/src/

mkdir /usr/local/apps
touch Makefile.config.incl
vim Makefile.config.incl


####################### Begin Makefile.config.incl - Ubuntu 9.04 ##############
TPP_ROOT=/usr/local/apps/tpp/
TPP_WEB=/tpp/
## for Boost
BOOST_INCL=-I/opt/boost/
BOOST_LIBDIR=/opt/boost/lib
BOOST_LIBSPEC=-gcc43-mt
LINK=shared
LIBEXT=so
PERL_LIB_CORE= /usr/lib/perl/5.10/CORE/
####################### End Makefile.config.incl - Ubuntu 9.04 ################

make -j5
make install


mkdir /var/log/tpp
cd /etc/apache2/sites-enabled/
mv 000-default 000-defaultORG
vim 000-default

######################### TPP Server Configuration ############################
<VirtualHost *:80>

	DocumentRoot /var/www
	#SetEnv WEBSERVER_ROOT /usr/local/apps/tpp
	SetEnv WEBSERVER_ROOT /var/www


	# Access logs
	<IfModule log_config_module>
		# Directives defining formatting of access log output
		LogFormat "%h %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-Agent}i\"" combined
		LogFormat "%h %l %u %t \"%r\" %>s %b" common

    		<IfModule logio_module>
      			# You need to enable mod_logio.c to use %I and %O
      			LogFormat "%h %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-Agent}i\" %I %O" combinedio
    		</IfModule>
		CustomLog /var/log/tpp/tpp_access_log common
	</IfModule>

	<Directory />
		Options FollowSymLinks
		AllowOverride None
	</Directory>
	#<Directory /usr/local/apps/tpp/>
	<Directory /var/www/>
		Options Indexes FollowSymLinks MultiViews
		AllowOverride All
		Order allow,deny
		allow from all
	</Directory>

	ScriptAlias /cgi-bin/ /usr/lib/cgi-bin/
	<Directory "/usr/lib/cgi-bin">
		AllowOverride None
		Options +ExecCGI -MultiViews +SymLinksIfOwnerMatch
		Order allow,deny
		Allow from all
	</Directory>

	ErrorLog /var/log/apache2/error.log

	# Possible values include: debug, info, notice, warn, error, crit,
	# alert, emerg.
	LogLevel warn

	CustomLog /var/log/apache2/access.log combined


	# directory to store data for web browser viewing
	ScriptAlias /tpp/data "/usr/local/apps/tpp/data"
	<Directory "/usr/local/apps/tpp/data">
		AllowOverride None
		Options Indexes FollowSymLinks Includes
		Order allow,deny
		Allow from all
	</Directory>

	# directory for tpp's html resources (css, js, images, etc)
	ScriptAlias /tpp/html "/usr/local/apps/tpp/html"
	<Directory "/usr/local/apps/tpp/html">
		AllowOverride None
		Options Includes Indexes FollowSymLinks MultiViews
		Order allow,deny
		Allow from all
	</Directory>

	# directory for tpp's schema resources
	<Directory "/usr/local/apps/tpp/schema">
		AllowOverride None
		Options Includes Indexes FollowSymLinks MultiViews
   		Order allow,deny
    		Allow from all
	</Directory>

	# directory for tpp's executable files
	ScriptAlias /tpp/cgi-bin "/usr/local/apps/tpp/cgi-bin"
        <Directory "/usr/local/apps/tpp/cgi-bin">
    		#AllowOverride AuthConfig Limit
    		AllowOverride All
    		Options Indexes FollowSymLinks MultiViews ExecCGI +Includes
                AddHandler default-handler .jpg .png .css .ico .gif
		AddHandler cgi-script .cgi .pl
    		Order allow,deny
    		Allow from all
    		SetEnv WEBSERVER_ROOT /usr/local/apps
                #SetEnv WEBSERVER_ROOT /var/www
	</Directory>
</VirtualHost>
##############################################################################

/etc/init.d/apache2 restart

firefox http://localhost/cgi-bin/tpp_gui.pl


