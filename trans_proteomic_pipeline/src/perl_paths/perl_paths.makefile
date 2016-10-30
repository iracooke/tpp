# makefile for tweaking perl scripts
# this is the cygwin/MinGW/*nix version, there's a VC8 one as well in perl_paths.mak
# the real meat of it is in perl_paths.inc

# get ARCH_FAMILY
SRC_ROOT= ../
include $(SRC_ROOT)Makefile.incl

SET_X= chmod 755 $(QT)$(OUTDIR)/*.pl$(QT)

# default is empty XFORM
XFORM= "s/\//\//g"

ifeq ($(ARCH_FAMILY), mingw)
XFORM= "s/\/cygdrive\/c\/Inetpub\/tpp-bin\//c:\x5c\x5cInetpub\x5c\x5ctpp-bin\x5c\x5c/g;s/\/cygdrive\/c/c\:/g;s/cygpath \-wp //g;s/\/usr\/bin\/perl/c\:\/perl\/bin\/perl/g;s/\x27\/usr\/bin\//\x24\{base_dir\}\.\x27/g;s/tpp\/cgi\-bin/tpp\-bin/g;s/tar /bsdtar /g;s/xzOf/\-xzOf/g;s/'\/bin\//'/g"
endif
ifeq ($(ARCH_FAMILY), linux)
XFORM= "s/\/tpp\/cgi-bin\//$(ESCAPED_CGI_WEB)/g;s/\/cygdrive\/c\/Inetpub\/tpp-bin\//$(ESCAPED_TPP_ROOT)cgi\-bin\//g;s/\x24\{base_dir\}users\//${ESCAPED_CGI_USERS_DIR}/g;s/\/tools\/bin\/TPP\/tpp\//$(ESCAPED_TPP_ROOT)/g;s/cygpath \-wp //g;s/\"\/cygdrive\/c\/Inetpub\/wwwroot\/\"/\\x24ENV\{\'WEBSERVER_ROOT\'\}\.'\/'/g;s/\/tpp\-bin/$(ESCAPED_TPP_WEB)cgi\-bin/g;s/'\/usr\/bin\//'/g;s/'\/bin\//'/g
ifeq  ($(strip $(PERL_BIN)),)
PERL_BIN:=$(shell which perl)
endif
ifneq  ($(strip $(PERL_BIN)),)
XFORM +=;s/\/usr\/bin\/perl/$(ESCAPED_PERL_BIN)/g
endif
XFORM +="
endif
ifeq ($(ARCH_FAMILY), darwin)
XFORM= "s/\/cygdrive\/c\/Inetpub\/tpp-bin\//$(ESCAPED_TPP_ROOT)cgi\-bin\//g;s/\x24\{base_dir\}users\//${ESCAPED_CGI_USERS_DIR}/g;s/\/tools\/bin\/TPP\/tpp\//$(ESCAPED_TPP_ROOT)/g;s/cygpath \-wp //g;s/\"\/cygdrive\/c\/Inetpub\/wwwroot\/\"/\\x24ENV\{\'WEBSERVER_ROOT\'\}\.'\/'/g;s/\/tpp\-bin/$(ESCAPED_TPP_WEB)cgi\-bin/g;s/'\/usr\/bin\//'/g;s/'\/bin\//'/g;s/linux/darwin/g
ifneq  ($(strip $(PERL_BIN)),)
XFORM +=;s/\/usr\/bin\/perl/$(ESCAPED_PERL_BIN)/g
endif
XFORM +="
else
ifneq  ($(strip $(PERL_BIN)),)
PERL= $(PERL_BIN)
endif
endif
include perl_paths.inc
