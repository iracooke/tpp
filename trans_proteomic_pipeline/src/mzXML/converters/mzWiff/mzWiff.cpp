// -*- mode: c++ -*-


/*
    File: mzWiff.cpp
    Description: This program converts Analyst/AnalystQS .wiff native data 
                 into mzXML format. It requires the Analyst/AnalystQS library 
                 from ABI to run.
    Date: July 31, 2007

    Copyright (C) 2007 Chee Hong WONG, Bioinformatics Institute
              The program depends on the general framework developed by
              Natalie Tasman, ISB Seattle

              This program is a re-write of (VB) mzStar by
              David Shteynberg and Robert Hubley based off
              ReAdW by Pedrioli Patrick.

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
#include "mzWiff.h"

#include "AnalystImpl.h"
#include "AnalystBaseInterface.h"
#include "ProgramOptions.h"
#include "mzXML/common/mzXMLWriter.h"
#include "mzXML/common/mzMLWriter.h"
#include "mzXML/common/MSTypes.h"
#include "mzXML/common/MSUtilities.h"
#include "common/TPPVersion.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

// TODO:
string G_version;

//functions declaration
void usage(const char* executable);
int translate(const ProgramOptions &programOptions, const char *exeName);
bool getUNCOrFullPath(const string &pathname, string &resultPath);
//END-functions declaration

// The one and only application object
CWinApp theApp;

const char *getVersion() {
	if (G_version.empty()) {
		G_version = toString(TPP_MAJOR_RELEASE_NUMBER) + "." + toString(TPP_MINOR_RELEASE_NUMBER) + "." + toString(TPP_REV_RELEASE_NUMBER) + "(build "__DATE__" " __TIME__ ")";;
	}
	return G_version.c_str();
}

int main(int argc, char* argv[], char* envp[])
{
	const char *exeName = strrchr(argv[0], '\\');
	if (NULL==exeName)
		exeName = argv[0];
	else
		exeName++;

	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		cerr << _T("FATAL ERROR: MFC initialization failed") << endl;
		nRetCode = 1;
	}
	else
	{
		//process argument parameters
		ProgramOptions programOptions;
		if (programOptions.parseArgs(argc, argv)) {
			// perform the translation based on specified options

			if (programOptions.m_fmzMLMode) {
				cerr << "mzML output is not supported in this release." << endl;
				exit(0);
			}

			if (programOptions.m_iVerbose>=VERBOSE_DEBUG) {
				programOptions.printArgs();
			}

			string strUNCPathName;
			if (getUNCOrFullPath(programOptions.m_inputFileName, strUNCPathName)) {
				if (checkPath(strUNCPathName)) {
					programOptions.m_inputFileName = strUNCPathName;
					nRetCode = translate(programOptions, exeName);
				}
			} else {
				cerr << "FATAL ERROR: Could not resolve input file '"
					<< programOptions.m_inputFileName << "'" << endl;
			}
		} else {
			usage(exeName);
			nRetCode = 1;
		}
	}

	return nRetCode;
} /*main*/

