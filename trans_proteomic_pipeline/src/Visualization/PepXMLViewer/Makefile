
################################################################################
#
#     Program: PepXMLViewer.cgi
#     Description: CGI program to display pepxml files in a web browser
#     Date: July 31 2006
#
#     Copyright (C) 2006  Natalie Tasman (original author), ISB Seattle
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
#     Natalie Tasman
#     Institute for Systems Biology
#     401 Terry Avenue North
#     Seattle, WA  98109  USA
#
#     email (remove underscores):
#     n_tasman at systems_biology dot org
#
#
###


# Include configuration variables for the build
SRC_ROOT= ../../
PEPXMLVIEWER_SRC=${SRC_ROOT}Visualization/PepXMLViewer
PEPXMLVIEWER_BUILDDIR=${OBJ_ARCH}

include $(SRC_ROOT)/Makefile.incl

APP = ${PEPXMLVIEWER_BUILDDIR}/PepXMLViewer.cgi

CXXFLAGS  = -DNDEBUG -DGCC #-pedantic -Werror -Wall -Wno-long-long
PROFILE  = #-pg

LDLIBS =  $(TPP_BUILD) $(LIB_PWIZ) $(TPP_MSDATA_READER_LIB) $(BOOST_LIBS) $(GZSTREAMLIB)  $(BOOST_REGEX_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_SERIALIZATION_LIB)  $(EXPAT_LIB) $(SYSLIBS) $(HDF5_LIB) $(ZLIB_LIB)

LDFLAGS = $(PROFILE)

IFLAGS = -I$(EXPAT_INCL) $(BOOST_INCL) -I$(SRC_ROOT) $(PWIZ_INCL) -I$(HDF5_INCL)


# defined in Makefile.incl
# currently, optimization causes a segfault.
OFLAGS = $(OPT)

MV  := mv -f
RM  := rm -f
SED := sed

