###################################################################################
# Program       : Makefile                                                        #
# Author        : Henry Lam <hlam@systemsbiology.org>                      #
# Date          : 01/18/2003                                                      #
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

#LDFLAGS= $(SYSLIBS)
#LDFLAGS= -L$(OBJ_ARCH) -lm $(ZLIB_LIB) -lexpat -ltpp
LDFLAGS= ${TPPLIB} ${SYSLIBS} ${DEBUG} ${EXPAT_LIB} $(HDF5_LIB)

EXE=${OBJ_ARCH}/spectrast ${OBJ_ARCH}/plotspectrast ${OBJ_ARCH}/plotspectrast.cgi ${OBJ_ARCH}/Lib2HTML

# lfs support

ifeq (${OS},Windows_NT)
LFSFLAGS= -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D__MINGW__ -DGNUPLOT_BINARY=\"${GNUPLOT_BINARY}\"
else
LFSFLAGS= -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -DGCC -DGNUPLOT_BINARY=\"${GNUPLOT_BINARY}\"
endif
WARNINGFLAGS = -Werror -Wformat -Wstrict-aliasing -Wno-deprecated -Wno-char-subscripts
#IFLAGS=  -I$(SRC_ROOT)/common  -I$(SRC_ROOT)/Parsers/ramp -I$(SRC_ROOT)/refresh_interact -I$(SRC_ROOT)../extern/pwiz $(BOOST_INCL)
IFLAGS=  -I$(SRC_ROOT) -I$(SRC_ROOT). -I$(TPP_MSDATA_READER_INCL) $(TPP_MSDATA_READER_SUPPORT_INCL) -I$(GSL_INCLDIR) -I$(HDF5_INCL) -I$(EXPAT_INCL) $(ZLIB_INCL)

ifeq (${LPGL},1)
CXX=g++ ${DEBUG} ${IFLAGS} ${WARNINGFLAGS} ${LFSFLAGS} ${OSFLAGS} -D__LGPL__
else
CXX=g++ ${DEBUG} ${IFLAGS} ${WARNINGFLAGS} ${LFSFLAGS} ${OSFLAGS}
endif  


######################################################################
#
# do not edit past this point
#
######################################################################

MYCRAMP =  $(TPP_MSDATA_READER_BUILD) 

#MYCRAMP = ${OBJDIR}/SpectraST_cramp.o ${OBJDIR}/SpectraST_ramp.o ${OBJDIR}/SpectraST_base64.o ${OBJDIR}/SpectraST_util.o
#MYCRAMP = ${OBJDIR}/cramp.o 

ifeq (${LGPL},1)
# no kwset, no REFRESH function
else
MYKWSET = ${OBJ_ARCH}/kwset.o ${OBJ_ARCH}/obstack.o
endif

OBJDIR=${OBJ_ARCH}/SpectraST_obj