void usage(const char* executable)
{
	cerr <<     endl;
	cerr << executable << " version " << getVersion() 
#ifdef NODEBUGDETAIL
		   << " (NODEBUGDETAIL)"
#endif
		   <<     endl;
	cerr <<     endl;
	cerr << "Usage: " << executable << " [options] <wiff file path> [<output file>]" << endl;
	cerr << endl;
	cerr << "  Information options=" << endl;
	cerr << "    -I\"<str>\"        where str specifies the ionisation used" << endl;
	cerr << "    -T\"<str>\"        where str specifies the mass spectrometry type" << endl;
	cerr << "    -D\"<str>\"        where str specifies detector used" << endl;
	cerr << "    -G               use information recorded in wiff file; default off" << endl;
	cerr << endl;
	//Console Text Alignment Ruler
	//Ruler:          1         2         3         4         5         6         7         8
	//Ruler:012345678901234567890123456789012345678901234567890123456789012345678901234567890
	cerr << "  General options=" << endl;
	cerr << "    --compress, -z   use zlib to compress peaks" << endl;
	cerr << "    --gzip, -g:      gzip the output file (independent of peak compression)" << endl;
	cerr << "    --verbose, -v    verbose mode; default: quiet mode" << endl;
	cerr << "    --mzXML          mzXML output format" << endl;
	cerr << "    --coordinate     report native scan refernce" << endl;
//	cerr << "    --mzML           mzML output format" << endl;
	cerr << "     -s<num>-<num>   report only these range of sample ids;" << endl;
	cerr << "                     -s1   : only sample#1" << endl;
	cerr << "                     -s3-  : only sample#3 onward" << endl;
	cerr << "                     -s-6  : only sample#1 to sample#6" << endl;
	cerr << "                     -s2-4 : only sample#2 to sample#4" << endl;
	cerr << "    --AnalystQS      assume AnalystQS library" << endl;
	cerr << "    --Analyst        assume Analyst library" << endl;
	cerr << "    --only-compatible" << endl;
	cerr << "                     convert only if library is compatible" << endl;
	cerr << "    --showwifftree   print wiff tree structure on console" << endl;
	cerr << endl;
	cerr << "  Processing Operation options=" << endl;
	cerr << "    -GC              determine precursor charge; default off" << endl;
	cerr << "    -PI<num1>-<num2> where num1 and num2 are float specifying min and max peak" << endl;
	cerr << "                     intensity for MS/MS to be considered as signal; default 0" << endl;
	cerr << "                     (i.e. no thresholding applied)" << endl;
	cerr << "                     -PI0.3     : only peak with intensity >=0.3" << endl;
	cerr << "                     -PI0.3-    : same as -PI0.3" << endl;
	cerr << "                     -PI-6.4    : only peak with intensity <=6.4" << endl;
	cerr << "                     -PI0.3-6.4 : only peak satisfying 0.3<=intensity<=6.4" << endl;
	cerr << "    -PP<num1>-<num2> where num1 and num2 are float specifying min and max % of" << endl;
	cerr << "                     max peak intensity for MS/MS to be considered as signal; " << endl;
	cerr << "                     default 0 (0-100); instrument and data dependent" << endl;
	cerr << "                     -PP0.1      : only peak with intensity >=0.1% of maxInt" << endl;
	cerr << "                     -PP0.1-     : same as -PP0.1" << endl;
	cerr << "                     -PP-97.6    : only peak with intensity <=97.6% of maxInt" << endl;
	cerr << "                     -PP0.1-97.6 : only peak satisfying " << endl;
	cerr << "                                   0.1<=intensity/maxInt*100<=97.6" << endl;
	cerr << "    -P1I<num1>-<num2> same as -PI<num1>-<num2> above but applies to MS data" << endl;
	cerr << "    -P1P<num1>-<num2> same as -PP<num1>-<num2> above but applies to MS data" << endl;
	cerr << "    -c1              centroid MS data; default off" << endl;
	cerr << "    --centroid, -c   centroid MS/MS data; default off" << endl;
	cerr << "    --deisotope, -d  deisotope MS/MS data; default off" << endl;
	cerr << endl;
	cerr << "  MS/MS Averaging options=" << endl;
	cerr << "    -GPM<num>      where num is a float specifying the precursor mass tolerance" << endl;
	cerr << "                   to be considered for grouping; unit da; default: 0, i.e." << endl;
	cerr << "                   no MS2 averaging; suggested: 1" << endl;
	cerr << "    -GMA<num>      where num is a int specifying the max cycles span allowed" << endl;
	cerr << "                   within a group; default 10" << endl;
	cerr << "    -GMI<num>      where num is a int specifying the min cycles per group;" << endl;
	cerr << "                   within a group; default 1" << endl;
	cerr << endl;
	cerr << "  MS/MS Filtering options=" << endl;
	cerr << "    -FPC<num>      where num is a int specifying the min peak count to include" << endl;
	cerr << "                   a spectra in output; default 10" << endl;
	cerr << endl;
	cerr << "  output file:     Filename for mzXML/mzML output;" << endl;
	cerr << "                   if not supplied, the wiff filename will be used," << endl;
	cerr << "                   with the extension reset to .mzXML/.mzML." << endl;
	cerr << endl;
} /*usage*/


