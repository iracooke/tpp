// -*- mode: c++ -*-

/*
	Program: massWolf
	Description: converts MassLynx native data into open XML formats
	(current mzXML and beta mzML).  Please note, this program requires the 
	DAC library from Waters to run.

	Date: July 2007 Author: Natalie Tasman, ISB Seattle, 2007, with
	contributions from Rune Schjellerup Philosof; based on
	original work by Pedrioli Patrick, ISB, Proteomics (original
	author), and Brian Pratt, InSilicos
	
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


#include "stdafx.h"

#include "massWolf.h"
#include "MassLynxInterface.h"
#include "mzXML/common/mzXMLWriter.h"
#include "mzXML/common/mzMLWriter.h"
#include "mzXML/common/ConverterArgs.h"
#include "mzXML/common/MSUtilities.h"
#include "common/TPPVersion.h"

#include <string>
#include <iostream>

using namespace std;

void usage(const string& exename, const string& version) {
	cout << endl << exename << " " << version << endl << endl;
	cout << "Usage: " << exename << " [options] <raw file path> [<output file>]" << endl << endl
		<< " Options\n"
		<< "  --mzXML:         mzXML mode (default)" << endl
//		<< "  --mzML:          mzML mode (EXPERIMENTAL)" << endl
		<< "      one of --mzXML or --mzML must be selected" << endl
		<< endl
		<< "  --centroid, -c: EXPERIMENTAL" << endl
		<< "      Centroid all scans (MS1 and MS2);" << endl
		<< "      meaningful only if data was acquired in profile mode;"  << endl
		<< "      default: off" << endl
		<< "  --compress, -z: Use zlib for compressing peaks" << endl
		<< "      default: off" << endl
		<< "  --verbose, -v:   verbose" << endl
		<< "  --gzip, -g:   gzip the output file (independent of peak compression)" << endl
		<< "  --nolockspray:   Normally " << exename << " will annotate the lockspray function;" << endl
		<< "      If the last function is MS and not MSMS, then it is assumed to be lockspray;" << endl
		<< "      specify \'--nolockspray\' to turn this off if your last function is MS and not actually lockspray." << endl
		<< "  --MSe:           Use if your run is MS^E data (that is, if your run is composed of three functions: Low, High, Lockspray (or two functions-- Low then High only, if --nolockspray set.))" << endl
		<< endl
		<< "  output file: (Optional) Filename for output file;" << endl
		<< "      if not supplied, the output file will be created" << endl
		<< "      in the same directory as the input file." << endl
		<< "  *EXPERIMENTAL* --TD <value>: threshold, leaving all peaks with <value> or above, and discarding all other peaks" << endl
		<< "  *EXPERIMENTAL* --TZ <value>: threshold, leaving all peaks with <value> or above, and setting intensity of all other peaks to zero" << endl
		<< endl
		<< endl

		<< endl
		<< endl
		<< "Example: convert input.raw directory to output.mzXML" << endl << endl
		<< "      " << exename << " --mzXML C:\\test\\input.raw c:\\test\\output.mzXML" << endl << endl
		<< "Author: Natalie Tasman (SPC/ISB) with contributions from Rune Schjellerup Philosof; based on orignal work by Patrick Pedriolli and Brian Pratt" << endl;
}





int main(int argc, char* argv[])
{
	// force the program name to wolf,
	// regardless of what it was called on the command line
	const char *execName = "massWolf"; // argv[0];
	string version = toString(TPP_MAJOR_RELEASE_NUMBER) + "." + toString(TPP_MINOR_RELEASE_NUMBER) + "." + toString(TPP_REV_RELEASE_NUMBER) + "(build "__DATE__" " __TIME__ ")";
	//const char *execName = argv[0];
	//// remove preceeding path info from execName
	//const char* slashPos=strrchr(execName,'\\');
	//if ( slashPos ) {
	//	execName = slashPos+1;
	//}

	if (argc <= 2) {
		// min args: output mode and input file 
		usage(execName, version);
		return -1;
	}

	ConverterArgs args;

	if (!args.parseArgs(argc, argv)) {
		usage(execName, version);
		return -1;
	}
	if (args.verbose) {
		args.printArgs();
	}

	if (args.mzMLMode) {
		cout << "mzML output is not supported in this release." << endl;
		exit(0);
	}

	//if (args.centroidScans) {
	//	cerr << "sorry, centroiding is not yet supported." << endl;
	//	return -1;
	//}

	MassLynxInterface massLynxInterface;
	if (args.verbose) {
		massLynxInterface.setVerbose(true);
	}
	if (args.lockspray) {
		massLynxInterface.setLockspray(true);
	}
	if (args.shotgunFragmentation) {
		massLynxInterface.setShotgunFragmentation(true);
	}

	if (args.threshold) {
		massLynxInterface.setThresholdInclusiveCutoff(args.inclusiveCutoff);
		massLynxInterface.setThresholdDiscard(args.thresholdDiscard);
	}

	// try to init the MassLynx library
	if (!massLynxInterface.initInterface()) {
		cerr << "unable to interface with MassLynx library" << endl;
		exit(-1);
	}


	if (!massLynxInterface.setInputFile(args.inputFileName)) {
		cerr << "unable to open " << args.inputFileName << " with MassLynx interface" << endl;
		exit(-1);
	}



	MassSpecXMLWriter* msWriter;
	if (args.mzMLMode) {
		msWriter = new mzMLWriter(execName, version, &massLynxInterface);
	} else if (args.mzXMLMode) {
		msWriter = new mzXMLWriter(execName, version, &massLynxInterface);
	}
	else {
		cerr << "either mzXML or mzML mode must be selected." << endl;
		exit(-1);
	}

	if (args.verbose) {
		msWriter->setVerbose(true);
	}

	if (!msWriter->setInputFile(args.inputFileName)) {
		cerr << "unable to set input file " << args.inputFileName << endl;
		exit(-1);
	}

	if (!msWriter->setOutputFile(args.outputFileName)) {
		cerr << "unable to open " << args.outputFileName << endl;
		exit(-1);
	}

	msWriter->setCentroiding(args.centroidScans);
	if (args.centroidScans) {
		cout << endl << "WARNING: using EXPERIMENTAL scan centroiding functionality." << endl;
	}
	msWriter->setCompression(args.compressScans);
	msWriter->setShotgunFragmentation(args.shotgunFragmentation);
	msWriter->setLockspray(args.lockspray);

	msWriter->writeDocument();

	delete msWriter;
	return 0;
}
