// -*- mode: c++ -*-


/*
    File: ProgramOptions.cpp
    Description: program options parsing for mzWiff
    Date: July 31, 2007

    Copyright (C) 2007 Chee Hong WONG, Bioinformatics Institute


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


#include "ProgramOptions.h"
#include <iostream>

#if defined(_MSC_VER) || defined(__MINGW32__)  // MSVC or MinGW
#ifndef strcasecmp
#define strcasecmp _stricmp
#endif
#endif

bool parseRangeDouble (const char *arg, const char *parameterName, double &dMin, double &dMax, bool autoEnd)
{
	double dTmp;

	if (('0' <= arg[0] && '9' >= arg[0]) || '.' == arg[0]) {
		if (sscanf(arg, "%lf", &dTmp) != 1 || dTmp < 0.00) {
			printf("Bad min %s: '%s' ... ignored\n", parameterName, arg);
			return false;
		} else {
			dMin = dTmp;

			const char *pszHyphen = strstr(&arg[1], "-");
			if (NULL == pszHyphen) {
				if (autoEnd) {
					dMax = dMin;
				}
			} else {
				if (sscanf(pszHyphen+1, "%lf", &dTmp) == 1) {
					if (dTmp<dMin) {
						dMax = dMin;
						dMin = dTmp;
					} else {
						dMax = dTmp;
					}
				}
			}
		}
	} else if ('-' == arg[0] && 0 != arg[1]) {
		if (sscanf(&arg[1], "%lf", &dTmp) != 1) {
			printf("Bad max %s: '%s' ... ignored\n", parameterName, arg);
			return false;
		} else {
			dMax = dTmp;
		}
	} else {
		return false;
	}

	return true;
}

bool parseRangeInt (const char *arg, const char *minName, const char *maxName, int &iMin, int &iMax, bool autoEnd)
{
	int iTmp;

	if ('0' <= arg[0] && '9' >= arg[0]) {
		if (sscanf(arg, "%d", &iTmp) != 1) {
			printf("Bad %s: '%s'\n", minName, arg);
			return false;
		} else {
			iMin = iTmp;

			const char *pszHyphen = strstr(arg, "-");
			if (NULL == pszHyphen) {
				if (autoEnd) {
					iMax = iMin;
				}
			} else {
				if (sscanf(pszHyphen+1, "%d", &iTmp) != 1) {
					// we take it that until the end of sample list
				} else {
					if (iTmp<iMin) {
						iMax = iMin;
						iMin = iTmp;
					} else {
						iMax = iTmp;
					}
				}
			}
		}
	} else if ('-' == arg[0] && 0 != arg[1]) {
		if (sscanf(&arg[1], "%d", &iTmp) != 1) {
			printf("Bad %s: '%s'\n", maxName, &arg[1]);
			return false;
		} else {
			iMax = iTmp;
		}
	} else {
		return false;
	}

	return true;
}

ProgramOptions::ProgramOptions ()
:	m_fAnalystLibQS(false),
	m_fAnalystLib(false),
	m_strModel(""),
	m_strIonisation(""),
	m_strMSType(""),
	m_strDetector(""),
	m_fUseWiffFileInformation(false),
	m_fCompression(false),
	m_fGzipOutput(false),
	m_fCentroid(false),
	m_fCentroidMS1(false),
	m_dPeakCutoffMin(0.0),
	m_dPeakCutoffPercentageMin(0),
	m_dPeakCutoffMax(0.0),
	m_dPeakCutoffPercentageMax(0),
	m_dPeakCutoffMinMS1(0.0),
	m_dPeakCutoffPercentageMinMS1(0),
	m_dPeakCutoffMaxMS1(0.0),
	m_dPeakCutoffPercentageMaxMS1(0),
	m_fDeisotope(false),
	m_dGroupPrecursorMassTolerance(0.0),
	m_iGroupMaxCyclesBetween(10),
	m_iGroupMinCycles(1),
	m_fGuessCharge(false),
	m_iFilterMinPeakCount(10),
	m_iVerbose(0),
	m_fmzMLMode(false),
	m_fmzXMLMode(false),
	m_fCoordinate(false),
	m_iSampleStartId(0),
	m_iSampleEndId(0),
	m_fOnlyCompatible(false),
	m_fShowWiffTree(false)
{
}

void
ProgramOptions::printArgs(void) {

	cout << "Information settings:" << endl;
	cout << "  instrument model: " << m_strModel.c_str () << endl;
	cout << "  ionisation used: " << m_strIonisation.c_str () << endl;
	cout << "  mass spectrometry type: " << m_strMSType.c_str () << endl;
	cout << "  detector used: " << m_strDetector.c_str () << endl;
	cout << "  use information recorded in wiff file: " << m_fUseWiffFileInformation << endl;
	cout << endl;

	cout << "General options:" << endl;
	cout << "  compression: " << (m_fCompression ? "Yes" : "No") << endl;
	cout << "  gzip output: " << (m_fGzipOutput ? "Yes" : "No") << endl;
	cout << "  verbose: " << ((m_iVerbose>0) ? "Yes" : "No");
	if (m_iVerbose>0)
		cout << " (Level " << m_iVerbose << ")";
	cout << endl;
	cout << "  mzXML output: " << (m_fmzXMLMode ? "Yes" : "No") << endl;
	cout << "  report native scan coordinate: " << (m_fCoordinate ? "Yes" : "No") << endl;
	cout << "  mzML output: "<< (m_fmzMLMode ? "Yes" : "No") << endl;
	cout << endl;

	cout << "Processing Operation options:" << endl;
	cout << "  determine precursor charge: " << (m_fGuessCharge ? "Yes" : "No") << endl;
	cout << "  min peak intensity (MS/MS): " << m_dPeakCutoffMin << endl;
	cout << "  min % of max peak intensity (MS/MS): " << m_dPeakCutoffPercentageMin << endl;
	cout << "  max peak intensity (MS/MS): " << m_dPeakCutoffMax << endl;
	cout << "  max % of max peak intensity (MS/MS): " << m_dPeakCutoffPercentageMax << endl;
	cout << "  min peak intensity (MS1): " << m_dPeakCutoffMinMS1 << endl;
	cout << "  min % of max peak intensity (MS1): " << m_dPeakCutoffPercentageMinMS1 << endl;
	cout << "  max peak intensity (MS1): " << m_dPeakCutoffMaxMS1 << endl;
	cout << "  max % of max peak intensity (MS1): " << m_dPeakCutoffPercentageMaxMS1 << endl;
	cout << "  centroid MS/MS scans: " << (m_fCentroid ? "Yes" : "No") << endl;
	cout << "  deisotope MS/MS scans: " << (m_fDeisotope ? "Yes" : "No") << endl;
	cout << "  centroid MS scans: " << (m_fCentroidMS1 ? "Yes" : "No") << endl;
	cout << endl;

	cout << "MS/MS Averaging options:" << endl;
	cout << "  precursor mass tolerance: " << m_dGroupPrecursorMassTolerance << endl;
	cout << "  max cycles span allowed: " << m_iGroupMaxCyclesBetween << endl;
	cout << "  min cycles per group: " << m_iGroupMinCycles << endl;
	cout << endl;

	cout << "MS/MS Filtering options:" << endl;
	cout << "  min peak count: " << m_iFilterMinPeakCount << endl;
	cout << endl;

	cout << "input filename: " << m_inputFileName.c_str () << endl;
	cout << "output filename: " << m_outputFileName.c_str () << endl;
	cout << endl;
}

bool 
ProgramOptions::parseArgs(int argc, char* argv[]) {
	int curArg = 1;
	bool fInvalidOption=false;
	for( curArg = 1 ; curArg < argc ; ++curArg ) {
		if (*argv[curArg] != '-')
			break;
		if (!setOption(argv[curArg])) {
			cerr << "Error: Invalid option: '" << argv[curArg] << "'" << endl;
			fInvalidOption = true;
			break;
		}
	}

	int nArgsLeft = argc - curArg;
	if (fInvalidOption) {
		return false;
	} else if (nArgsLeft < 1 || nArgsLeft > 2) {
		cerr << "Error: Please specify a wiff file to translate to mzXML" << endl;
		return false;
	} else if (m_fmzMLMode && m_fmzXMLMode) {
		cerr << "Error: Please specify one of mzXML or mzML output format" << endl;
		return false;
	/*
	} else if (!(m_fmzMLMode || m_fmzXMLMode)) {
		cerr << "Error: Please specify one of mzXML or mzML output format" << endl;
		return false;
	*/
	} else {
		if (2==nArgsLeft) {
			m_inputFileName = argv[argc-2];
			m_outputFileName = argv[argc-1];
		} else if (1==nArgsLeft) {
			m_inputFileName = argv[argc-1];
			string::size_type dotPos = m_inputFileName.find_last_of('.');
			if (dotPos == string::npos) {
				cerr << "Error: input file name did not have extension" << endl;
				return false;
			}
			else {
				m_outputFileName = m_inputFileName.substr(0, dotPos);
				if (m_fmzMLMode) {
					m_outputFileName += ".mzML";
				}
				else if (m_fmzXMLMode) {
					m_outputFileName += ".mzXML";
				}
			}
		} else {
			return false;
		}
	}

	if (m_fGzipOutput) {
		string::size_type dotPos = m_outputFileName.find_last_of('.');
		if ((dotPos==string::npos) || strcasecmp(m_outputFileName.c_str()+dotPos,".gz")) {
			m_outputFileName += ".gz";  // this will trigger gzip compression on output
		}
	}
	return true;
}