inline void timestampReport(const char *szPrefix, time_t &timestamp)
{
	char timebuf[26];
	time(&timestamp);
	ctime_s(timebuf, 26, &timestamp);
	cout << szPrefix << timebuf << endl;
}

inline void reportTimeDifference (const char *szPrefix, time_t startTime, time_t endTime)
{
	time_t lDifference = endTime - startTime;
	time_t lSeconds = lDifference % 60;
	time_t lMinutes = 0;
	lDifference -= lSeconds;
	lDifference = lDifference / 60;
	lMinutes = lDifference % 60;
	time_t lHours = 0;
	lDifference -= lMinutes;
	lHours = lDifference / 60;
	cout << szPrefix;
	if (lHours > 0) {
		cout << lHours << " hr ";
	}
	if (lMinutes > 0) {
		cout << lMinutes << " min ";
	}
	if (lSeconds > 0 || (lHours == 0 && lMinutes==0)) {
		cout << lSeconds << " sec ";
	}
	cout << endl;
}

inline void makeOutputFilename(string &outputFilename, long lSampleId) {
	string suffix("-s");
	suffix.append(toString(lSampleId));

	string::size_type nPos = outputFilename.rfind(".");
	if (string::npos == nPos) {
		outputFilename.append(suffix);
	} else {
		outputFilename.insert(nPos, suffix);
	}
}