OBJS= ${OBJDIR}/SpectraSTLib.o ${OBJDIR}/SpectraSTLibIndex.o \
	${OBJDIR}/SpectraSTLibEntry.o ${OBJDIR}/SpectraSTPeakList.o \
	${OBJDIR}/SpectraSTMzLibIndex.o ${OBJDIR}/SpectraSTCreateParams.o \
	${OBJDIR}/SpectraSTLibImporter.o ${OBJDIR}/SpectraSTMspLibImporter.o \
	${OBJDIR}/SpectraSTPepXMLLibImporter.o ${OBJDIR}/SpectraSTSpLibImporter.o \
	${OBJDIR}/SpectraSTXHunterLibImporter.o ${OBJDIR}/SpectraSTTsvLibImporter.o \
	${OBJDIR}/SpectraSTSearchTask.o \
	${OBJDIR}/SpectraSTDtaSearchTask.o ${OBJDIR}/SpectraSTMspSearchTask.o \
	${OBJDIR}/SpectraSTMgfSearchTask.o \
	${OBJDIR}/SpectraSTMzXMLSearchTask.o ${OBJDIR}/SpectraSTCandidate.o\
	${OBJDIR}/SpectraSTSearch.o ${OBJDIR}/SpectraSTReplicates.o \
	${OBJDIR}/SpectraSTSearchOutput.o ${OBJDIR}/SpectraSTTxtSearchOutput.o\
	${OBJDIR}/SpectraSTXlsSearchOutput.o ${OBJDIR}/SpectraSTPepXMLSearchOutput.o \
	${OBJDIR}/SpectraSTHtmlSearchOutput.o ${OBJDIR}/SpectraSTPeptideLibIndex.o ${OBJDIR}/SpectraSTSimScores.o \
	${OBJDIR}/SpectraSTSearchParams.o ${OBJDIR}/SpectraSTMain.o \
	${OBJDIR}/SpectraSTQuery.o ${OBJDIR}/SpectraSTFileList.o \
	${OBJDIR}/SpectraSTLog.o ${OBJDIR}/SpectraSTMs2LibImporter.o \
	${OBJDIR}/SpectraSTMzXMLLibImporter.o \
	${OBJDIR}/SpectraSTFastaFileHandler.o \
	${OBJDIR}/SpectraSTSearchTaskStats.o \
	${OBJDIR}/SpectraSTDenoiser.o \
	${OBJDIR}/FileUtils.o ${OBJDIR}/Peptide.o ${OBJDIR}/XMLWalker.o \
	${OBJDIR}/ProgressCount.o ${OBJDIR}/Predicate.o \
	${MYKWSET}


#
# rules
#
.SUFFIXES:	.o .cpp 

#
# note we have our own arch subdirs, needed
# to avoid filename conflicts in the OBJ_ARCH
# directory of the parent project
#

${OBJDIR}/%.o : %.cpp
	@ mkdir -p ${OBJDIR}
	$(CXX) $(OLD) $(DEBUG) $(IFLAGS) $(LFSFLAGS) ${WARNINGFLAGS} -O2 -c $< -o $@


#
# targets
#

SPECTRAST_HPP = *.hpp *.h


all : ${EXE}

lib : ${OBJS} ${OBJ_ARCH}/libspectrast.a

${OBJ_ARCH}/libspectrast.a :  ${OBJDIR}/SpectraSTLib.o ${OBJDIR}/SpectraSTLibIndex.o \
	${OBJDIR}/SpectraSTLibEntry.o ${OBJDIR}/SpectraSTPeakList.o \
	${OBJDIR}/SpectraSTMzLibIndex.o ${OBJDIR}/SpectraSTCreateParams.o \
	${OBJDIR}/SpectraSTLibImporter.o ${OBJDIR}/SpectraSTMspLibImporter.o \
	${OBJDIR}/SpectraSTPepXMLLibImporter.o ${OBJDIR}/SpectraSTSpLibImporter.o \
	${OBJDIR}/SpectraSTXHunterLibImporter.o ${OBJDIR}/SpectraSTTsvLibImporter.o \
	${OBJDIR}/SpectraSTSearchTask.o \
	${OBJDIR}/SpectraSTDtaSearchTask.o ${OBJDIR}/SpectraSTMspSearchTask.o \
	${OBJDIR}/SpectraSTMgfSearchTask.o \
	${OBJDIR}/SpectraSTMzXMLSearchTask.o ${OBJDIR}/SpectraSTCandidate.o\
	${OBJDIR}/SpectraSTSearch.o ${OBJDIR}/SpectraSTReplicates.o \
	${OBJDIR}/SpectraSTSearchOutput.o ${OBJDIR}/SpectraSTTxtSearchOutput.o\
	${OBJDIR}/SpectraSTXlsSearchOutput.o ${OBJDIR}/SpectraSTPepXMLSearchOutput.o \
	${OBJDIR}/SpectraSTHtmlSearchOutput.o ${OBJDIR}/SpectraSTPeptideLibIndex.o ${OBJDIR}/SpectraSTSimScores.o \
	${OBJDIR}/SpectraSTSearchParams.o \
	${OBJDIR}/SpectraSTMainLIB.o \
	${OBJDIR}/SpectraSTQuery.o ${OBJDIR}/SpectraSTFileList.o \
	${OBJDIR}/SpectraSTLog.o ${OBJDIR}/SpectraSTMs2LibImporter.o \
	${OBJDIR}/SpectraSTMzXMLLibImporter.o \
	${OBJDIR}/SpectraSTFastaFileHandler.o \
	${OBJDIR}/SpectraSTSearchTaskStats.o \
	${OBJDIR}/FileUtils.o ${OBJDIR}/Peptide.o ${OBJDIR}/XMLWalker.o \
	${OBJDIR}/ProgressCount.o ${OBJDIR}/Predicate.o\
	${MYKWSET} \
	${OBJDIR}/SpectraSTDenoiser.o $(GSL_LIB)
	ar -r $@ $^

