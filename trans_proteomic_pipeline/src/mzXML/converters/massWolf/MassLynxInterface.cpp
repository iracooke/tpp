// -*- mode: c++ -*-


/*
    File: MassLynxInterface.cpp
    Description: Encapsulation for Waters MassLynx interface.
    Date: July 25, 2007

    Copyright (C) 2007 Joshua Tasman, ISB Seattle


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



#include <iostream>
#include <algorithm> // for sort
#include <io.h> // for _findfirst
//#include "stdafx.h"
#include "MassLynxInterface.h"
#include "mzXML/common/Scan.h"
#include "mzXML/common/ScanCentroider.h"
#include "mzXML/common/MSUtilities.h"
#include <boost/date_time/gregorian/gregorian.hpp>

// This class interfaces with the MassLynx/DACserver.dll
// (MassLynx being whereever you installed MassLynx, probably c:/masslynx).
// Documentation for the dll is in MassLynx/InterfacingHelp.hlp

// BE SURE THAT THE FOLLOWING PATH REFERS TO THE ACTUAL
// LOCATION OF DACServer.dll ON YOUR SYSTEM
// TODO: add preprocessor flag switch
//#import "C:\MassLynx\DACServer.dll" no_namespace named_guids
#include "dacserver_4-1.h"


using namespace std;


// function to sort MassLynxScanHeader by retention Time
struct RetentionSorter{  
	bool operator()(const MassLynxScanHeader& a, const MassLynxScanHeader& b) {
		return a.retentionTimeInSec < b.retentionTimeInSec;
	}
};


typedef std::pair<float,float> floatfloatpair;
typedef std::vector<floatfloatpair> floatfloatpairVec;




MassLynxInterface::MassLynxInterface(void)
{
	verbose_ = false;
	// InstrumentInterface members
	totalNumScans_ = -1;
	doCompression_ = false;
	doCentroid_ = false;
	doDeisotope_=false;
	lockspray_ = false;
	shotgunFragmentation_ = false;
	accurateMasses_ = 0;
	inaccurateMasses_ = 0;
	// store counts for up to +15
	chargeCounts_.clear();
	chargeCounts_.resize(16, 0);
	instrumentInfo_.manufacturer_ = WATERS;
	instrumentInfo_.acquisitionSoftware_ = MASSLYNX;


	// TODO: since DAC has no function to retrieve version,
	// add user definable switch
	instrumentInfo_.acquisitionSoftwareVersion_ = "4.1";

	startTimeInSec_ = -1;
	endTimeInSec_ = -1;


	// MassLynxInterface members
	firstScanNumber_ = -1;
	lastScanNumber_ = -1;	
	firstTime_ = true;
	curScanNum_ = -1;

	totalNumFunctions_ = -1;
	scanHeaderVec_.clear();

	// thresholding
	threshold_ = false;
	inclusiveCutoff_ = -1;
	thresholdDiscard_ = false;
}

MassLynxInterface::~MassLynxInterface(void)
{
}


bool MassLynxInterface::initInterface(void) {
	// Initializes the COM library on the current thread 
	// and identifies the concurrency model as single-thread 
	// apartment (STA)
	CoInitialize(NULL);
	return true;
}

void MassLynxInterface::printInfo() {
	// Print experiment info
	IDACExperimentInfoPtr pExperimentInfo;
	try {
		pExperimentInfo = IDACExperimentInfoPtr(CLSID_DACExperimentInfo);
	}
	catch (_com_error err) {
		cerr << "Problem with the MassLynx DACServer.dll - is it installed?" << endl;
		cerr << err.Description() << endl;
		exit(-1);
	}
	pExperimentInfo->GetExperimentInfo(inputFileName_.c_str());
	BSTR bstrExpText = NULL;
	pExperimentInfo->get_ExperimentText(&bstrExpText);
	string expText = convertBstrToString(bstrExpText);
	SysFreeString(bstrExpText);

	cout << "Experiment text: ";
	cout << expText << endl;


	// Print calibration info
	IDACCalibrationInfoPtr pCalibrationInfo;
	try {
		pCalibrationInfo = IDACCalibrationInfoPtr(CLSID_DACCalibrationInfo);
	}
	catch (_com_error err) {
		cerr << "Problem with the MassLynx DACServer.dll - is it installed?" << endl;
		cerr << err.Description() << endl;
		exit(-1);
	}
	pCalibrationInfo->GetCalibration(inputFileName_.c_str());
	VARIANT pfCalFunctions;
	pCalibrationInfo->get_CalFunctions(&pfCalFunctions);

	BSTR HUGEP *calFunctionsPtr;
	// lock safe arrays for access
	HRESULT hr;
	// TODO: check hr return value?
	hr = SafeArrayAccessData(
		pfCalFunctions.parray,
		(void HUGEP**)&calFunctionsPtr);
	
	long numCalFunctions = pCalibrationInfo->GetNumCalFunctions();
	for (long c=0; c<numCalFunctions; c++) {
		cout << "Calibration " << c << ": ";
		cout << convertBstrToString(calFunctionsPtr[c]) << endl;
	}

	// clean up
	hr = SafeArrayUnaccessData(pfCalFunctions.parray);
	hr = SafeArrayDestroyData(pfCalFunctions.parray);

	// Determine number of processes per function
	// TODO: include data from processes into XML output
	IDACProcessInfoPtr pProcessInfo;
	try {
		pProcessInfo = IDACProcessInfoPtr(CLSID_DACProcessInfo);
	}
	catch (_com_error err)
	{
		cerr << "Problem with the MassLynx DACServer.DLL - is it installed?" << endl;
		cerr << err.Description() << endl;
		exit(-1);
	}
	long numProcesses;
	for(int n=0; n < totalNumFunctions_; n++) {
		pProcessInfo->GetProcessInfo(inputFileName_.c_str(), n+1);
		pProcessInfo->get_NumProcesses(&numProcesses);
		if (verbose_) {
			cout << "Function " << n+1 << " has " << numProcesses << " processes." << endl;
		}
		if (numProcesses > 0) {
			VARIANT pfDescs;
			pProcessInfo->get_ProcessDescs(&pfDescs);

			char HUGEP **descArrayPtr;
			// lock safe arrays for access
			HRESULT hr;
			hr = SafeArrayAccessData(
				pfDescs.parray,
				(void HUGEP**)&descArrayPtr);

			if (FAILED(hr)) {
					cerr << "not able to read process descriptions" << endl;
			} else {
				for (int c=0; c<numProcesses; c++) {
					string desc(descArrayPtr[c]);
					cout << "Process description nr " << c+1 << ": '" << endl;
					// The descriptions are in a weird datatype
					// After each character there's a '\0' and the string terminates with '\0\0'
					for (int i=0; i<100000; i++) {
						if (descArrayPtr[c][i]=='\0') {
							if (descArrayPtr[c][i+1]!='\0') {
								continue;
							} else {
								break;
							}
						}
						cout << descArrayPtr[c][i];
					}
					cout << "' " << endl;
				}

				// clean up
				hr = SafeArrayUnaccessData(pfDescs.parray);
				hr = SafeArrayDestroyData(pfDescs.parray);
			}
		}
	}
}

bool MassLynxInterface::setInputFile(const string& inputFileName) {
	inputFileName_ = inputFileName;

	// Determine number of accquired functions
	bool fileExists;
	int curFunctionNumber = 1;
	FILE* fp;
	char funcName[500];
	// TODO: use fstat instead?
	fileExists = true;
	totalNumFunctions_ = 0;
	while (fileExists) {
		sprintf(funcName, 
			"%s\\_func%0.3d.dat",
			inputFileName_.c_str(),
			curFunctionNumber);
		fp = fopen(funcName, "r");
		if (fp!=NULL) 
		{
			fileExists = true;
			totalNumFunctions_++;
			curFunctionNumber++;
			fclose(fp);
		} 
		else 
		{
			fileExists = false;
		}
	}
	if (totalNumFunctions_ == 0) {
		cerr << "No functions found in " << inputFileName_ 
			<< "; check filename" << endl;
		exit(-1);
	}

	if (verbose_) {
		printInfo();
	}
	bool processing = processFile();

	if (verbose_ && processing) {
		cout << "Initialization complete." << endl;
	}
	return processing;
}

bool MassLynxInterface::processFile() {
	// Determine number of scans in each function
	IDACFunctionInfoPtr pFunctionInfo;
	pFunctionInfo = IDACFunctionInfoPtr(CLSID_DACFunctionInfo);
	IDACScanStatsPtr pScanStats;
	pScanStats = IDACScanStatsPtr(CLSID_DACScanStats);

	long numScan, totNumScan;
	totNumScan = 0;
	int q = 0;
	MassLynxScanHeader tempScanHeader;
	for (int curFunction=0; curFunction<totalNumFunctions_; curFunction++) {
		if (verbose_) {
			cout 
				<< "Preprocessing function " << curFunction+1 
				<< endl;
		}

		pFunctionInfo->GetFunctionInfo(inputFileName_.c_str(), curFunction+1);

		// determine function type and corresponding MS level
		BSTR bstrFuncType = NULL;
		pFunctionInfo->get_FunctionType(&bstrFuncType);
		string funcType = convertBstrToString(bstrFuncType);
		SysFreeString(bstrFuncType);

		// TODO: figure out a better way. At least complete the list of other possible funcTypes
		int curMSLevel = 0;
		bool ignoreFunction = false;

		// TODO: fix mapping of scan types to OBO types
		if (funcType.find("MSMSMS") != string::npos) {
			curMSLevel = 3;
			functionTypes_.push_back(FULL);
		} else if (funcType.find("MSMS") != string::npos) {
			curMSLevel = 2;
			functionTypes_.push_back(FULL);
		} else if (funcType.find("Daughter") != string::npos) {
				curMSLevel = 2;
				functionTypes_.push_back(FULL);
		}
		else if (funcType.find("MS")!= string::npos) {
				curMSLevel  = 1;
				if (lockspray_ && curFunction == totalNumFunctions_ - 1) {
					functionTypes_.push_back(CALIBRATION_SCAN);
				} else {
					functionTypes_.push_back(FULL);
				}
		}
		else if (funcType.find("Scan") != string::npos) {
				curMSLevel  = 1;
				functionTypes_.push_back(FULL);
		}
		else if (funcType.find("Survey") != string::npos) {
				curMSLevel  = 1;
				functionTypes_.push_back(FULL);
		}
		else if (funcType.find("Maldi TOF") != string::npos) {
				curMSLevel  = 1;
				functionTypes_.push_back(FULL);
		}
		else {
			cerr 
				<< "!!Warning!!: ignoring function "
				<< (curFunction+1)
				<< " with type " << funcType
				<< endl;
			functionTypes_.push_back(SCAN_UNDEF);
			ignoreFunction = true;
		}

		if (shotgunFragmentation_) {
			switch (curFunction) {
				case 0:
					if (funcType != "MS") {
						cout << "First function is not of type MS, is this MS^E data? Continuing anyway." << endl;
					}
					curMSLevel = 1;
					break;
				case 1:
					if (funcType != "MS") {
						cout << "Second function is not of type MS, is this MS^E data? Continuing anyway." << endl;
					}
					curMSLevel = 2;
					break;
				case 2:
					if (!lockspray_) {
						cout << "You turned off lockspray, but a third function of type "
							<< funcType << " exists, it will be ignored."
							<< endl;
						ignoreFunction = true;
						break;
					}
					curMSLevel = 1;
					functionTypes_.pop_back();
					functionTypes_.push_back(CALIBRATION_SCAN);
					break;
				default:
					cout << "More than three functions in MS^E data, I don't know how to handle this." << endl;
					cout << "Ignoring the function: no. " << curFunction+1
						<< " with type " << funcType << "."
						<< endl;
					ignoreFunction = true;
					break;
			}
		}

		if (!ignoreFunction) {
			if (verbose_) {
				cout 
					<< "  function " 
					<< curFunction+1
					<< " with type " << funcType
					<< " set to ms level " << curMSLevel
					<< endl;
			}

			pFunctionInfo->get_NumScans(&numScan);
			if (verbose_) {
				cout 
					<< "  function " 
					<< curFunction+1 
					<< " has "
					<< numScan 
					<< " scans."
					<< endl;
			}
			totNumScan += numScan;

			for (int p=0; p<numScan; p++) {
				if (verbose_) {
					if ( (p % 100) == 0 ) {
						cout << p << " / " << numScan;
					}
					else if ( (p%10) == 0) {
						cout << "..";
					}
				}



				pScanStats->GetScanStats(inputFileName_.c_str(), curFunction+1, 0, p+1);
				tempScanHeader.funcNum = curFunction+1;
				tempScanHeader.scanNum = p+1;

				tempScanHeader.msLevel = curMSLevel;
				pScanStats->get_PeaksInScan(&tempScanHeader.numPeaksInScan);
				pScanStats->get_RetnTime(&tempScanHeader.retentionTimeInSec);
				// convert RT to seconds (from minutes)
				tempScanHeader.retentionTimeInSec = (float)(tempScanHeader.retentionTimeInSec * 60.0);
				pScanStats->get_LoMass(&tempScanHeader.lowMass);
				pScanStats->get_HiMass(&tempScanHeader.highMass);
				pScanStats->get_TIC(&tempScanHeader.TIC);
				pScanStats->get_BPM(&tempScanHeader.basePeakMass);
				pScanStats->get_BPI(&tempScanHeader.basePeakIntensity);

				//From original xml_out.cpp (note, curFunction == 1 is the second function)
				// Kludge.  Reference scans are (always?) in function 2
				//(and above?)
				//This is temporary until ExScanStats is repaired by
				//Waters
				//tempScanHeader.isCalibrated = (curFunction>0);

				q++;
				scanHeaderVec_.push_back(tempScanHeader);
			}
			if (verbose_) {
				cout << numScan << " scans preprocessed" << endl;
			}
		}
	}
	totalNumScans_ = scanHeaderVec_.size();
	if (totNumScan != totalNumScans_) {
		cerr << "logic error, please report" << endl;
		exit(1);
	}

	if (totalNumScans_ == 0) {
		cerr << "no scans found, exiting with error" << endl;
		exit(-1);
	}

	if (verbose_) {
		cout << "Function preprocessing done." << endl
			<< "Sorting by RT" << endl;
	}

	if (scanHeaderVec_.size()) {
		// sorts the MassLynxScanHeader vector by RT
		RetentionSorter retentionSorter; 
		std::sort(scanHeaderVec_.begin(), scanHeaderVec_.end(), retentionSorter);
	}
	//for (vector<MassLynxScanHeader>::iterator i = scanHeaderVec_.begin(); i != scanHeaderVec_.end(); ++i) 
	//{
	//	cout << p << " " << i->funcNum << " " << i->RT << endl;
	//}
	if (verbose_) {
		cout << "Sorting done." << endl;
	}

	// set run header info
	startTimeInSec_ =  scanHeaderVec_.begin()->retentionTimeInSec;
	endTimeInSec_ =  scanHeaderVec_.rbegin()->retentionTimeInSec;
	firstScanNumber_ = 1;
	lastScanNumber_ = totalNumScans_;


	IDACHeaderPtr pHeader;
	pHeader = IDACHeaderPtr(CLSID_DACHeader);
	try {
		pHeader->GetHeader(inputFileName_.c_str());
	} 
	catch (_com_error err) {
		cerr << "Error getting file header: " << err.ErrorMessage() << endl;
		exit(1);
	}



	//BSTR bstrInstType=NULL;
	//bstrInstType = pHeader->GetInstrumentType();
	////instrumentInfo_.instrumentModel_ = convertBstrToString(bstrInstName);
	//SysFreeString(bstrInstType);

	//BSTR bstrPlateDesc=NULL;
	//bstrPlateDesc = pHeader->GetPlateDesc();
	////instrumentInfo_.instrumentModel_ = convertBstrToString(bstrInstName);
	//SysFreeString(bstrPlateDesc);

	//long versionMajor=-1;
	//versionMajor = pHeader->GetVersionMajor();
	////instrumentInfo_.instrumentModel_ = convertBstrToString(bstrInstName);

	//long versionMinor=-1;
	//versionMinor = pHeader->GetVersionMinor();
	////instrumentInfo_.instrumentModel_ = convertBstrToString(bstrInstName);
	//

	// TODO: set this correctly
	// set dummy instrument info:
	instrumentInfo_.instrumentModel_ = Q_TOF_MICRO;
	instrumentInfo_.instrumentName_ = "Q-TOF Micro";
	instrumentInfo_.ionSource_ = ESI;
	instrumentInfo_.analyzerList_.push_back(TOFMS);
	instrumentInfo_.detector_ = DETECTOR_UNDEF;

	BSTR bstrAcquDate=NULL;
	bstrAcquDate = pHeader->GetAcquDate();

	boost::gregorian::date dateTime(boost::gregorian::from_uk_string(convertBstrToString(bstrAcquDate)));
	string dateStr=boost::gregorian::to_iso_extended_string(dateTime);
	//CTime dateTime();
	//CTime dateTime();
	//dateTime.Parse(convertBstrToString(bstrAcquDate));
	//instrumentInfo_.instrumentModel_ = convertBstrToString(bstrInstName);
	SysFreeString(bstrAcquDate);
	
	BSTR bstrAcquTime=NULL;
	bstrAcquTime = pHeader->GetAcquTime();
	dateStr += "T";
	dateStr += convertBstrToString(bstrAcquTime);
	//instrumentInfo_.instrumentModel_ = convertBstrToString(bstrInstName);
	SysFreeString(bstrAcquTime);
	timeStamp_ = dateStr;
	
	

	// TODO: is this correct?
	BSTR bstrInstSerialNumber=NULL;
	bstrInstSerialNumber = pHeader->GetInstrument();
	instrumentInfo_.instrumentSerialNumber_ = convertBstrToString(bstrInstSerialNumber);
	SysFreeString(bstrInstSerialNumber);
	

	//BSTR bstrInstModel = NULL;
	//pHeader->get_Instrument(&bstrInstModel);
	//instrumentInfo_.instrumentModel_ = convertBstrToString(bstrInstModel);
	//SysFreeString(bstrInstModel);

	curScanNum_ = 0;

	//record filenames
	//
	// get the names of all the files in the raw directory
	struct _finddata_t fileData;
	long findHandle;

	string dirName = inputFileName_ + "\\*";
	if( (findHandle = _findfirst(dirName.c_str(), &fileData)) == -1L) {
		cerr << "Could not read input directory " << dirName << endl;
		exit(-1);
	}

	do {
		if ((!strcmp(fileData.name, "." )) || (!strcmp(fileData.name, ".."))) {
			// We are not interested in "." and ".."
			continue;
		}
		string inputFile = inputFileName_ + "\\" + fileData.name;
		if (verbose_) {
			cout << "included file " << inputFile << endl;
		}
		inputFileNameList_.push_back(inputFile);
	} while (_findnext(findHandle, &fileData) == 0);
	_findclose(findHandle);


	if (verbose_) {
		cout << "Initialization complete." << endl;
	}
	return true;
}







void MassLynxInterface::setCentroiding(bool centroid) {
	doCentroid_ = centroid;
}


void MassLynxInterface::setDeisotoping(bool deisotope) {
	doDeisotope_ = deisotope;
}

void MassLynxInterface::setCompression(bool compression) {
	doCompression_ = compression;
}

void MassLynxInterface::setLockspray(bool ls) {
	lockspray_ = ls;
}

void MassLynxInterface::setShotgunFragmentation(bool sf) {
	shotgunFragmentation_ = sf;
}

// thresholding
void MassLynxInterface::setThresholdInclusiveCutoff(double inclusiveCutoff) {
	threshold_ = true;
	inclusiveCutoff_ = inclusiveCutoff;
}

// otherwise, zero-out intensities
void MassLynxInterface::setThresholdDiscard(bool discard) {
	thresholdDiscard_ = discard;
}


void MassLynxInterface::setVerbose(bool verbose) {
	verbose_ = verbose;
}


Scan* MassLynxInterface::getScan(void) {

	if (!firstTime_) {
		++curScanNum_;
		if (curScanNum_ > lastScanNumber_) {
			// we're done
			return NULL;
		}
	} 
	else {
		firstTime_ = false;
	}

	Scan* curScan = new Scan();
	curScan->isMassLynx_ = true;

	// we've already stored a lot of scan info in the header:
	// copy that over to the scan object that we're building
	MassLynxScanHeader curScanHeader = scanHeaderVec_[curScanNum_];

	curScan->msLevel_ = curScanHeader.msLevel;
	curScan->setNumDataPoints(curScanHeader.numPeaksInScan);
	curScan->retentionTimeInSec_ = curScanHeader.retentionTimeInSec;
	curScan->minObservedMZ_ = curScanHeader.lowMass;
	curScan->maxObservedMZ_ = curScanHeader.highMass;
	curScan->totalIonCurrent_ = curScanHeader.TIC;
	curScan->basePeakMZ_ = curScanHeader.basePeakMass;
	curScan->basePeakIntensity_ = curScanHeader.basePeakIntensity;
	curScan->scanType_ = functionTypes_[curScanHeader.funcNum-1];

	// TODO: determine activation type correctly
	curScan->activation_ = CID;

	// MassLynx scans only:
	curScan->isCalibrated_ = curScanHeader.isCalibrated;


	// TODO: get scan range correctly
	// hack: set scan ranges to min/max observed
	curScan->startMZ_ = curScan->minObservedMZ_;
	curScan->endMZ_ = curScan->maxObservedMZ_;

	IDACExScanStatsPtr pExScanStats;
	pExScanStats = IDACExScanStatsPtr(CLSID_DACExScanStats);
	IDACSpectrumPtr pSpectrum;

	//double nScans = scanHeaderVec.size();

	// go for precursor info
	if (curScan->msLevel_ > 1) {
		try {
			pExScanStats->GetExScanStats(
				inputFileName_.c_str(),
				curScanHeader.funcNum,
				0, // process-- why always fixed to 0?
				curScanHeader.scanNum);
		}
		catch (_com_error err )
		{
			cerr << "Error getting scan stats: " << err.ErrorMessage() << endl;
			exit(-1);
		}
		float CE = -1;
		float precursorMZ = -1;
		pExScanStats->get_CollisionEnergy(&CE);
		pExScanStats->get_SetMass(&precursorMZ);
		curScan->collisionEnergy_ = CE;
		curScan->precursorMZ_ = precursorMZ;
		// TODO: set precursor to accurate mass?

		// TODO: get precursor scan number
		//// try to get precursor scan number
		//unsigned char referenceScan = 0;
		//referenceScan = pExScanStats->GetReferenceScan();
		//cout << referenceScan << endl;
	}

	// Read the m/z intensity pairs
	pSpectrum = IDACSpectrumPtr(CLSID_DACSpectrum);
	pSpectrum->GetSpectrum(
		inputFileName_.c_str(),
		curScanHeader.funcNum, 
		0, 
		curScanHeader.scanNum);
	VARIANT pfIntensities;
	VARIANT pfMasses;
	pSpectrum->get_Intensities(&pfIntensities);
	pSpectrum->get_Masses(&pfMasses);


	float HUGEP *intensityArrayPtr;
	float HUGEP *massArrayPtr;
	// lock safe arrays for access
	HRESULT hr;
	// TODO: check hr return value?
	hr = SafeArrayAccessData(
		pfIntensities.parray,
		(void HUGEP**)&intensityArrayPtr);
	hr = SafeArrayAccessData(
		pfMasses.parray,
		(void HUGEP**)&massArrayPtr);


	long numDataPoints = curScanHeader.numPeaksInScan;
	for (long c=0; c<numDataPoints; c++) {
		curScan->mzArray_[c] = massArrayPtr[c];
		curScan->intensityArray_[c] = intensityArrayPtr[c];
	}



	// what to do if thresholding AND centroiding?
	if (threshold_) {
		curScan->threshold(inclusiveCutoff_, thresholdDiscard_);
	}

	// centroiding
	if (doCentroid_) {
		curScan->centroid("unset"); // we don't have very good instrument info here
	}

	// clean up
	hr = SafeArrayUnaccessData(pfIntensities.parray);
	hr = SafeArrayDestroyData(pfIntensities.parray);
	hr = SafeArrayUnaccessData(pfMasses.parray);
	hr = SafeArrayDestroyData(pfMasses.parray);

	return curScan;
}
