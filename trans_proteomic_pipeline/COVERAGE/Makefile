###################################################################################
# Program       : Makefile                                                        #
# Author        : Andrew Keller <akeller@systemsbiology.org>                      #
# Date          : 11.27.02                                                        #
#                                                                                 #
# for compilation of C++ version of PeptideProphet                                #
#                                                                                 #
# This software will be available as open source in the future.  However, at this #
# time we are sharing it with you as a collaborator and ask that you do not       #
# redistribute it.                                                                #
#                                                                                 #
# for compilation of C++ version of PeptideProphet                                #
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
# By downloading and using this software, you agree that you have read,           #
# understand, and accept the above terms.                                         #
# If the terms are not acceptable, do not download and use the                    #
# software!                                                                       #
###################################################################################

# get ARCH
SRC_ROOT= ../src/
include $(SRC_ROOT)Makefile.incl

# put here the name of c++ compiler
CC=g++  -Wno-deprecated


# put here the complete directory path for the include subdirectory
CCINC= -I/usr/include

# here again the name of the c++ compiler
LD= g++


# here the complete directory path for the lib subdirectory followed by  -lm
LDFLAGS= $(SYSLIBS)
OLD = 
#-oldcxx

######################################################################
#
# do not edit past this point
#
######################################################################

#
# rules
#

.SUFFIXES:	.o .cxx

${ARCH}/%.o : %.cxx		
		$(CC) $(OLD) $(DEBUG) $(CCOPT) $(CCINC) -O3 -c $< -o $@

#
# targets
#

EXE = $(OBJ_ARCH)/batchcoverage
OBJECTS = $(ARCH)/main.o $(ARCH)/Coverage.o

all:		${EXE}

$(ARCH) :
	mkdir -p $(ARCH)

${EXE}:	$(ARCH) $(OBJECTS)
		$(LD) $(DEBUG) -O3 -o $(EXE) $(OBJECTS) $(LDFLAGS)

clean:
	rm -f $(ARCH)/*.o $(EXE) core

opt:
	CCOPT='-O'  make ${EXE}

debug:
	DEBUG='-g'  make ${EXE}

#
# dependencies
#

$(ARCH)/main.o : Coverage.h main.cxx

$(ARCH)/Coverage.o : sysdepend.h Coverage.cxx

install: ${EXE}
	cp -f ${EXE} ${BIN_DIR}

