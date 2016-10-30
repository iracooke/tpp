// -*- mode: c++ -*-

/*
	Program: MHConverter
	Description: converts Aglient MassHunter / SpectrumMill native 
	mass spectroscopy data into open XML formats (current mzXML 
	and beta mzML).  Please note, this program requires the
	MassHunter libraries from Agilent.

	Date: August 2008
	Author: Natalie Tasman, ISB Seattle, 2008

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


#include <string>

#include "mzXML/common/mzXMLWriter.h"
#include "mzXML/common/mzMLWriter.h"
#include "ConverterArgs.h"

#include "MassHunterInterface.h"

#include "MHConverter.h"
#include "common/TPPVersion.h"
#include "mzXML/common/MSUtilities.h"

using namespace std;



void MHConverter::usage(const string& exename, const string& version) {
	cerr << endl << exename <<  " version " << version <<  endl << endl;
	cerr << "    requires MHDAC API version 1.3.1" << endl << endl;
	cerr << "Usage: " << exename << " [options] <raw file path> [<output file>]" << endl
		<< " Options\n"
		<< "  --mzXML:         mzXML mode" << endl
//		<< "                    (--mzML will be supported in an upcoming version;" << endl
//		<< "                     one of --mzXML or --mzML must be selected)" << endl
//<< "  --mzML:          mzML mode" << endl
		//<< "                    (one of --mzXML or --mzML must be selected)" << endl
		<< endl
		<< "  --centroid, -c:  Centroid scans (meaningful only" << endl
		<< "                    if data was acquired in profile mode;"  << endl
		<< "                     default: not active)" << endl
		//<< "  --compress, -z:  Use zlib for comrpessing peaks (" << endl
		//<< "                    default: not active)." << endl
		<< "  --gzip, -g:   gzip the output file (independent of peak compression)" << endl

		<< "  --verbose, -v:   verbose" << endl
		<< "  --deisotope" << endl
		<< "  --relTol <n.n>" << endl
		<< "  --absTol <n.n>" << endl
		<< "  --maxCharge <n>" << endl
		<< "  --requirePepProfile" << endl
		<< endl
		<< "  output file:     Filename for output file;" << endl
		<< "                    if not supplied, the raw file's name" << endl
		<< "                    will be used," << endl
		<< "                    with the extension changed," << endl
		<< "                    and file output in current directory." << endl
		<< endl
		<< endl
		<< "Example:" << endl
		<< " convert input.d directory to output.mzXML:" << endl
		<< "  " << exename << " --mzXML C:\\test\\input.d c:\\test\\output.mzXML" << endl << endl
		<< "Author: Natalie Tasman (ISB/SPC)" << endl;

}





int MHConverter::run(int argc, char* argv[], MHDACWrapper* mhdacWrapper)
{
	// force the program name to trapper,
	// regardless of what it was called on the command line
	const char *execName = "trapper"; // argv[0];
	//SYSTEMTIME systime;
	//GetSystemTime(&systime);
	//CTime time(systime);
	//time.Format("%Y-%m-%d");
	string version = toString(TPP_MAJOR_RELEASE_NUMBER) + "." + toString(TPP_MINOR_RELEASE_NUMBER) + "." + toString(TPP_REV_RELEASE_NUMBER);
	version += " (build "__DATE__" " __TIME__ ")";

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

	if (args.mzMLMode) {
		cout << "mzML output is not supported in this release." << endl;
		exit(0);
	}


	if (args.verbose) {
		args.printArgs();
	}

	


	if (args.mzMLMode) {
		cerr << "mzML mode is not supported in this version." << endl;
		exit(0);
	}

	if (args.compressScans) {
		cerr << "sorry, scan compression is not supported in this version." << endl;
		exit(0);
	}

	MassHunterInterface massHunterInterface(mhdacWrapper);
	if (args.verbose) {
		massHunterInterface.setVerbose(true);
	}

	if (args.deisotope_) {
		massHunterInterface.doDeisotope_ = true;
	}
	if (args.absoluteTolerance_ > 0) {
		massHunterInterface.absoluteTolerance_ = args.absoluteTolerance_;
	}
	if (args.relativeTolerance_ > 0) {
		massHunterInterface.relativeTolerance_ = args.relativeTolerance_;
	}
	// TODO: clean up limit/max checking
	if (args.maxChargeState_ > 0) {
		massHunterInterface.maxChargeState_ = args.maxChargeState_;
		massHunterInterface.limitChargeState_ = true;
	}
	if (args.requirePeptideLikeAbundanceProfile_) {
		massHunterInterface.requirePeptideLikeAbundanceProfile_ = true;
	}


	
	
	// try to init the MassHunter library
	if (!massHunterInterface.initInterface()) {
		cerr << "unable to interface with MassHunter library" << endl;
		exit(-1);
	}

	// must do this before setInputFile;
	// spectra to be extracted (via chromatogram obj) are seleted here
	// -- this applies to mixed-mode files (both profile and peak-picked included) 
	massHunterInterface.setCentroiding(args.centroidScans);
	if (!massHunterInterface.setInputFile(args.inputFileName)) {
		cerr << "unable to open " << args.inputFileName << " with MassHunter interface" << endl;
		exit(-1);
	}


	MassSpecXMLWriter* msWriter;
	if (args.mzMLMode) {
		msWriter = new mzMLWriter(execName, version, &massHunterInterface);
	} else if (args.mzXMLMode) {
		msWriter = new mzXMLWriter(execName, version, &massHunterInterface);
	}
	else {
		cerr << "either mzXML or mzML mode must be selected." << endl;
		exit(-1);
	}

	if (args.verbose) {
		msWriter->setVerbose(true);
	}

	//if (args.skipChecksum) {
	//	msWriter->setSkipChecksum(true);
	//}

	if (!msWriter->setInputFile(args.inputFileName)) {
		cerr << "unable to set input file " << args.inputFileName << endl;
		exit(-1);
	}

	if (!msWriter->setOutputFile(args.outputFileName)) {
		cerr << "unable to open " << args.outputFileName << endl;
		exit(-1);
	}

	// TODO: why does the writer do this rather than the instrument interface?
	msWriter->setCentroiding(args.centroidScans);
	msWriter->writeDocument();

	delete msWriter;
	return 0;
}
