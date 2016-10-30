# Include configuration variables for the build, including $(OBJ_ARCH)
SRC_ROOT ?= ../

include $(SRC_ROOT)Makefile.incl

UTIL_OBJ_ARCH = $(OBJ_ARCH)/util

# put here the complete path to the RAMP ROOT
RAMPROOT= ../Parsers/ramp
RAMPINC= -I . -I ${RAMPROOT} -I ../common -I ..

EXE=$(OBJ_ARCH)/checkdb $(OBJ_ARCH)/digestdb $(OBJ_ARCH)/mzxml2dta $(OBJ_ARCH)/plottic $(OBJ_ARCH)/subsetdb  $(OBJ_ARCH)/prophet_png $(OBJ_ARCH)/getSpectrum $(OBJ_ARCH)/seeker $(OBJ_ARCH)/pep_dbcount $(OBJ_ARCH)/fdr_plot $(OBJ_ARCH)/translateDNA2AA-FASTA $(OBJ_ARCH)/calculate_pi $(OBJ_ARCH)/add_mz

# handle 2GB+ files
CCOPT= -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -Wformat -Wstrict-aliasing  -Wno-deprecated -Wno-char-subscripts


# put here the name of c compiler
COMPILER = g++ $(CCOPT) 
CPP=mkdir -p $(UTIL_OBJ_ARCH) ; $(COMPILER) -o $@ ${RAMPINC}
LINK= $(COMPILER) -o $@ $^ $(LDFLAGS)


.SUFFIXES:	.o .cpp

$(UTIL_OBJ_ARCH)/%.o: %.cpp 
		$(CPP) -O3  -c $<


#
# targets
#
all : $(EXE)


$(OBJ_ARCH)/add_mz: $(UTIL_OBJ_ARCH)/add_mz.o
	$(LINK)

$(OBJ_ARCH)/calculate_pi: $(UTIL_OBJ_ARCH)/calculate_pi.o $(UTIL_OBJ_ARCH)/calc_pI.o
	$(LINK) -lm

$(OBJ_ARCH)/translateDNA2AA-FASTA: $(UTIL_OBJ_ARCH)/translateDNA2AA-FASTA.o
	$(LINK)

$(OBJ_ARCH)/pep_dbcount: $(UTIL_OBJ_ARCH)/pep_dbcount.o
	$(LINK)

$(OBJ_ARCH)/getSpectrum: $(UTIL_OBJ_ARCH)/getSpectrum.o $(UTIL_OBJ_ARCH)/ramp_base64.o $(UTIL_OBJ_ARCH)/ramp.o
	$(LINK) ${RAMPINC} -lz

$(OBJ_ARCH)/mzxml2dta: $(UTIL_OBJ_ARCH)/mzxml2dta1.o $(UTIL_OBJ_ARCH)/ramp_base64.o $(UTIL_OBJ_ARCH)/ramp.o
	$(LINK) ${RAMPINC} -lz

$(OBJ_ARCH)/subsetdb: $(UTIL_OBJ_ARCH)/subsetdb1.o
	$(LINK)

$(OBJ_ARCH)/checkdb: $(UTIL_OBJ_ARCH)/checkdb1.o $(UTIL_OBJ_ARCH)/SHA1_TPP.o
	$(LINK)

$(OBJ_ARCH)/plottic: $(UTIL_OBJ_ARCH)/plottic1.o
	$(LINK)

$(OBJ_ARCH)/seeker: $(UTIL_OBJ_ARCH)/seeker.o
	$(LINK)

$(OBJ_ARCH)/fdr_plot: $(UTIL_OBJ_ARCH)/fdr_plot.o
	$(LINK)

$(OBJ_ARCH)/digestdb: $(UTIL_OBJ_ARCH)/digestdb1.o $(UTIL_OBJ_ARCH)/calc_pI.o
	$(LINK) -lm

$(OBJ_ARCH)/prophet_png: $(UTIL_OBJ_ARCH)/prophet_png1.o
	$(LINK)

$(UTIL_OBJ_ARCH)/seeker.o: seeker.cpp
	$(CPP) -O2 -c seeker.cpp

$(UTIL_OBJ_ARCH)/fdr_plot.o: fdr_plot.cpp
	$(CPP) -O2 -c fdr_plot.cpp

clean:
	rm -f $(UTIL_OBJ_ARCH)/*.o $(EXE) core

debug:
	DEBUG='-g'  make all

#
# dependencies
#
$(UTIL_OBJ_ARCH)/getSpectrum.o : getSpectrum.cpp
$(UTIL_OBJ_ARCH)/mzxml2dta1.o : mzxml2dta1.cpp
$(UTIL_OBJ_ARCH)/readmzXML1.o : readmzXML1.cpp
$(UTIL_OBJ_ARCH)/subsetdb1.o : subsetdb1.cpp
$(UTIL_OBJ_ARCH)/digestdb1.o : digestdb1.cpp
$(UTIL_OBJ_ARCH)/calc_pI.o : calc_pI.cpp
$(UTIL_OBJ_ARCH)/prophet_png1.o : prophet_png1.cpp
$(UTIL_OBJ_ARCH)/seeker.o : seeker.cpp
$(UTIL_OBJ_ARCH)/fdr_plot.o : fdr_plot.cpp
$(UTIL_OBJ_ARCH)/calculate_pi.o : calculate_pi.cpp
$(UTIL_OBJ_ARCH)/add_mz.o : add_mz.cpp

$(UTIL_OBJ_ARCH)/ramp_base64.o : ${RAMPROOT}/ramp_base64.cpp ${RAMPROOT}/ramp_base64.h
	$(CPP) -c $<

$(UTIL_OBJ_ARCH)/ramp.o : ${RAMPROOT}/ramp.cpp ${RAMPROOT}/ramp.h
	$(CPP) -c $<
