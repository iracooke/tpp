CPPC    = g++
CPP_FILE= mzXMLRecalibratorSHA1.cpp mzXMLRecalibratorSHA1Wrapper.cpp mzXMLRecalibrator.cpp mzXMLRecalibratorOptions.cpp mzXMLRecalibratorBase64.cpp mzXMLUtils.cpp mzXMLRecalibratorRamp.cpp guessCalibInterval.cpp calibrationRoutines.cpp
OBJS    = mzXMLRecalibrator.o  mzXMLRecalibratorSHA1.o mzXMLRecalibratorSHA1Wrapper.o mzXMLRecalibratorOptions.o mzXMLRecalibratorBase64.o mzXMLUtils.o mzXMLRecalibratorRamp.o guessCalibInterval.o calibrationRoutines.o
CFLAGS  = -g -I. -I./include -I/usr/include/libxml2  -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
EXE     = mzXMLRecalibrator

all:
	$(CPPC) -c $(CFLAGS) $(CPP_FILE)
	$(CPPC) -o $(EXE) $(OBJS) -lm  -lxml2


clean:
	rm -f *.o $(EXE) core
