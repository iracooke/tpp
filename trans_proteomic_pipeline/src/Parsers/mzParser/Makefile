###################################################################################
# Program       : Makefile for MzParser                                           #
# Author        : Mike Hoopmann, Built from SpectraST Makefile by Henry Lam       #
# Date          : 01/28/2011                                                      #
#                                                                                 #
# THE SOFTWARE IS PROVIDED BY THE INSTITUTE FOR SYSTEMS BIOLOGY (ISB)             #
# "AS IS" AND "WITH ALL FAULTS." ISB MAKES NO REPRESENTATIONS OR WARRANTI         #
# ES OF ANY KIND CONCERNING THE QUALITY, SAFETY OR SUITABILITY OF THE             #
# SOFTWARE, EITHER EXPRESSED OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IM-     #
# PLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,          #
# OR NON-INFRINGEMENT.                                                            #
#                                                                                 #
# ISB MAKES NO REPRESENTATIONS OR WARRANTIES AS TO THE TRUTH, ACCURACY            #
# OR COMPLETENESS OF ANY STATEMENTS, INFORMATION OR MATERIALS CONCERNING          #
#                                                                                 #
# THE SOFTWARE THAT IS CONTAINED IN ANY DOCUMENTATION INCLUDED WITH THE           #
# SOFTWARE OR ON AND WITHIN ANY OF THE WEBSITES OWNED AND OPERATED BY ISB         #
#                                                                                 #
# IN NO EVENT WILL ISB BE LIABLE FOR ANY INDIRECT, PUNITIVE, SPECIAL,             #
# INCIDENTAL OR CONSEQUENTIAL DAMAGES HOWEVER THEY MAY ARISE AND EVEN IF          #
# ISB HAVE BEEN PREVIOUSLY ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.            #
#                                                                                 #
###################################################################################

# Include configuration variables for the build
SRC_ROOT= ../../
include $(SRC_ROOT)/Makefile.incl

LDFLAGS= $(SYSLIBS)

MZPARSER=${OBJ_ARCH}/libmzparser.a

# other flags
MZPFLAGS= -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -DGCC
WARNINGFLAGS = -Werror -Wformat -Wstrict-aliasing -Wno-deprecated -Wno-char-subscripts
IFLAGS=  -I$(SRC_ROOT) -I$(EXPAT_INCL) -I$(HDF5_INCL)

CXX=g++
  


######################################################################
#
# do not edit past this point
#
######################################################################

OBJS= ${ARCH}/mzp_base64.o \
	${ARCH}/BasicChromatogram.o \
	${ARCH}/BasicSpectrum.o \
	${ARCH}/saxhandler.o \
	${ARCH}/saxmzmlhandler.o \
	${ARCH}/saxmzxmlhandler.o \
	${ARCH}/mz5handler.o \
	${ARCH}/mzpMz5Config.o \
	${ARCH}/mzpMz5Structs.o \
	${ARCH}/PWIZface.o \
	${ARCH}/RAMPface.o \
	${ARCH}/Czran.o \
	${ARCH}/mzParser.o

#
# rules
#
.SUFFIXES:	.o .cpp 

#
# note we have our own arch subdirs, needed
# to avoid filename conflicts in the OBJ_ARCH
# directory of the parent project
#

${ARCH}/%.o : %.cpp
	@ mkdir -p $(ARCH)
	$(CXX) $(OLD) $(DEBUG) $(IFLAGS) $(MZPFLAGS) $(WARNINGFLAGS) -O3 -c $< -o $@


#
# targets
#

all : ${OBJS}
#	$(CXX) $(DEBUG) -O3 $^ $(LDFLAGS) -o $@ 
	ar rcs $(MZPARSER) $(OBJS)

clean:
	rm -rf ${ARCH}; rm -f $(MZPARSER) core* *~

debug:
	DEBUG='-g'  make all

#
# dependencies
#
${ARCH}/mzp_base64.o : mzp_base64.cpp  mzParser.h
${ARCH}/BasicChromatogram.o : BasicChromatogram.cpp mzParser.h
${ARCH}/BasicSpectrum.o : BasicSpectrum.cpp  mzParser.h
${ARCH}/saxhandler.o : saxhandler.cpp  mzParser.h
${ARCH}/saxmzmlhandler.o : saxhandler.cpp  saxmzmlhandler.cpp  mzParser.h 
${ARCH}/saxmzxmlhandler.o : saxhandler.cpp  saxmzxmlhandler.cpp  mzParser.h
${ARCH}/mz5handler.o : mzpMz5Config.cpp  mzpMz5Structs.cpp  mz5handler.cpp  mzParser.h
${ARCH}/mzpMz5Config.o : mzpMz5Config.cpp  mzParser.h
${ARCH}/mzpMz5Structs.o : mzpMz5Structs.cpp  mzParser.h
${ARCH}/PWIZface.o : saxhandler.cpp  saxmzmlhandler.cpp  mz5handler.cpp PWIZface.cpp mzParser.h
${ARCH}/RAMPface.o : saxhandler.cpp  saxmzmlhandler.cpp  saxmzxmlhandler.cpp  mz5handler.cpp  RAMPface.cpp  mzParser.h
${ARCH}/Czran.o : Czran.cpp  mzParser.h
${ARCH}/mzParser.o : mzParser.cpp  mzParser.h
