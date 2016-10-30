#
# Program: TPP Web Launcher for Amazon (TWA)
# Author:  Joe Slagel
#
# Copyright (C) 2011 by Joseph Slagel
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
# $Id: Makefile 6241 2013-07-17 17:57:44Z slagelwa $
#

# =============================================================================
# This Makefile builds and installs the TPP web launcher for Amazon (TWA)
# provided for using TPP on Amazon Web Services.
#
# Type "make help" for more information on the targets and usage.
# =============================================================================

#
# -- Commonly changed settings ------------------------------------------------
#

# Path to ExtJS
EXTJS_DIR = ext-4.0.0

# Destination directory.  Should be a location under your Web Server where
# you are serving web pages out of.
DEST = /var/www/proteomics/twa

# -----------------------------------------------------------------------------

SHELL   = /bin/sh
INSTALL = install -m 644 


# --  Rule which outputs a short summary on the targets available to the user.
# Should always be the first rule in the Makefile to ensure that if no targets
# are provided on the command line it is invoked.
.PHONY: help
help :
	@echo
	@echo "Usage: make target ... [ DEFINES ...]"
	@echo
	@echo "Makefile for mass spectrography protein identification"
	@echo "projects using the tools available in TPP."
	@echo
	@echo "Targets:"
	@echo "   help          Displays this help"
	@echo "   install       Install package"
	@echo
	@echo "You may additionally define these environment variables:"
	@echo "   DEST_DIR      Desination directory"
	@echo "   EXTJS_DIR     ExtJs directory"
	@echo


# Install main files
install : install-prg install-aux install-extjs

install-prg : 
	mkdir -p ${DEST_DIR} 
	${INSTALL} MyViewport.ui.js ${DEST_DIR}
	${INSTALL} MyViewport.js ${DEST_DIR}
	${INSTALL} twa.js ${DEST_DIR}
	${INSTALL} AWS.js ${DEST_DIR}

# Install auxillary files
install-aux : 
	mkdir -p ${DEST_DIR} 
	${INSTALL} welcome.html ${DEST_DIR}
	${INSTALL} index.html ${DEST_DIR}
	${INSTALL} hmacsha1.js ${DEST_DIR}
	find images -type f -exec ${INSTALL} -D {} ${DEST_DIR}/{} \;

# Install what we need out of extjs
install-extjs : 
	mkdir -p ${DEST_DIR}/ext
	${INSTALL} ${EXTJS_DIR}/ext-all.js ${DEST_DIR}/ext
#	${INSTALL} -D ${EXTJS_DIR}/adapter/ext/ext-base.js ${DEST_DIR}/ext/adapter/ext/ext-base.js
	${INSTALL} ${EXTJS_DIR}/ext-all-debug.js ${DEST_DIR}/ext
	find ext/resources -type f -exec ${INSTALL} -D {} ${DEST_DIR}/{} \;

lint :
	jsl -process twa.js
	jsl -process AWS.js
	jsl -process Options.ui.js
	jsl -process MyViewport.js
	jsl -process MyViewport.ui.js