SOURCES = $(wildcard ${PEPXMLVIEWER_SRC}/*.cxx) 
OBJECTS = $(patsubst ${PEPXMLVIEWER_SRC}/%.cxx,${PEPXMLVIEWER_BUILDDIR}/%.o,$(SOURCES)) 


CC = g++ $(CXXFLAGS) $(OFLAGS) $(OSFLAGS)

.SUFFIXES: .cxx

#
# note we have our own arch subdirs, needed
# to avoid filename conflicts in the OBJ_ARCH
# directory of the parent project
#

${PEPXMLVIEWER_BUILDDIR}/%.o : %.cxx	
	$(CC)   -c -Wno-deprecated $(DEBUG) $(OFLAGS) $(PROFILE) $(IFLAGS) $< -o $@


all: $(OBJ_ARCH) $(EXPAT_LIB) $(APP)


$(APP) : $(OBJECTS) 
	@echo $(OBJECTS)
	$(CXX) $(DEBUG) $(OFLAGS) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@


${PEPXMLVIEWER_BUILDDIR}/Field.o : PepXField.cxx PepXField.h XMLNode.h $(TPP_MSDATA_READER_H) $(EXPAT_INCL)/expat.h \
	$(EXPAT_INCL)/expat_external.h PMap.h \
	$(SRC_ROOT)/Visualization/Comet/comet-fastadb/calc_pI.cxx

${PEPXMLVIEWER_BUILDDIR}/Filter.o : PepXFilter.cxx PepXFilter.h PepXField.h XMLNode.h \
	$(EXPAT_INCL)/expat.h \
	$(EXPAT_INCL)/expat_external.h PMap.h

${PEPXMLVIEWER_BUILDDIR}/HeaderMRSSQ.o : HeaderMRSSQ.cxx HeaderMRSSQ.h \
	PepXSAXHandler.h $(EXPAT_INCL)/expat.h \
	$(EXPAT_INCL)/expat_external.h XMLNode.h \
	XMLTree.h PepXOptions.h PepXField.h PMap.h PepXFilter.h \
	PipelineAnalysis.h PepXNode.h PepXUtility.h PepXDebug.h SQHandler.h

${PEPXMLVIEWER_BUILDDIR}/HeaderOnly.o : HeaderOnly.cxx HeaderOnly.h PepXSAXHandler.h \
	$(EXPAT_INCL)/expat.h \
	$(EXPAT_INCL)/expat_external.h XMLTree.h XMLNode.h \
	PipelineAnalysis.h PepXNode.h PepXUtility.h PepXDebug.h PepXField.h PMap.h \
	PepXOptions.h PepXFilter.h SQHandler.h

${PEPXMLVIEWER_BUILDDIR}/HeaderOnly.o : HeaderOnly.cxx HeaderOnly.h PepXSAXHandler.h \
	$(EXPAT_INCL)/expat.h \
	$(EXPAT_INCL)/expat_external.h XMLTree.h XMLNode.h \
	PipelineAnalysis.h PepXNode.h PepXUtility.h PepXDebug.h PepXField.h PMap.h \
	PepXOptions.h PepXFilter.h SQHandler.h

${PEPXMLVIEWER_BUILDDIR}/IndexFile.o : PepXIndexFile.cxx PepXIndexFile.h \
	PepXUtility.h PepXDebug.h PipelineAnalysis.h XMLNode.h \
	$(EXPAT_INCL)/expat.h \
	$(EXPAT_INCL)/expat_external.h PepXField.h PMap.h PepXOptions.h \
	PepXFilter.h SQHandler.h

${PEPXMLVIEWER_BUILDDIR}/main.o main.d : PepXMain.cxx PepXViewer.h PepXOptions.h PepXField.h XMLNode.h \
	$(EXPAT_INCL)/expat.h \
	$(EXPAT_INCL)/expat_external.h PMap.h PepXFilter.h PepXParser.h \
	SQHandler.h PipelineAnalysis.h PepXNode.h PepXUtility.h PepXDebug.h \
	PepXSAXHandler.h HeaderMRSSQ.h XMLTree.h MRSOffsetOnly.h SQOffsetOnly.h \
	HeaderOnly.h

${PEPXMLVIEWER_BUILDDIR}/MRSOffsetOnly.o : MRSOffsetOnly.cxx MRSOffsetOnly.h PepXSAXHandler.h \
	$(EXPAT_INCL)/expat.h \
	$(EXPAT_INCL)/expat_external.h XMLTree.h XMLNode.h \
	PipelineAnalysis.h PepXNode.h PepXUtility.h PepXDebug.h PepXField.h PMap.h \
	PepXOptions.h PepXFilter.h SQHandler.h

${PEPXMLVIEWER_BUILDDIR}/Options.o : PepXOptions.cxx PepXUtility.h PepXDebug.h PepXOptions.h PepXField.h XMLNode.h \
	$(EXPAT_INCL)/expat.h \
	$(EXPAT_INCL)/expat_external.h PMap.h PepXFilter.h

${PEPXMLVIEWER_BUILDDIR}/PepXNodeVecBuilder.o : PepXNodeVecBuilder.cxx PepXNodeVecBuilder.h \
	PepXNode.h PepXUtility.h PepXDebug.h PipelineAnalysis.h XMLNode.h \
	$(EXPAT_INCL)/expat.h \
	$(EXPAT_INCL)/expat_external.h PepXField.h PMap.h PepXOptions.h \
	PepXFilter.h SQHandler.h

${PEPXMLVIEWER_BUILDDIR}/PepXNodeVecSorter.o : PepXNodeVecSorter.cxx XMLNode.h \
	$(EXPAT_INCL)/expat.h \
	$(EXPAT_INCL)/expat_external.h XMLTree.h PepXUtility.h PepXDebug.h \
	PepXNodeVecSorter.h PepXField.h PMap.h PepXNode.h

${PEPXMLVIEWER_BUILDDIR}/PepXParser.o : PepXParser.cxx PepXParser.h SQHandler.h XMLNode.h \
	$(EXPAT_INCL)/expat.h \
	$(EXPAT_INCL)/expat_external.h PipelineAnalysis.h PepXNode.h \
	PepXUtility.h PepXDebug.h PepXField.h PMap.h PepXOptions.h PepXFilter.h PepXSAXHandler.h \
	HeaderMRSSQ.h XMLTree.h MRSOffsetOnly.h SQOffsetOnly.h HeaderOnly.h \
	PepXNodeVecBuilder.h

${PEPXMLVIEWER_BUILDDIR}/PepXViewer.o : PepXViewer.cxx PepXViewer.h PepXOptions.h PepXField.h XMLNode.h \
	$(EXPAT_INCL)/expat.h \
	$(EXPAT_INCL)/expat_external.h PMap.h PepXFilter.h PepXParser.h \
	SQHandler.h PipelineAnalysis.h PepXNode.h PepXUtility.h PepXDebug.h \
	PepXSAXHandler.h HeaderMRSSQ.h XMLTree.h MRSOffsetOnly.h SQOffsetOnly.h \
	HeaderOnly.h SQspreadsheetDisplayer.h SQhtmlDisplayer.h \
	PepXNodeVecSorter.h $(SRC_ROOT)common/constants.h

${PEPXMLVIEWER_BUILDDIR}/PipelineAnalysis.o : PipelineAnalysis.cxx PipelineAnalysis.h XMLNode.h \
	$(EXPAT_INCL)/expat.h \
	$(EXPAT_INCL)/expat_external.h PepXNode.h PepXUtility.h PepXDebug.h \
	PepXField.h PMap.h PepXOptions.h PepXFilter.h $(SRC_ROOT)common/constants.h \
	$(OBJ_ARCH)/TPPVersionInfo.o

${PEPXMLVIEWER_BUILDDIR}/PepXSAXHandler.o : PepXSAXHandler.cxx PepXSAXHandler.h \
	$(EXPAT_INCL)/expat.h \
	$(EXPAT_INCL)/expat_external.h PepXUtility.h PepXDebug.h

${PEPXMLVIEWER_BUILDDIR}/SQOffsetOnly.o : SQOffsetOnly.cxx SQOffsetOnly.h PepXSAXHandler.h \
	$(EXPAT_INCL)/expat.h \
	$(EXPAT_INCL)/expat_external.h XMLTree.h XMLNode.h \
	PipelineAnalysis.h PepXNode.h PepXUtility.h PepXDebug.h PepXField.h PMap.h \
	PepXOptions.h PepXFilter.h SQHandler.h

${PEPXMLVIEWER_BUILDDIR}/utility.o : PepXUtility.cxx PepXUtility.h PepXDebug.h

${PEPXMLVIEWER_BUILDDIR}/XMLNode.o : XMLNode.cxx XMLNode.h $(EXPAT_INCL)/expat.h \
	$(EXPAT_INCL)/expat_external.h

${PEPXMLVIEWER_BUILDDIR}/XMLTree.o : XMLTree.cxx XMLTree.h $(EXPAT_INCL)/expat.h \
	$(EXPAT_INCL)/expat_external.h XMLNode.h


.PHONY: doc
doc:
	# makefile will fail if doxygen isn't visible from the shell
	@if which doxygen &>/dev/null; then echo "found doxygen; creating documentation"; else echo "doxygen not found: required for documentation."; false; fi
	doxygen doxygen.config


.PHONY: clean
clean:
	@echo $(OBJECTS)
	rm -rf $(OBJECTS) $(APP) *.gch gmon.out *.o   *~



install-linux:
	mkdir -p ${HTML_DIR}css
	chmod a+x ${HTML_DIR}css
	mkdir -p ${HTML_DIR}js
	chmod a+x ${HTML_DIR}js
	cp -f ${OBJ_ARCH}/PepXMLViewer.cgi ${CGI_DIR}
	cp -f html/css/*.css ${HTML_DIR}css
	chmod a+r ${HTML_DIR}css/*.css
	cp -f html/js/*.js ${HTML_DIR}js
	chmod a+r ${HTML_DIR}js/*.js
	cp -f html/*.html ${HTML_DIR}
	chmod a+r ${HTML_DIR}*.html

install-windows:
	cp -f ${OBJ_ARCH}/PepXMLViewer.cgi `cygpath -u $$WEBSERVER_ROOT`/../tpp-bin
	if ! test -d `cygpath -u $$WEBSERVER_ROOT`/../tpp-bin/css; then \
	mkdir `cygpath -u $$WEBSERVER_ROOT`/../tpp-bin/css;  fi
	if ! test -d `cygpath -u $$WEBSERVER_ROOT`/../tpp-bin/js; then \
	mkdir `cygpath -u $$WEBSERVER_ROOT`/../tpp-bin/js;  fi
	cp -f html/css/*.css `cygpath -u $$WEBSERVER_ROOT`/../tpp-bin/css
	cp -f html/js/*.js `cygpath -u $$WEBSERVER_ROOT`/../tpp-bin/js
	chmod -R a+xr `cygpath -u $$WEBSERVER_ROOT`/../tpp-bin/css
	chmod -R a+xr `cygpath -u $$WEBSERVER_ROOT`/../tpp-bin/js
	cp -f html/*.html `cygpath -u $$WEBSERVER_ROOT`/../tpp-bin
	chmod a+xr `cygpath -u $$WEBSERVER_ROOT`/../tpp-bin/*.html


