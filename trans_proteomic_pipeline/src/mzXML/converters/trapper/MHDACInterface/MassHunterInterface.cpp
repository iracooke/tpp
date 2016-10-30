// -*- mode: c++ -*-


/*
File: MassHunterInterface.cpp
Description: Encapsulation for Agilent MassHunter interface.
Date: July 25, 2007

Copyright (C) 2007 Natalie Tasman, ISB Seattle
Based on code from Agilent sample "DataReaderCPlusClient" project.

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
#include "stdafx.h"
#include "mzXML/common/Scan.h"
#include "mzXML/common/MSUtilities.h"

#include "MassHunterInterface.h"


using namespace std;

#define COMCHECK_NOTHROW(comcall, errMsg, action) \
{ \
	HRESULT hr = comcall; \
	if (hr != S_OK) { \
	cerr << "ERROR at " << __FILE__ << ", " << __LINE__ << ":" << endl; \
	cerr << "HR = " << hr << endl; \
	cerr << errMsg << endl; \
	action; \
	} \
}

MassHunterInterface::MassHunterInterface(MHDACWrapper* mhdacWrapper)
{
	MHDACWrapper_ = mhdacWrapper;
	verbose_ = false;
	// InstrumentInterface members
	totalNumScans_ = -1;
	doCompression_ = false;
	doCentroid_ = false;
	doDeisotope_= false;

	// deisotoping paramaters, available in MHDAC 1.2.1
	requirePeptideLikeAbundanceProfile_ = false;
	relativeTolerance_ = 0;
	absoluteTolerance_ = 0 ;
	limitChargeState_ = false;
	maxChargeState_ =0 ;

	accurateMasses_ = 0;
	inaccurateMasses_ = 0;
	// store counts for up to +15
	chargeCounts_.clear();
	chargeCounts_.resize(16, 0);
	// try to refine this later based on actual instrument name,
	// once obtained.
	instrumentInfo_.manufacturer_ = AGILENT;
	instrumentInfo_.acquisitionSoftware_ = MASSHUNTER;


	// TODO: determine MassHunter version programatically
	instrumentInfo_.acquisitionSoftwareVersion_ = "unknown";

	startTimeInSec_ = -1;
	endTimeInSec_ = -1;


	// MassHunterInterface members
	totalNumScans_ = -1;
	firstScanNumber_ = -1;
	lastScanNumber_ = -1;	
	firstTime_ = true;
	curScanNum_ = -1;

	bool requirePeptideLikeAbundanceProfile_ = false;
	relativeTolerance_ = -1;
	absoluteTolerance_ = -1;
	limitChargeState_ = false;
	int maxChargeState_ = -1;


	pSafeArrayRetentionTimes_ = NULL;
	retentionTimeArray_ = NULL;
	totalIonCurrentArray_ = NULL;
	basePeakIntensityArray_ = NULL;

	pMSDataReader_ = NULL;
	pChromData_ = NULL;
	pChromDataBasePeaks_ = NULL;
}

MassHunterInterface::~MassHunterInterface(void)
{
	COMCHECK_NOTHROW(SafeArrayUnaccessData(pSafeArrayRetentionTimes_),
		"unable to deallocate memory", );
	COMCHECK_NOTHROW(SafeArrayUnaccessData(pSafeArrayTotalIonCurrents_),
		"unable to deallocate memory", );
	COMCHECK_NOTHROW(SafeArrayUnaccessData(pSafeArrayBasePeakIntensities_),
		"unable to deallocate memory", );
	COMCHECK_NOTHROW(SafeArrayDestroy(pSafeArrayRetentionTimes_),
		"unable to deallocate memory", );
	COMCHECK_NOTHROW(SafeArrayDestroy(pSafeArrayTotalIonCurrents_),
		"unable to deallocate memory", );
	COMCHECK_NOTHROW(SafeArrayDestroy(pSafeArrayBasePeakIntensities_),
		"unable to deallocate memory", );

	// TODO: why not pSafeArrayChromData_ also?
	// NEW WAY: SafeArrayUnaccessData(pSafeArrayChromData_);
	pChromData_->Release();
	pChromDataBasePeaks_->Release();
	pMSDataReader_->Release();
	// make sure any com smart ptr objects have descoped before now!
	// uninitialize COM
	CoUninitialize();
}





bool MassHunterInterface::initInterface(void) {
	MHDACWrapper_->InitCOM();
	return true; // error thrown, otherwise	
//	// initialize MFC and print and error on failure
//	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0)) {
//		cerr << "Fatal Error: MFC initialization failed" << endl;
//		return false;
//	}
//
//
//	// Initializes the COM library on the current thread 
//	// and identifies the concurrency model as single-thread 
//	// apartment (STA)
//
//	CoInitialize(NULL);
//	return true;
}





bool MassHunterInterface::setInputFile(const string& inputFileName) {
	inputFileName_ = inputFileName;

	HRESULT hr = S_OK;
	USES_CONVERSION;

	try {
		//IMsdrDataReaderPtr pMSDataReader;


		pMSDataReader_ = MHDACWrapper_->CreateMsdrDataReader();

		VARIANT_BOOL pRetVal = MHDACWrapper_->OpenDataFile(pMSDataReader_, inputFileName);
		// also check return value
		if (pRetVal == VARIANT_FALSE) {
			cerr << "Failed to open data folder" << endl;
			return false;
		}	
		//VariantClear(pRetVal);

		// get acquisition timestamp

		// get scan file information
		MH::BDA::IBDAFileInformationPtr pFileInfo = MHDACWrapper_->GetFileInformation(pMSDataReader_);

		DATE date = MHDACWrapper_->GetAcquisitionTime(pFileInfo);
		COleDateTime dateTime(date);
		timeStamp_ = dateTime.Format("%Y-%m-%dT%H:%M:%S");



		// get MHDAC version
		BSTR tempVerStr = MHDACWrapper_->GetVersion(pMSDataReader_);
		string MHDAC_version = convertBstrToString(tempVerStr);
		
		// TODO: create new place to store this:
		instrumentInfo_.acquisitionSoftwareVersion_ = MHDAC_version;

		// Get MS scan file information
		MH::BDA::IBDAMSScanFileInformationPtr pScanInfo = MHDACWrapper_->GetMSScanFileInformation(pMSDataReader_);

		// get total number of scans
		__int64 totalNumScans = MHDACWrapper_->GetTotalScansPresent(pScanInfo);
		totalNumScans_ = (long)totalNumScans;

		if (verbose_) {
			cout << "total number of expected scans: " << totalNumScans_ << endl;
		}

		
		// dual-mode detection
		VARIANT_BOOL bDualMode = MHDACWrapper_->CheckDualMode(pScanInfo);
		if (bDualMode == VARIANT_FALSE) {
			cout << "single mode file" << endl;
		} else {
			cout << "dual-mode file" << endl;
		}
		//VariantClear(bDualMode);

		// specifically record and detect if peak and/or profile data is present
		MH::MSStorageMode storageMode = MHDACWrapper_->GetSpectraFormat(pScanInfo);

		// TODO: record this info in mzXML header;
		// make dataprocessing section cleaner (centroided is not necessarily considered data processed by software.)
		switch (storageMode) {
			case MH::MSStorageMode_PeakDetectedSpectrum:
				// we can only get centroid data;
				// deisotoping is Sok
				spectraMode_ = PEAKPICKED_ONLY;
				break;
			case MH::MSStorageMode_ProfileSpectrum:
				// we can get profile or centroid
				// deisotoping only ok if centroid specificed
				if (doDeisotope_ && !doCentroid_) {
					cerr << "deisotoping requires centroided data.  Please specify centroiding to handle this profile data." << endl;
					return false;
				}
				spectraMode_ = PROFILE_ONLY;
				break;
			case MH::MSStorageMode_Mixed:
				// each spectra might have either or both available
				// deisotoping only ok if centroid specificed
				if (doDeisotope_ && !doCentroid_) {
					cerr << "deisotoping requires centroided data.  Please specify centroiding to handle this mixed mode data." << endl;
					return false;
				}
				spectraMode_ = MIXED;
				break;
		
			case MH::MSStorageMode_Unspecified:
			default: 
				// TODO: error out here
				break;
		}

	


		//
		// read instrument information
		//

		// TODO: get instrument model
		//instrumentInfo_.instrumentModel_ = instType;
		//instrumentInfo_.instrumentName_ = instType;

		// Get analyzer type
		MH::DeviceType devType = MHDACWrapper_->GetDeviceType(pScanInfo);

		if (verbose_) {
			cout << "Device Type: " << devType << endl;
		}
		MSAnalyzerType analyzerType ;
		switch (devType) {
			case MH::DeviceType_QuadrupoleTimeOfFlight:
				analyzerType = QTOF;
				break;
			case MH::DeviceType_TimeOfFlight:
				analyzerType = TOFMS;
				break;
			case MH::DeviceType_TandemQuadrupole:
				analyzerType = TANDEM_QUAD;
				break;
			case MH::DeviceType_Unknown:
			default:
				analyzerType = ANALYZER_UNDEF;
				break;
		}
		if (analyzerType == ANALYZER_UNDEF) {
			cerr << "unknown instrument type: " << devType << endl;
			return false;	
		}
		instrumentInfo_.analyzerList_.push_back(analyzerType);


		// now that we have the device type, we can get the device name
		BSTR tempBstr = MHDACWrapper_->GetDeviceName(pFileInfo, devType);
		string modelName = convertBstrToString(tempBstr);
		// TODO: waiting for Agilent instrument types in order to fill in MSTypes corrects
		instrumentInfo_.instrumentModel_ = AGILENT_TOF;
		SysFreeString(tempBstr);

		// TODO: get dectector
		instrumentInfo_.detector_ = DETECTOR_UNDEF;


		// get ionization mode
		MH::IonizationMode ionMode = MHDACWrapper_->GetIonModes(pScanInfo);
	
		// TODO: check with Agilent if correct
		MSIonizationType ionization;
		switch (ionMode) {
			case MH::IonizationMode_Esi:
				ionization = ESI;
				break;
			case MH::IonizationMode_NanoEsi:
				ionization = NSI;
				break;
			case MH::IonizationMode_MsChip:
				ionization = MS_CHIP;
				break;
			case MH::IonizationMode_Unspecified:
			default:
				ionization = IONIZATION_UNDEF;
				break;
		}
		if (ionization == IONIZATION_UNDEF) {
			cerr << "unknown ion type: " << ionMode << endl;
			return false;
		}
		instrumentInfo_.ionSource_ = ionization;



		// TODO: inst serial #
		// TODO: inst hardware version
		// TODO: inst software version

		firstScanNumber_ = 1;
		lastScanNumber_ = totalNumScans_;
		curScanNum_ = 0;


		// note: if default (no filter passed in),
		// chrom (spectra containing obj) GetTIC selects
		// "peaks else profile" for
		// mixed mode data (mix of both profile, peak pick
		// time segments in the run)
		// so for finer grain control, use chromfilter and GetChromatogram

		// if user requests centroid,
		// get "peaks else profile;"
		// if user requests profile, get "profile else peaks"
		
		// detect this case with fileinformation:
		// msstoragemode mixed




		//// OLD WAY
		//hr = pMSDataReader_->GetTIC(&pChromData_);
		//if ( S_OK != hr ) {
		//	cerr << "Error getting TIC" << endl;
		//	return false;
		//}

		// new way
		MH::BDA::IBDAChromFilterPtr pChromFilter;
		pChromFilter = MHDACWrapper_->CreateChromFilter();
		//COMCHECK_NOTHROW(
		//	CoCreateInstance( MH::BDA::CLSID_BDAChromFilter, NULL, 
		//	CLSCTX_INPROC_SERVER ,	
		//	MH::BDA::IID_IBDAChromFilter, 
		//	(void**)&pChromFilter),
		//	"couldn't create chromfilter",
		//	return (false)
		//	);

		//pChromFilter->put_ChromatogramType(MH::ChromType_TotalIon);
		MHDACWrapper_->SetChromatogramType(pChromFilter, MH::ChromType_TotalIon);
		MHDACWrapper_->SetDoCycleSum(pChromFilter, VARIANT_FALSE);//do not cycle sum

		if (doCentroid_) {
			// make centroiding more effienct: get peak-picked spectra, when already in file
			MHDACWrapper_->SetDesiredChromMSStorageType(pChromFilter, MH::DesiredMSStorageType_PeakElseProfile);
		} else {
			// profile: get profile when availble, fall back to peak-peaked if that's the
			// only thing available
			MHDACWrapper_->SetDesiredChromMSStorageType(pChromFilter, MH::DesiredMSStorageType_ProfileElsePeak);
		}
		
		// extract the chromdata obj from the safearray-- I'm expecting only one
		pSafeArrayChromData_ = MHDACWrapper_->GetChromatogram(pMSDataReader_, pChromFilter);

		
		// check to make sure that exactly one TIC was returned from the call
		long numTIC = 0;
		COMCHECK_NOTHROW(SafeArrayGetUBound(pSafeArrayChromData_, 1, &numTIC),
			"SafeArrayGetUBound failed.",
			return false);

		// TODO: check ubonds result for 0 entries
		if (numTIC < 0) {
			cerr << "ERROR: no TICs found" << endl;
			return false;
		}
		else if (numTIC > 0) {
			cerr << "ERROR: more than one TIC found" << endl;
			return false;
		}

		MH::BDA::IBDAChromData **chromDataObjsArray = NULL;

		SafeArrayAccessData(pSafeArrayChromData_, reinterpret_cast<void**>(&chromDataObjsArray));
		// again, assuming only one chromatogram returned, based on chosen chromfilter settings, above
		// TODO: assert safearray only contains one obj
		
		pChromData_ = chromDataObjsArray[0];




		long dataPoints = MHDACWrapper_->GetTotalChromDataPoints(pChromData_);
		
		int numMSspectra = dataPoints;

		if (numMSspectra != totalNumScans_) {
			cerr << "unexpected difference in scan counting" << endl;
			return false;
		}

		// xArray is double[]
		pSafeArrayRetentionTimes_ = MHDACWrapper_->GetChromDataXArray(pChromData_);
		SafeArrayAccessData(pSafeArrayRetentionTimes_, reinterpret_cast<void**>(&retentionTimeArray_));
		// pSafeArrayRetentionTimes_ is unaccessed in destructor
		// set start, end time from retention times
		startTimeInSec_ = retentionTimeArray_[0] * 60;
		endTimeInSec_ = retentionTimeArray_[totalNumScans_ - 1] * 60;

		// yArray is float[]
		pSafeArrayTotalIonCurrents_ = MHDACWrapper_->GetChromDataYArray(pChromData_);
		SafeArrayAccessData(pSafeArrayTotalIonCurrents_, reinterpret_cast<void**>(&totalIonCurrentArray_));
		// pSafeArrayTotalIonCurrents_ is unaccessed in destructor
		
		// get base peak info
		//CComPtr<MH::BDA::IBDAChromData> pChromDataBasePeaks;	

		pChromDataBasePeaks_ = MHDACWrapper_->GetTIC(pMSDataReader_);

		// yArray is float[]
		pSafeArrayBasePeakIntensities_ = MHDACWrapper_->GetChromDataYArray(pChromDataBasePeaks_);


		SafeArrayAccessData(pSafeArrayBasePeakIntensities_, reinterpret_cast<void**>(&basePeakIntensityArray_));
		// pSafeArrayBasePeakIntensities_ is unaccessed in destructor



	
		//// TODO: start, end times from time range?
		//double* timeRange = NULL;
		//SAFEARRAY *pTimeRange = NULL;
		//hr = pChromData_->get_AcquiredTimeRange(&pTimeRange);
		//if ( S_OK != hr ) {
		//	cerr << "Error getting time range of TIC." << endl;
		//	return false;
		//}
		//SafeArrayAccessData(pTimeRange, reinterpret_cast<void**>(&timeRange));
		//SafeArrayUnaccessData(pTimeRange);

		//cout << endl;

	}
	// TODO: more informative catching
	catch(_com_error& e) {
		// why does this trap?
		hr = e.Error();
		if (hr != S_OK) {
			cerr << e.ErrorMessage() << endl;
			return false;
		}
	}
	catch(HRESULT& hr) {
		//_com_error e(hr);
		//hr = e.Error();
		cerr << hr << endl;
		return false;
	}
	catch(...) {
		//hr = E_UNEXPECTED;
		return false;
	}


	if (verbose_) {
		cout << "Initialization complete." << endl;
	}


	return true;
}







void MassHunterInterface::setCentroiding(bool centroid) {
	doCentroid_ = centroid;
}


void MassHunterInterface::setDeisotoping(bool deisotope) {
	doDeisotope_ = deisotope;
}

void MassHunterInterface::setCompression(bool compression) {
	doCompression_ = compression;
}

void MassHunterInterface::setVerbose(bool verbose) {
	verbose_ = verbose;
}


Scan* MassHunterInterface::getScan(void) {
	try {
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
		// TODO: massHunter flag for Scan object
		//curScan->isMassHunter_ = true;

		//double *xArray = NULL;
		//SAFEARRAY *pRTs = NULL;

		HRESULT hr = S_OK;

		// retentionTime now accessed in setInputFile
		//hr = pChromData_->get_xArray(&pRTs);
		//if ( S_OK != hr ) {
		//	cerr << "Error getting XArray of TIC." << endl;
		//	return false;
		//}
		//SafeArrayAccessData(pRTs, reinterpret_cast<void**>(&xArray));


		MH::MSLevel mslevel;

		CComPtr<MH::BDA::IBDAMSScanFileInformation> pScanInfo;
		//cout << "on scan " << curScanNum_ << endl;

		curScan->retentionTimeInSec_ = retentionTimeArray_[curScanNum_] * 60;
		curScan->totalIonCurrent_ = (double)totalIonCurrentArray_[curScanNum_];

		pScanInfo = NULL;
		hr = pMSDataReader_->GetMSScanInformation(
			retentionTimeArray_[curScanNum_],
			&pScanInfo);
		if ( NULL == pScanInfo) {
			cerr << "Error reading scan info for time: " <<  retentionTimeArray_[curScanNum_] << endl;
			return NULL;
		}

		pScanInfo->get_MSLevel(&mslevel);
		// set the scan's MS level
		switch (mslevel) {
		case MH::MSLevel_MS:
			curScan->msLevel_ = 1;
			break;
		case MH::MSLevel_MSMS:
			curScan->msLevel_ = 2;
			break;
		default:
			break;
		}
		if (curScan->msLevel_ < 1) {
			cerr << "unknown scan type: " << mslevel << endl;
			exit(-1);
		}



		CComPtr<MH::IMsdrPeakFilter> pPeakFilter;
		hr = CoCreateInstance( MH::CLSID_MsdrPeakFilter, NULL, 
			CLSCTX_INPROC_SERVER ,	
			MH::IID_IMsdrPeakFilter, 
			(void**)&pPeakFilter);

		// TODO: make these user parameters
		pPeakFilter->put_AbsoluteThreshold(0); // return everything, no cutoff
		pPeakFilter->put_RelativeThreshold(0); // return everything, no cutoff
		pPeakFilter->put_MaxNumPeaks(0); // 0 is "no limit"



		// set no filters on scan type, ionpolarity, ionmode

		VARIANT_BOOL filterOnCentroid;
		MH::IMsdrPeakFilter* filter = NULL; // set to NULL or real filter

		CComPtr<MH::BDA::IBDASpecFilter> pSpecFilter;

		// TODO: add user control of if we should refilter centroided data, with new threshold
		if (spectraMode_ == PROFILE_ONLY) {
			if (!doCentroid_) {
				filter = NULL;
				filterOnCentroid = VARIANT_FALSE; // no meaning
			}
			else {
				filter = pPeakFilter;
				filterOnCentroid = VARIANT_FALSE;
			}
		}
		else if (spectraMode_ == PEAKPICKED_ONLY) {
			// start with centroid, end with centroid no matter what
			// nothing to do, unless user wants to threshold as well (add later)
			filter = NULL;
			filterOnCentroid = VARIANT_FALSE; // no meaning

		}


		else if (spectraMode_ == MIXED) {
			// tricky-- don't know what we'll get
			if (doCentroid_) {
				// run the filter, but not on already peak-picked
				filter = pPeakFilter;
				filterOnCentroid = VARIANT_FALSE;
			} else {
				// don't want centroid, but will get it for already centroided
				
				// David Horn - create a spectrum filter to be able to specify that we
				//	want to extract profile spectra from mixed data.  This will require a 
				//	call to a different GetSpectrum function shown down below that
				//	has a specFilter as an input parameter.
	
				hr = CoCreateInstance (MH::BDA::CLSID_BDASpecFilter, NULL, 
					CLSCTX_INPROC_SERVER, 
					MH::BDA::IID_IBDASpecFilter,
					(void **)&pSpecFilter);
				hr = pSpecFilter->put_IonizationMode(MH::IonizationMode_Unspecified);
				hr = pSpecFilter->put_IonPolarityFilter(MH::IonPolarity_Mixed);
				hr = pSpecFilter->put_MSScanTypeFilter(MH::MSScanType_All);
				hr = pSpecFilter->put_DesiredMSStorageType(MH::DesiredMSStorageType_Profile);
				hr = pSpecFilter->put_MSLevelFilter(MH::MSLevel_All);
				hr = pSpecFilter->put_SpectrumType(MH::SpecType_MassSpectrum);
				MH::IMinMaxRange *minmaxrange = NULL;
				hr = CoCreateInstance ( MH::CLSID_MinMaxRange, NULL, 
					CLSCTX_INPROC_SERVER, 
					MH::IID_IMinMaxRange,
					(void **) &minmaxrange);
				
				hr = minmaxrange->put_Min(retentionTimeArray_[curScanNum_]);
				hr = minmaxrange->put_Max(retentionTimeArray_[curScanNum_]);
				SAFEARRAY *range;
				SAFEARRAYBOUND rgsabound[1];
				rgsabound[0].lLbound = 0;
				rgsabound[0].cElements = 1;
				long index[1];
				index[0] = 0;
				range = SafeArrayCreate(VT_DISPATCH, 1, rgsabound);
				if (range == NULL)
				{
					cerr << "unable to create time range" << endl;
					exit(1);
				}
				SafeArrayPutElement(range, index, (void*)minmaxrange);
				hr = pSpecFilter->put_ScanRange(range);

				minmaxrange->Release();

				//filter = NULL;
				//filterOnCentroid = VARIANT_FALSE; // no meaning
			}
		}

		CComPtr<MH::BDA::IBDASpecData> pSpecData;
		SAFEARRAY *pSpecDataArray;

		if (pSpecFilter == NULL) {
			pSpecData.Attach(MHDACWrapper_->GetSpectrum(pMSDataReader_,
				retentionTimeArray_[curScanNum_],
				MH::MSScanType_All,
				MH::IonPolarity_Mixed,
				MH::IonizationMode_Unspecified,
				filter,
				&filterOnCentroid));
		} else {
			
			pSpecDataArray = (MHDACWrapper_->GetProfileSpectrumfromMixedData(pMSDataReader_, pSpecFilter));
			MH::BDA::IBDASpecData** pBDASpecDataArray = NULL;
			long lower=0;
			hr = SafeArrayGetLBound(pSpecDataArray, 1, &lower);
			hr = SafeArrayAccessData(pSpecDataArray, reinterpret_cast<void**>(&pBDASpecDataArray));
			hr = SafeArrayUnaccessData(pSpecDataArray);
			pSpecData.Attach(pBDASpecDataArray[lower]);
			SafeArrayDestroy(pSpecDataArray);

		
		}

		long numPeaks = MHDACWrapper_->GetTotalSpectrumDataPoints(pSpecData);
		if (numPeaks == 0) {
			cerr << "scan " << curScanNum_ << " is empty scan" << endl;
			return curScan;
		}

		MHDACWrapper_->ConvertDataToMassUnits(pSpecData);

		// determine if spectra is centroid or profile mode
		// and record this info
		MH::MSStorageMode spectrumMode = MHDACWrapper_->GetMSStorageMode(pSpecData);
		// sanity check:
		if ((spectrumMode != MH::MSStorageMode_ProfileSpectrum)
			&&
			(spectrumMode != MH::MSStorageMode_PeakDetectedSpectrum)) {
				cerr << "error: spectrum was neither profile nor centroid" << endl;
				//return NULL;
		}

		// TODO: error checking logic based on spectraMode_ and doCentroid_
		switch (spectrumMode) {
		case MH::MSStorageMode_ProfileSpectrum:
			curScan->isCentroided_ = false;
			break;
		case MH::MSStorageMode_PeakDetectedSpectrum:
			curScan->isCentroided_ = true;
			break;
		default:
			// shouldn't get here
			// TODO: exit with error
			break;
		}
		//if (verbose_) {
		//	if (curScan->isCentroided_) {
		//		cout << "centroid";
		//	}
		//	else {
		//		cout << "profile";
		//	}
		//	cout << " scan." << endl;
		//}



		// deisotoping
		// 
		// deisotoping can be applied to any scan type or level, as long as it's been centroided
		if (doDeisotope_) {
			if (!curScan->isCentroided_) {
				// exit with error; should have already been caught
				cerr << "attempt to deisotope non-centroid data" << endl;
				exit(1);
			}
			CComPtr<MH::IMsdrChargeStateAssignmentFilter> pCsaFilter = MHDACWrapper_->CreateChargeStateAssignmentFilter();

			if (limitChargeState_) {
				MHDACWrapper_->SetCSAFMaxChargeStateLimit(pCsaFilter, VARIANT_TRUE);
				MHDACWrapper_->SetCSAFMaximumChargeState(pCsaFilter, maxChargeState_);
			} else {
				MHDACWrapper_->SetCSAFMaxChargeStateLimit(pCsaFilter, VARIANT_FALSE);
			}

			if (requirePeptideLikeAbundanceProfile_) {
				MHDACWrapper_->SetCSAFRequirePeptideLikeAbundanceProfile(pCsaFilter, VARIANT_TRUE);
			}

			if (absoluteTolerance_ >= 0) {
				MHDACWrapper_->SetCSAFAbsoluteTolerance(pCsaFilter, absoluteTolerance_);
			}

			if (relativeTolerance_ >= 0) {
				MHDACWrapper_->SetCSAFRelativeTolerance(pCsaFilter, relativeTolerance_);
			}

			MHDACWrapper_->Deisotope(pMSDataReader_,pSpecData, pCsaFilter);
		}


		//get scan ID
		long scanID = MHDACWrapper_->GetScanID(pSpecData);
		nativeToSequentialScanNums_[scanID] = curScanNum_;
		curScan->nativeScanRef_.setCoordinateType(AGILENT);
		curScan->nativeScanRef_.addCoordinate(MHDAC_COORDINATE_NATIVESCANNUM, toString(scanID));


		// get scan polarity
		MH::IonPolarity polarity = MHDACWrapper_->GetIonPolarity(pSpecData);
		switch(polarity) {
		case MH::IonPolarity_Negative:
			curScan->polarity_ = NEGATIVE;
			break;
		case MH::IonPolarity_Positive:
			curScan->polarity_ = POSITIVE;
			break;
		default:
			curScan->polarity_ = POLARITY_UNDEF;
			break;
		}





		// scan type
		MH::MSScanType mhScanType = MHDACWrapper_->GetMSScanType(pSpecData);
		switch (mhScanType) {
		case MH::MSScanType_HighResolutionScan:
			curScan->scanType_ = HighResolution;
			break;
		case MH::MSScanType_MultipleReaction:
			curScan->scanType_ = MultipleReaction;
			break;
		case MH::MSScanType_NeutralGain:
			curScan->scanType_ = NeutralGain;
			break;
		case MH::MSScanType_NeutralLoss:
			curScan->scanType_ = NeutralLoss;
			break;
		case MH::MSScanType_PrecursorIon:
			curScan->scanType_ = PrecursorIon;
			break;
		case MH::MSScanType_ProductIon:
			curScan->scanType_ = ProductIon;
			break;
		case MH::MSScanType_Scan:
			curScan->scanType_ = MS1SurveyScan;// what's this one?
			break;
		case MH::MSScanType_SelectedIon:
			curScan->scanType_ = SelectedIon;
			break;
		case MH::MSScanType_TotalIon:
			curScan->scanType_ = TotalIon;
			break;
		default:
			cerr << "warning, encoutered unknown scan type at scan " << curScanNum_
				<< ", not recording" << endl;
			curScan->scanType_ = SCAN_UNDEF;
			break;
		}





		// precursor info
		if (curScan->msLevel_ > 1) {
			//get precursor mass
			SAFEARRAY *psaPrecursors = NULL;
			double *dblArrayPrecursors = NULL;
			long precCount = 0;
			psaPrecursors = MHDACWrapper_->GetPrecursorIon(pSpecData,&precCount);
			if (precCount != 1) {
				cerr << "unexpected number of ms2 precursors: " << precCount << endl;
				exit(-1);
			}

			SafeArrayAccessData(psaPrecursors, reinterpret_cast<void**>(&dblArrayPrecursors));
			curScan->precursorMZ_ = dblArrayPrecursors[0];
			SafeArrayUnaccessData(psaPrecursors);

			//get precursor charge
			VARIANT_BOOL chargeSuccess = VARIANT_TRUE;
			long charge = MHDACWrapper_->GetPrecursorCharge(pSpecData, &chargeSuccess);
			if (chargeSuccess == VARIANT_TRUE ) {
				// record in our scan obj
				curScan->precursorCharge_ = charge;
				//cout << "got charge " << charge << " for scan " << curScanNum_ << endl;
			}


			//get precursor intensity
			VARIANT_BOOL intSuccess = VARIANT_TRUE;
			double intensity = MHDACWrapper_->GetPrecursorIntensity(pSpecData, &intSuccess);
			if (intSuccess == VARIANT_TRUE) {
				curScan->precursorIntensity_ = intensity;
			}

			// parent scan ID
			long parentScanID = MHDACWrapper_->GetParentScanID(pSpecData);
			// TODO: error checking on map
			map<long,long>::iterator iter = nativeToSequentialScanNums_.find(parentScanID);
			if (iter != nativeToSequentialScanNums_.end() ) {
				curScan->precursorScanNumber_ = iter->second;
			}

			// collision energy:
			curScan->collisionEnergy_ = MHDACWrapper_->GetCollisionEnergy(pSpecData);
		}
		// end precursor info



		// get actual peak information

		//long numPeaks = MHDACWrapper_->GetTotalSpectrumDataPoints(pSpecData);
		//if (numPeaks == 0) {
			//cerr << "scan " << curScanNum_ << " is empty scan" << endl;
		//}
		//else {
			SAFEARRAY *psaX = MHDACWrapper_->GetSpectrumXArray(pSpecData); // xArray is double [] !
			SAFEARRAY *psaY = MHDACWrapper_->GetSpectrumYArray(pSpecData); // yArray is float [] !

			double *mzArray = NULL; // xArray is double [] !
			float *intensityArray = NULL; // yArray is float [] !
			SafeArrayAccessData(psaX, reinterpret_cast<void**>(&mzArray));
			SafeArrayAccessData(psaY, reinterpret_cast<void**>(&intensityArray));

			curScan->setNumDataPoints(numPeaks);

			// actually copy the data!

			// save basepeak info
			// TODO: can we get basepeak info from the API?
			long basePeakIndex = -1;
			float basePeakIntensity = -1;

			//cout << "scan " << curScanNum_ 
			//	<< ", rt=" << curScan->retentionTimeInSec_
			//	<< ", ms level=" << curScan->msLevel_
			//	<< ", #peaks=" << numPeaks
			//	<< endl;
			//getchar();

			// TODO: fix memory error here
			for (long j=0; j<numPeaks; j++) {
				curScan->mzArray_[j] = mzArray[j];
				curScan->intensityArray_[j] = (double) intensityArray[j];
				if (intensityArray[j] > basePeakIntensity) {
					basePeakIntensity = intensityArray[j];
					basePeakIndex = j;
				}
				//cout << "mz: " << mzArray[j] << "\t" << "intensity: " << intensityArray[j] << endl;
			}
			SafeArrayUnaccessData(psaX);
			SafeArrayUnaccessData(psaY);
			SafeArrayDestroy(psaX);
			SafeArrayDestroy(psaY);
			//getchar();

			// store basepeak info

			// manually set basepeak info
			// TODO: can we get this from the API?
			curScan->basePeakIntensity_ = (double) basePeakIntensity;
			float chromBPI = basePeakIntensityArray_[curScanNum_];
			//// sanity check:
			//if (basePeakIntensity != basePeakIntensityArray_[curScanNum_]) {
			//	cerr << "problem with base peak intensity" << endl;
			//	exit(-1);
			//}

			curScan->basePeakMZ_ = curScan->mzArray_[basePeakIndex];

			// set min, max observed mass
			curScan->minObservedMZ_ = curScan->mzArray_[0];
			curScan->maxObservedMZ_ = curScan->mzArray_[numPeaks-1];

			// TODO: correctly extract scan range
//		}



		return curScan;
	}
	catch (...) {
		return NULL;
	}
}
