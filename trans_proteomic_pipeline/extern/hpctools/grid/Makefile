#
# Program: TPP HPC Tools
# Author:  Joe Slagel
#
# Copyright (C) 2009 by Joseph Slagel
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
# $Id: $
#

# =============================================================================
# This Makefile builds and installs the grid high performace computing (HPC)
# tools provided for TPP. To use, simply type make on the command line and
# all of the tools will be installed in the default location.  To override
# the installation location either edit and change the settings below or
# override them on the command line.  Type "make help" for more information.
# =============================================================================
SHELL = /bin/sh

# -- Commonly changed settings ------------------------------------------------
#

PREFIX   = /usr/local
BINDIR   = $(PREFIX)/bin
MANDIR   = $(PREFIX)/man
PARAMDIR = $(PREFIX)/params

# -----------------------------------------------------------------------------


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
	@echo
	@echo "   PREFIX        Installation location for package (default $(PREFIX))"
	@echo "   BINDIR        The directory for installing executables"
	@echo "   MANDIR        The top-level directory for installing documentation"
	@echo "   PARAMDIR      The top-level directory for installing parameter templates"
	@echo
	@echo "Example:"
	@echo
	@echo "The following command will install this package in the directory"
	@echo "/opt, putting executables in /opt/bin and documentation in"
	@echo "/opt/man."	
	@echo 
	@echo "   % make PREFIX=/opt"
	@echo

install : install-man install-templates
	mkdir -p $(BINDIR)
	install -m 444 bin/qgrid_functions $(BINDIR)
	install -m 555 bin/qcomet $(BINDIR)
	install -m 555 bin/qconvert $(BINDIR)
	install -m 555 bin/qinspect $(BINDIR)
	install -m 555 bin/qinteract $(BINDIR)
	install -m 555 bin/qlog $(BINDIR)
	install -m 555 bin/qmascot $(BINDIR)
	install -m 555 bin/qmsgfplus $(BINDIR)
	install -m 555 bin/qmyrimatch $(BINDIR)
	install -m 555 bin/qomssa $(BINDIR)
	install -m 555 bin/qsequest $(BINDIR)
	install -m 555 bin/qsequest27 $(BINDIR)
	install -m 555 bin/qspectrast $(BINDIR)
	install -m 555 bin/qtandem $(BINDIR)

install-man :
	install -d $(MANDIR)/man1
	-chmod -f 744 $(MANDIR)/man1/{qtandem,qomssa,qinspect,qtpp,runtpp,qmyrimatch,qcomet}.1
	-chmod -f 744 $(MANDIR)/man1/qspectrast.1
	pod2man -n 'HPC TOOLS' -c 'Trans-Proteomic Pipeline (TPP)' --section=1 \
		./bin/qgrid_functions > $(MANDIR)/man1/qtandem.1
	pod2man -n 'HPC TOOLS' -c 'Trans-Proteomic Pipeline (TPP)' --section=1 \
		./bin/qgrid_functions > $(MANDIR)/man1/qomssa.1
	pod2man -n 'HPC TOOLS' -c 'Trans-Proteomic Pipeline (TPP)' --section=1 \
		./bin/qgrid_functions > $(MANDIR)/man1/qinspect.1
	pod2man -n 'HPC TOOLS' -c 'Trans-Proteomic Pipeline (TPP)' --section=1 \
		./bin/qtpp > $(MANDIR)/man1/qtpp.1
	pod2man -n 'HPC TOOLS' -c 'Trans-Proteomic Pipeline (TPP)' --section=1 \
		./bin/runtpp > $(MANDIR)/man1/runtpp.1
	pod2man -n 'HPC TOOLS' -c 'Trans-Proteomic Pipeline (TPP)' --section=1 \
		./bin/qspectrast > $(MANDIR)/man1/qspectrast.1
	pod2man -n 'HPC TOOLS' -c 'Trans-Proteomic Pipeline (TPP)' --section=1 \
		./bin/qmyrimatch > $(MANDIR)/man1/qmyrimatch.1
	pod2man -n 'HPC TOOLS' -c 'Trans-Proteomic Pipeline (TPP)' --section=1 \
		./bin/qgrid_functions > $(MANDIR)/man1/qcomet.1
	chmod 444 $(MANDIR)/man1/{qtandem,qomssa,qinspect,qtpp,runtpp,qmyrimatch,qspectrast,qcomet}.1

install-templates :
	install -d $(PARAMDIR)
	install -m 444 -C templates/* $(PARAMDIR)