${OBJ_ARCH}/spectrast : ${OBJS} ${MYCRAMP} 
	$(CXX) $(DEBUG) -O2 $^ $(LDFLAGS) -o $@ 

${OBJ_ARCH}/plotspectrast.cgi : ${OBJDIR}/plotspectrast_cgi.o ${OBJDIR}/SpectraSTLibEntry.o ${OBJDIR}/SpectraSTQuery.o ${OBJDIR}/SpectraSTPeakList.o ${OBJDIR}/SpectraSTDenoiser.o ${OBJDIR}/SpectraSTLog.o ${OBJDIR}/FileUtils.o ${OBJDIR}/Peptide.o ${MYCRAMP}
	$(CXX) -O2 $^  $(LDFLAGS) -o $@ 

${OBJ_ARCH}/plotspectrast : ${OBJDIR}/plotspectrast.o ${OBJDIR}/SpectraSTLibEntry.o ${OBJDIR}/SpectraSTQuery.o ${OBJDIR}/SpectraSTPeakList.o ${OBJDIR}/SpectraSTDenoiser.o ${OBJDIR}/SpectraSTLog.o ${OBJDIR}/FileUtils.o ${OBJDIR}/Peptide.o ${MYCRAMP}
	$(CXX) -O2 $^ $(LDFLAGS) -o $@ 

${OBJ_ARCH}/Lib2HTML : ${OBJDIR}/Lib2HTML.o ${OBJDIR}/SpectraSTPeptideLibIndex.o ${OBJDIR}/SpectraSTMzLibIndex.o ${OBJDIR}/SpectraSTLibIndex.o ${OBJDIR}/SpectraSTLibEntry.o ${OBJDIR}/SpectraSTPeakList.o ${OBJDIR}/SpectraSTDenoiser.o ${OBJDIR}/SpectraSTLog.o ${OBJDIR}/FileUtils.o ${OBJDIR}/Peptide.o ${MYCRAMP}
	$(CXX) -O2 $^ $(LDFLAGS) -o $@

${OBJDIR}/plotspectrast_cgi.o : plotspectrast.cpp 
	$(CXX) $(LFSFLAGS) $(IFLAGS) ${WARNINGFLAGS} -D RUN_AS_CGI -O2 -o ${OBJDIR}/plotspectrast_cgi.o -c plotspectrast.cpp 

ifeq (${LGPL},1)

else
${OBJ_ARCH}/kwset.o : ${SRC_ROOT}/refresh_interact/kwset.cxx ${SRC_ROOT}/refresh_interact/kwset.h ${SRC_ROOT}/refresh_interact/obstack.h
	$(CXX) $(LFSFLAGS) $(IFLAGS) ${WARNINGFLAGS} -O2 -o ${OBJ_ARCH}/kwset.o -c ${SRC_ROOT}/refresh_interact/kwset.cxx 

${OBJ_ARCH}/obstack.o : ${SRC_ROOT}/refresh_interact/obstack.cxx ${SRC_ROOT}/refresh_interact/obstack.h
	$(CXX) $(LFSFLAGS) $(IFLAGS) ${WARNINGFLAGS} -O2 -o ${OBJ_ARCH}/obstack.o -c ${SRC_ROOT}/refresh_interact/obstack.cxx 
