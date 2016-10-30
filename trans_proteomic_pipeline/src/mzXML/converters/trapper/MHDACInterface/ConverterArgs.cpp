// -*- mode: c++ -*-


/*
    File: ConverterArgs.cpp
    Description: common program argument parsing for MS data converters.
    Date: July 25, 2007

    Copyright (C) 2007 Natalie Tasman, ISB Seattle


    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/


#include "ConverterArgs.h"
#include <iostream>

using namespace std;
#if defined(_MSC_VER) || defined(__MINGW32__)  // MSVC or MinGW
#ifndef strcasecmp
#define strcasecmp _stricmp
#endif
#endif

ConverterArgs::ConverterArgs() {
	centroidScans = false;
	compressScans = false;
	verbose = false;
	mzMLMode = false;
	mzXMLMode = false;
	skipChecksum = false;
	inputFileName="";
	outputFileName="";
	gzipOutputFile=false; // gzip the output file? (independent of peak compression)

	deisotope_ = false;
	requirePeptideLikeAbundanceProfile_ = false;
	relativeTolerance_ = 0;
	absoluteTolerance_ = 0 ;
	limitChargeState_ = false;
	maxChargeState_ =0 ;
	
}

void
ConverterArgs::printArgs(void) {
	cout 
		<< "Settings:" << endl
		<< "  centroid scans: " << centroidScans << endl
		<< "  compress scans: " << compressScans << endl
		<< "  verbose mode: " << verbose << endl
		<< "  skip checksum: " << skipChecksum << endl

		<< "  deisotope:" << deisotope_ << endl
		<< "  requirePeptideLikeAbundanceProfile: " << requirePeptideLikeAbundanceProfile_ << endl
		<< "  relativeTolerance: " << relativeTolerance_ << endl
		<< "  absoluteTolerance: " << absoluteTolerance_ << endl
		<< "  limitChargeState:" <<  limitChargeState_ << endl
		<< "  maxChargeState: " << maxChargeState_ << endl

		<< "  mzML mode: " << mzMLMode << endl
		<< "  mzXML mode: " << mzXMLMode << endl
		<< "  input filename: " << inputFileName << endl
		<< "  output filename: " << outputFileName << endl


		<< endl;
};

bool 
ConverterArgs::parseArgs(int argc, char* argv[]) {
	int curArg = 1;
	for( curArg = 1 ; curArg < argc ; ++curArg ) {
		if (*argv[curArg] == '-') {
			if (strlen(argv[curArg])==2 && 
				*(argv[curArg]+1) == 'c') {
					centroidScans = true;
			}
			else if (strstr(argv[curArg], "-centroid")) {
				centroidScans = true;
			}
			else if (strlen(argv[curArg])==2 && 
				*(argv[curArg]+1) == 'z') {	
					compressScans = true;
			}
			else if (strstr(argv[curArg], "-compress")) {
				compressScans = true;
			}
			else if (strcmp(argv[curArg], "-g") == 0) {
				gzipOutputFile = true;
			}
			else if (strcmp(argv[curArg], "--gzip") == 0) {
				gzipOutputFile = true;
			}

			else if (strlen(argv[curArg])==2 && 
				*(argv[curArg]+1) == 'v') {	
					verbose = true;
			}
			else if (strstr( argv[curArg] , "-verbose")) {
				verbose = true;
			}
			else if (strstr( argv[curArg] , "--mzXML")) {
				mzXMLMode = true;
			}
			else if (strstr( argv[curArg] , "--mzML")) {
				mzMLMode = true;
			}
			else if (strstr( argv[curArg] , "--skiphash")) {
				skipChecksum = true;
			}
			else if (strstr( argv[curArg] , "--deisotope")) {
				deisotope_ = true;
			}
			else if (strstr( argv[curArg] , "--relTol")) {
				++curArg;
				relativeTolerance_ = atof(argv[curArg]);
			}

			else if (strstr( argv[curArg] , "--absTol")) {
				++curArg;
				absoluteTolerance_ = atof(argv[curArg]);;
			}
			else if (strstr( argv[curArg] , "--maxCharge")) {
				limitChargeState_ = true;
				++curArg;
				maxChargeState_ = atoi(argv[curArg]);
			}
			else if (strstr( argv[curArg] , "--requirePepProfile")) {
				requirePeptideLikeAbundanceProfile_ = true;
			}

		}
		else {
			break;
		}
	}

	if (mzMLMode && mzXMLMode) {
		cerr << "  error: only one of mzXML or mzML mode can be selected" << endl;
		return false;
	}
	else if (!(mzXMLMode || mzMLMode)) {
		cerr << "  error: either mzXML or mzML mode must be selected" << endl;
		return false;
	}

	if (curArg > argc-1) {
		cerr << "  error: no input file specified" << endl;
		return false;
	} else {
		inputFileName = argv[curArg];
	}

	curArg++;
	if (curArg > argc-1) {
		// no output filename specified
		// cout << "(no output filename specified; using inputfile filename)" << endl;
		string::size_type dotPos = inputFileName.find_last_of('.');
		if (dotPos == string::npos) {
			cerr << "  error: input file name did not have extension" << endl;
			return false;
		}
		else {
			outputFileName = inputFileName.substr(0, dotPos);
			if (mzMLMode) {
				outputFileName += ".mzML";
			}
			else if (mzXMLMode) {
				outputFileName += ".mzXML";
			}
		}
	} else {
		outputFileName = argv[curArg];
	}

	if (gzipOutputFile) { // write entire file as gzip? (independent of scan compression)
		string::size_type dotPos = outputFileName.find_last_of('.');
		if ((dotPos==string::npos) || strcasecmp(outputFileName.c_str()+dotPos,".gz")) {
			outputFileName += ".gz";  // this will trigger gzip compression on output
		}
	}

	return true;
}