bool 
ProgramOptions::setOption(const char *arg) {
	int    iTmp = 0;
	double dTmp = 0.0;

	switch (arg[1]) {
		case 'c' :
			// -c
			if (0 == arg[2]) {
				m_fCentroid = true;
			} else if ('1' == arg[2] && 0 == arg[3]) {
				m_fCentroidMS1 = true;
			} else {
				return false;
			}
			break;

		case 'd' :
			// -d
			if (0 == arg[2]) {
				m_fDeisotope = true;
			} else {
				return false;
			}
			break;

		case 'D' :
			// -D"<str>"
			m_strDetector = &arg[2];
			break;

		case 'F' :
			// -FPC<num>
			if ('P' == arg[2]) {
				if ('C' == arg[3]) {
					if (sscanf(&arg[4], "%d", &iTmp) != 1 || iTmp < 0)
						printf("Bad filter min peak count: '%s' ... ignored\n", &arg[4]);
					else
						m_iFilterMinPeakCount = iTmp;
				} else {
					return false;
				}
			} else {
				return false;
			}
			break;

		case 'G' :
			// -G, -GC, -GPM<num>, -GMA<num>, -GMI<num>
			if (0 == arg[2]) {
				m_fUseWiffFileInformation = true;
			} else if ('C' == arg[2] && 0 == arg[3]) {
				m_fGuessCharge = true;
			} else if ('P' == arg[2] && 'M' == arg[3]) {
				if (sscanf(&arg[4], "%lf", &dTmp) != 1 || dTmp < 0)
					printf("Bad group precursor mass tolerance: '%s' ... ignored\n", &arg[4]);
				else
					m_dGroupPrecursorMassTolerance = dTmp;
			} else if ('M' == arg[2]) {
				if ('I' == arg[3]) {
					if (sscanf(&arg[4], "%d", &iTmp) != 1 || iTmp < 0)
						printf("Bad min cycle per group: '%s' ... ignored\n", &arg[4]);
					else
						m_iGroupMinCycles = iTmp;
				} else if ('A' == arg[3]) {
					if (sscanf(&arg[4], "%d", &iTmp) != 1 || iTmp > 60000)
						printf("Bad max cycle span in group: '%s' ... ignored\n", &arg[4]);
					else
						m_iGroupMaxCyclesBetween = iTmp;
				} else {
					return false;
				}
			} else {
				return false;
			}
			break;

		case 'I' :
			// -I"<str>"
			m_strIonisation = &arg[2];
			break;

		case 'O' :
			// -O"<str>"
			m_strModel = &arg[2];
			break;

		case 'P' :
			// -PI<num>, -PP<num>
			// -PI<num>-<num>, -PP<num>-<num>
			// -P1I<num>-<num>, -P1P<num>-<num>
			if ('I' == arg[2]) {
				if (!parseRangeDouble (&arg[3], "MS/MS peak cutoff intensity", 
					m_dPeakCutoffMin, m_dPeakCutoffMax, false)) {
						return false;
				}
			} else if ('P' == arg[2]) {
				if (!parseRangeDouble (&arg[3], "MS/MS peak cutoff intensity percentage", 
					m_dPeakCutoffPercentageMin, m_dPeakCutoffPercentageMax, false)) {
						return false;
				}
			} else if ('1' == arg[2]) {
				if ('I' == arg[3]) {
					if (!parseRangeDouble (&arg[4], "MS1 peak cutoff intensity", 
						m_dPeakCutoffMinMS1, m_dPeakCutoffMaxMS1, false)) {
							return false;
					}
				} else if ('P' == arg[3]) {
					if (!parseRangeDouble (&arg[4], "MS1 peak cutoff intensity percentage", 
						m_dPeakCutoffPercentageMinMS1, m_dPeakCutoffPercentageMaxMS1, false)) {
							return false;
					}
				} else {
					return false;
				}
			} else {
				return false;
			}
			break;

		case 's' :
			// -s<num>-<num>
			if (!parseRangeInt (&arg[2], 
				"sample start id", "sample end id", m_iSampleStartId, m_iSampleEndId, true)) {
				return false;
			}
			break;

		case 'T' :
			// -T"<str>"
			m_strMSType = &arg[2];
			break;

		case 'v' :
			// -v
			if (0 == arg[2]) {
				m_iVerbose++;
			} else {
				return false;
			}
			break;

		case 'z' :
			// -z
			if (0 == arg[2]) {
				m_fCompression = true;
			} else {
				return false;
			}
			break;
		case 'g' :
			// -g
			if (0 == arg[2]) {
				m_fGzipOutput = true;
			} else {
				return false;
			}
			break;
		case '-' :
			//--mzXML, --mzML, --compress, --verbose, --centroid, --deisotope
			//--only-compatible --gzip
			if (strcmp(&arg[1], "-mzXML") == 0) {
				m_fmzXMLMode = true;
			} else if (strcmp(&arg[1], "-mzML") == 0) {
				m_fmzMLMode = true;
			} else if (strcmp(&arg[1], "-compress") == 0) {
				m_fCompression = true;
			} else if (strcmp(&arg[1], "-gzip") == 0) {
				m_fGzipOutput = true;
			} else if (strcmp(&arg[1], "-verbose") == 0) {
				m_iVerbose++;
			} else if (strcmp(&arg[1], "-centroid") == 0) {
				m_fCentroid = true;
			} else if (strcmp(&arg[1], "-deisotope") == 0) {
				m_fDeisotope = true;
			} else if (strcmp(&arg[1], "-coordinate") == 0) {
				m_fCoordinate = true;
			} else if (strcmp(&arg[1], "-only-compatible") == 0) {
				m_fOnlyCompatible = true;
			} else if (strcmp(&arg[1], "-AnalystQS") == 0) {
				m_fAnalystLibQS = true;
				if (m_fAnalystLib) {
					printf("Specify --AnalystQS or --Analyst, NOT both\n");
					return false;
				}
			} else if (strcmp(&arg[1], "-Analyst") == 0) {
				m_fAnalystLib = true;
				if (m_fAnalystLibQS) {
					printf("Specify --AnalystQS or --Analyst, NOT both\n");
					return false;
				}
			} else if (strcmp(&arg[1], "-showwifftree") == 0) {
				m_fShowWiffTree = true;
			} else {
				return false;
			}
			break;

		default:
			return false;
	}

	return true;
}