int translate(const ProgramOptions &programOptions, const char *exeName)
{
	// report start time
	time_t startTime;
	time_t endTime;
	time_t startSampleTime;
	time_t endSampleTime;
	timestampReport("Started: ", startTime);

	int nRetCode = 0;

	// dump structure first
	if (programOptions.m_fShowWiffTree) {
		WiffStructureLoader wiffStructure(programOptions.m_inputFileName, programOptions.m_iVerbose);
		wiffStructure.load(true);
	}

	// perform the translation
	{
		bool fToProceed=true;
		//auto_ptr<InstrumentInterface> apInstrumentInterface;
		auto_ptr<AnalystBaseInterface> apInstrumentInterface;

		string strLibraryVersion;
		if (programOptions.m_fAnalystLib || 0 == getLibraryVersion(LIBRARY_Analyst, strLibraryVersion)) {
			if (programOptions.m_fAnalystLib) {
				cerr << "INFO: Assuming Analyst library installed." << endl;
			}

			apInstrumentInterface = auto_ptr<AnalystBaseInterface>(getAnalystInterface(true));

			apInstrumentInterface->assumeLibrary(programOptions.m_fAnalystLib);
			/*if (!programOptions.m_strModel.empty()) {
				apInstrumentInterface->instrumentInfo_.instrumentModel_ = programOptions.m_strModel;
			}*/
			if (!programOptions.m_strIonisation.empty()) {
				apInstrumentInterface->instrumentInfo_.ionSource_ = MSIonizationTypeFromString(programOptions.m_strIonisation);
			}
			if (!programOptions.m_strMSType.empty()) {
				apInstrumentInterface->instrumentInfo_.analyzerList_[0] = MSAnalyzerTypeFromString(programOptions.m_strMSType);
			}
			if (!programOptions.m_strDetector.empty()) {
				apInstrumentInterface->instrumentInfo_.detector_ = MSDetectorTypeFromString(programOptions.m_strDetector);
			}

			apInstrumentInterface->setVerbose(programOptions.m_iVerbose);
			apInstrumentInterface->setCentroiding(programOptions.m_fCentroid);
			apInstrumentInterface->setDeisotoping(programOptions.m_fDeisotope);

			apInstrumentInterface->m_DPSettings.m_dPeakCutoffMin = programOptions.m_dPeakCutoffMin;
			apInstrumentInterface->m_DPSettings.m_dPeakCutoffPercentageMin = programOptions.m_dPeakCutoffPercentageMin;
			apInstrumentInterface->m_DPSettings.m_dPeakCutoffMax = programOptions.m_dPeakCutoffMax;
			apInstrumentInterface->m_DPSettings.m_dPeakCutoffPercentageMax = programOptions.m_dPeakCutoffPercentageMax;

			apInstrumentInterface->m_DPSettings.m_dPeakCutoffMinMS1 = programOptions.m_dPeakCutoffMinMS1;
			apInstrumentInterface->m_DPSettings.m_dPeakCutoffPercentageMinMS1 = programOptions.m_dPeakCutoffPercentageMinMS1;
			apInstrumentInterface->m_DPSettings.m_dPeakCutoffMaxMS1 = programOptions.m_dPeakCutoffMaxMS1;
			apInstrumentInterface->m_DPSettings.m_dPeakCutoffPercentageMaxMS1 = programOptions.m_dPeakCutoffPercentageMaxMS1;

			apInstrumentInterface->m_DPSettings.m_dGroupPrecursorMassTolerance = programOptions.m_dGroupPrecursorMassTolerance;
			apInstrumentInterface->m_DPSettings.m_iGroupMaxCyclesBetween = programOptions.m_iGroupMaxCyclesBetween;
			apInstrumentInterface->m_DPSettings.m_iGroupMinCycles = programOptions.m_iGroupMinCycles;
			apInstrumentInterface->m_DPSettings.m_fGuessCharge = programOptions.m_fGuessCharge;

			apInstrumentInterface->m_DPSettings.m_iFilterMinPeakCount = programOptions.m_iFilterMinPeakCount;

			apInstrumentInterface->m_DPSettings.m_fCentroidMS1 = programOptions.m_fCentroidMS1;

			apInstrumentInterface->m_DPSettings.m_fCoordinate = programOptions.m_fCoordinate;

		} else if (programOptions.m_fAnalystLibQS || 0 == getLibraryVersion(LIBRARY_AnalystQS, strLibraryVersion)) {
			if (programOptions.m_fAnalystLibQS) {
				cerr << "INFO: Assuming Analyst library installed." << endl;
			}

			apInstrumentInterface = auto_ptr<AnalystBaseInterface>(getAnalystQSInterface(true));

			apInstrumentInterface->assumeLibrary(programOptions.m_fAnalystLibQS);
			/*if (!programOptions.m_strModel.empty()) {
				apInstrumentInterface->instrumentInfo_.instrumentModel_ = programOptions.m_strModel;
			}*/
			if (!programOptions.m_strIonisation.empty()) {
				apInstrumentInterface->instrumentInfo_.ionSource_ = MSIonizationTypeFromString(programOptions.m_strIonisation);
			}
			if (!programOptions.m_strMSType.empty()) {
				apInstrumentInterface->instrumentInfo_.analyzerList_[0] = MSAnalyzerTypeFromString(programOptions.m_strMSType);
			}
			if (!programOptions.m_strDetector.empty()) {
				apInstrumentInterface->instrumentInfo_.detector_ = MSDetectorTypeFromString(programOptions.m_strDetector);
			}

			apInstrumentInterface->setVerbose(programOptions.m_iVerbose);
			apInstrumentInterface->setCentroiding(programOptions.m_fCentroid);
			apInstrumentInterface->setDeisotoping(programOptions.m_fDeisotope);

			apInstrumentInterface->m_DPSettings.m_dPeakCutoffMin = programOptions.m_dPeakCutoffMin;
			apInstrumentInterface->m_DPSettings.m_dPeakCutoffPercentageMin = programOptions.m_dPeakCutoffPercentageMin;
			apInstrumentInterface->m_DPSettings.m_dPeakCutoffMax = programOptions.m_dPeakCutoffMax;
			apInstrumentInterface->m_DPSettings.m_dPeakCutoffPercentageMax = programOptions.m_dPeakCutoffPercentageMax;

			apInstrumentInterface->m_DPSettings.m_dPeakCutoffMinMS1 = programOptions.m_dPeakCutoffMinMS1;
			apInstrumentInterface->m_DPSettings.m_dPeakCutoffPercentageMinMS1 = programOptions.m_dPeakCutoffPercentageMinMS1;
			apInstrumentInterface->m_DPSettings.m_dPeakCutoffMaxMS1 = programOptions.m_dPeakCutoffMaxMS1;
			apInstrumentInterface->m_DPSettings.m_dPeakCutoffPercentageMaxMS1 = programOptions.m_dPeakCutoffPercentageMaxMS1;

			apInstrumentInterface->m_DPSettings.m_dGroupPrecursorMassTolerance = programOptions.m_dGroupPrecursorMassTolerance;
			apInstrumentInterface->m_DPSettings.m_iGroupMaxCyclesBetween = programOptions.m_iGroupMaxCyclesBetween;
			apInstrumentInterface->m_DPSettings.m_iGroupMinCycles = programOptions.m_iGroupMinCycles;
			apInstrumentInterface->m_DPSettings.m_fGuessCharge = programOptions.m_fGuessCharge;

			apInstrumentInterface->m_DPSettings.m_iFilterMinPeakCount = programOptions.m_iFilterMinPeakCount;

			apInstrumentInterface->m_DPSettings.m_fCentroidMS1 = programOptions.m_fCentroidMS1;

			apInstrumentInterface->m_DPSettings.m_fCoordinate = programOptions.m_fCoordinate;

		} else {
			cerr << "ERROR: No " << getLibraryName(LIBRARY_Analyst) << "/"
				<< getLibraryName(LIBRARY_AnalystQS)<<" library found." << endl;
			cerr << "ERROR: You can use the '--Analyst' if you are sure that Analyst is installed," << endl;
			cerr << "ERROR:          or the '--AnalystQS' if you are sure that AnalystQS is installed." << endl;
			fToProceed=false;
		}

		try {
			if (fToProceed) {
				if (!(fToProceed = apInstrumentInterface->initInterface())) {
					nRetCode = COMPATIBILITY_No_Library_Available;
					cerr << "ERROR: unable to interface with library" << endl;
				}
			}

			if (fToProceed) {
				if (!(fToProceed = apInstrumentInterface->setInputFile(programOptions.m_inputFileName))) {
					cerr << "ERROR: unable to open " << programOptions.m_inputFileName.c_str() << " with interface" << endl;
				}
				nRetCode = apInstrumentInterface->getCompatibilityTest();
				if (!programOptions.m_fmzXMLMode && !programOptions.m_fmzMLMode) {
					// we are just suppose to report compatibility
					cerr << "INFO: Output format mzXML or mzML has not been specified, testing compatibility." << endl;
					cerr << "INFO: Compatibility test returns " << nRetCode << endl;
					fToProceed = false;
				}
				if (programOptions.m_fOnlyCompatible && nRetCode != COMPATIBILITY_OK) {
					// we can only proceed if compatibility test passed
					cerr << "INFO: '--only-compatible' switch specified and Compatibility test returns " << nRetCode << endl;
					cerr << "INFO: Compatibility could not be ensured.  HALTING program." << endl;
					fToProceed = false;
				}
			}

			if (fToProceed) {
				string strTimestampPrefix;
				long lNumberOfSamples = (long)apInstrumentInterface->getNumberOfSamples();

				long lSampleStartId = (0==programOptions.m_iSampleStartId) ? 1 : programOptions.m_iSampleStartId;
				long lSampleEndId = (0==programOptions.m_iSampleEndId) ? lNumberOfSamples : programOptions.m_iSampleEndId;

				for (long lSampleIndex=lSampleStartId; lSampleIndex<=lSampleEndId; lSampleIndex++) {
					strTimestampPrefix.resize(0);
					strTimestampPrefix.append("Started: Sample#").append (toString(lSampleIndex)).append(": ");
					timestampReport(strTimestampPrefix.c_str(), startSampleTime);

					fToProceed = true;
					try {
						if (apInstrumentInterface->setSample(lSampleIndex)) {
							auto_ptr<MassSpecXMLWriter> apMSWriter;
							if (programOptions.m_fmzXMLMode) {

								auto_ptr<MassSpecXMLWriter> apWriter(new mzXMLWriter(exeName, getVersion(), apInstrumentInterface.get()));
								apWriter->setVerbose(programOptions.m_iVerbose>0);
								apMSWriter = auto_ptr<MassSpecXMLWriter>(apWriter.release());

							} else if (programOptions.m_fmzMLMode) {

								auto_ptr<MassSpecXMLWriter> apWriter(new mzMLWriter(exeName, getVersion(), apInstrumentInterface.get()));
								apWriter->setVerbose(programOptions.m_iVerbose>0);
								apMSWriter = auto_ptr<MassSpecXMLWriter>(apWriter.release());

							} else {
								//test for compatibility and return
								//cerr << "ERROR: Please specify one of mzXML or mzML output format" << endl;
								fToProceed = false;
								break;  // no reason to try all sample given unknown format
							}

							if (fToProceed) {
								if (!(fToProceed = apMSWriter->setInputFile(programOptions.m_inputFileName))) {
									nRetCode = 2;
									cerr << "ERROR: unable to set input file " << programOptions.m_inputFileName.c_str() << endl;
								}
							}

							if (fToProceed) {
								string filenameWithSampleId(programOptions.m_outputFileName);
								makeOutputFilename(filenameWithSampleId, lSampleIndex);
								if (!(fToProceed = apMSWriter->setOutputFile(filenameWithSampleId))) {
									nRetCode = 4;
									cerr << "ERROR: unable to open " << filenameWithSampleId.c_str() << endl;
								}
							}

							if (fToProceed) {
								apMSWriter->setCompression(programOptions.m_fCompression);
								apMSWriter->setCentroiding(programOptions.m_fCentroid);
								apMSWriter->setDeisotoping(programOptions.m_fDeisotope);
								apMSWriter->writeDocument();
							}
						} else {
							cerr << endl << endl << "ERROR: Failed to load sample#" << lSampleIndex << endl;
						}
					} catch (_com_error &com_error) {
						cerr << endl << endl 
							<< "ERROR: COM error " 
							<< std::hex << com_error.Error() << std::dec 
							<< " while processing sample#" << lSampleIndex
							<< endl;
						printCOMErrorString(com_error.Error());
						nRetCode = com_error.Error();
					}

					strTimestampPrefix.resize(0);
					strTimestampPrefix.append("Ended: Sample#").append (toString(lSampleIndex)).append(": ");
					timestampReport(strTimestampPrefix.c_str(), endSampleTime);

					strTimestampPrefix.resize(0);
					strTimestampPrefix.append("Elapsed: Sample#").append (toString(lSampleIndex)).append(": ");
					reportTimeDifference (strTimestampPrefix.c_str(), startSampleTime, endSampleTime);
				}
			}
		} catch (_com_error &com_error) {
			cerr << endl << endl << "ERROR: COM error " << std::hex << com_error.Error() << std::dec << endl;
			printCOMErrorString(com_error.Error());
			nRetCode = com_error.Error();
		}
	}
	// report end time
	timestampReport("Ended: ", endTime);

	// report elapsed time
	reportTimeDifference ("Elapsed: ", startTime, endTime);

	return nRetCode;

} /*translate*/

