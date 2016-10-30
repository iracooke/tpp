
################################################################################
#
#     This makefile is only useful for installing the lorikeet spectrum viewer
#     auxiliary files.  Application is built and installed by main TPP makefile
#     Date: December 30 2010
#
#     Copyright (C) 2010  Luis, ISB Seattle
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License as published by the Free Software Foundation; either
#     version 2.1 of the License, or (at your option) any later version.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#
#     Institute for Systems Biology
#     401 Terry Avenue North
#     Seattle, WA  98109  USA
#
###


# Include configuration variables for the build
SRC_ROOT= ../../../

include $(SRC_ROOT)/Makefile.incl


install-linux:
	mkdir -p ${HTML_DIR}css
	chmod a+x ${HTML_DIR}css
	mkdir -p ${HTML_DIR}js
	chmod a+x ${HTML_DIR}js
	mkdir -p ${HTML_DIR}images/lorikeet
	chmod a+x ${HTML_DIR}images/lorikeet
	cp -f css/*.css ${HTML_DIR}css
	chmod a+r ${HTML_DIR}css/*.css
	cp -f js/*.js ${HTML_DIR}js
	chmod a+r ${HTML_DIR}js/*.js
	cp -f images/*.png ${HTML_DIR}images/lorikeet
	chmod a+r ${HTML_DIR}images/lorikeet/*.png