endif

clean:
	rm -rf ${OBJDIR}; rm -f $(EXE) core* *~

debug:
	DEBUG='-O2'  make all

#
# dependencies
#

${OBJDIR}/SpectraSTLib.o : SpectraSTLib.cpp  ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTLibIndex.o : SpectraSTLibIndex.cpp  ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTMzLibIndex.o : SpectraSTMzLibIndex.cpp  ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTLibEntry.o : SpectraSTLibEntry.cpp  ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTPeakList.o : SpectraSTPeakList.cpp  ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTLibImporter.o : SpectraSTLibImporter.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTMspLibImporter.o : SpectraSTMspLibImporter.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTPepXMLLibImporter.o : SpectraSTPepXMLLibImporter.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTSpLibImporter.o : SpectraSTSpLibImporter.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTMs2LibImporter.o : SpectraSTMs2LibImporter.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTXHunterLibImporter.o : SpectraSTXHunterLibImporter.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTTsvLibImporter.o : SpectraSTTsvLibImporter.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTMzXMLLibImporter.o : SpectraSTMzXMLLibImporter.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTFastaFileHandler.o : SpectraSTFastaFileHandler.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTSearchTask.o : SpectraSTSearchTask.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTSearchTaskStats.o : SpectraSTSearchTaskStats.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTMspSearchTask.o : SpectraSTMspSearchTask.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTMzXMLSearchTask.o : SpectraSTMzXMLSearchTask.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTDtaSearchTask.o : SpectraSTDtaSearchTask.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTCandidate.o : SpectraSTCandidate.cpp  ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTQuery.o : SpectraSTQuery.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTFileList.o : SpectraSTFileList.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTSearch.o : SpectraSTSearch.cpp  ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTReplicates.o : SpectraSTReplicates.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTSearchOutput.o : SpectraSTSearchOutput.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTTxtSearchOutput.o : SpectraSTTxtSearchOutput.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTXlsSearchOutput.o : SpectraSTXlsSearchOutput.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTPepXMLSearchOutput.o : SpectraSTPepXMLSearchOutput.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTHtmlSearchOutput.o : SpectraSTHtmlSearchOutput.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTPeptideLibIndex.o : SpectraSTPeptideLibIndex.cpp ${SPECTRAST_HPP}
${OBJDIR}/SpectraSTSimScores.o : SpectraSTSimScores.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTSearchParams.o : SpectraSTSearchParams.cpp  ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTCreateParams.o : SpectraSTCreateParams.cpp  ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTLog.o : SpectraSTLog.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTDenoiser.o : SpectraSTDenoiser.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTMain.o : SpectraSTMain.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraSTMainLIB.o : SpectraSTMain.cpp ${SPECTRAST_HPP} 
	$(CXX) $(LFSFLAGS) $(IFLAGS) ${WARNINGFLAGS} -DLIB_BUILD -O2 -o $@ -c SpectraSTMain.cpp 
${OBJDIR}/FileUtils.o : FileUtils.cpp ${SPECTRAST_HPP} 
${OBJDIR}/Peptide.o : Peptide.cpp ${SPECTRAST_HPP} 
${OBJDIR}/XMLWalker.o : XMLWalker.cpp ${SPECTRAST_HPP} 
${OBJDIR}/Predicate.o : Predicate.cpp ${SPECTRAST_HPP} 
${OBJDIR}/ProgressCount.o : ProgressCount.cpp ${SPECTRAST_HPP} 
${OBJDIR}/plotspectrast.o : plotspectrast.cpp ${SPECTRAST_HPP} 
${OBJDIR}/plotspectrast_cgi.o : plotspectrast.cpp ${SPECTRAST_HPP} 
${OBJDIR}/Lib2HTML.o : Lib2HTML.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraST_base64.o : SpectraST_base64.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraST_ramp.o :  SpectraST_ramp.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraST_cramp.o : SpectraST_cramp.cpp ${SPECTRAST_HPP} 
${OBJDIR}/SpectraST_util.o : SpectraST_util.cpp ${SPECTRAST_HPP} 