bool getUNCOrFullPath(const string &pathname, string &resultPath)
{
	resultPath.resize(0);

  basic_string<TCHAR> tcPathname;
	USES_CONVERSION;
	tcPathname = A2T(pathname.c_str());

	TCHAR tcDrivePath[_MAX_DRIVE+_MAX_DIR+_MAX_FNAME+_MAX_EXT];
	TCHAR *ptcFilePart;
  DWORD nBufferLength=_MAX_DRIVE+_MAX_DIR+_MAX_FNAME+_MAX_EXT;

	if (0==GetFullPathName(tcPathname.data(), nBufferLength, tcDrivePath, &ptcFilePart))
		return false;

	resultPath = T2A(tcDrivePath);
	if (0==resultPath.find("\\\\")) {
		// already a UNC path
		return true;
	}

	if (resultPath.length()>2 && ':'==resultPath[1]) {
		// we have a drive letter, let's check if we need to get path
		tcDrivePath[2] = 0;  // just keep "<drive>:"

		if (DRIVE_REMOTE == GetDriveType(tcDrivePath)) {
			TCHAR tcRemoteName[_MAX_DRIVE+_MAX_DIR+_MAX_FNAME+_MAX_EXT+1]={0};
			nBufferLength=_MAX_DRIVE+_MAX_DIR+_MAX_FNAME+_MAX_EXT+1;
			if (0!=WNetGetConnection(tcDrivePath, tcRemoteName, &nBufferLength)) {
				// it is a remote device, but we fail to retrieve information
				return false;
			}

			// let's replace the drive part with share volume
			resultPath.replace(0,2, T2A(tcRemoteName));
		}
	}

	return true;
} /*getUNCOrFullPath*/

