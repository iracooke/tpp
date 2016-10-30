// -*- mode: c++ -*-


/*
    File: AnalystQS20Interface.cpp
    Description: Implementation of instrument interface for data acquired 
                 with AnalystQS.  The program depends on the general framework 
                 developed by Natalie Tasman, ISB Seattle
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


#include <iostream>

#include "stdafx.h"
#include "AnalystQS20Interface.h"
#include "mzXML/common/Scan.h"
#include "mzXML/common/MSUtilities.h"

#include <float.h>	// _controlfp
#include <math.h>	// fabs

using namespace AnalystQS20;

AnalystQS20Interface::AnalystQS20Interface(void) :
	m_pInstrumentInfoLoader(NULL),
	m_pInstrumentInfoParser(NULL)
{
	m_enumLibraryType = LIBRARY_AnalystQS;
	m_InterfaceVersion = "2.0";
}

AnalystQS20Interface::~AnalystQS20Interface(void)
{
	termInfoLoader();
	termInterface();
}

bool AnalystQS20Interface::initInterface(void) {
	// Initializes the COM library on the current thread 
	// and identifies the concurrency model as single-thread 
	// apartment (STA)
	if (!m_fCOMInitialized) {
		if (S_OK==CoInitializeEx( NULL , COINIT_APARTMENTTHREADED)) {
			m_fCOMInitialized = true;
		}
	}

	//----------------------------------------

	m_paramsString=SysAllocString(L"");

	HRESULT hr;

	hr = m_ipFMANChromData.CreateInstance(__uuidof(FMANChromData));
	if (FAILED(hr)) {
		LPOLESTR lpolestrCLSID;
		StringFromCLSID(__uuidof(FMANChromData), &lpolestrCLSID);
		CW2A pszACLSID( lpolestrCLSID );
		std::cerr << "Could not create FMANChromData" << pszACLSID << " instance, hr("
			<< std::hex << hr << std::dec << ")" << std::endl;
		CoTaskMemFree(lpolestrCLSID);

		return false;
	}

	hr = m_ipFMANSpecData.CreateInstance(__uuidof(FMANSpecData));
	if (FAILED(hr)) {
		LPOLESTR lpolestrCLSID;
		StringFromCLSID(__uuidof(FMANSpecData), &lpolestrCLSID);
		CW2A pszACLSID( lpolestrCLSID );
		std::cerr << "Could not create FMANSpecData" << pszACLSID << " instance, hr("
			<< std::hex << hr << std::dec << ")" << std::endl;
		CoTaskMemFree(lpolestrCLSID);

		return false;
	}

	hr = m_ipFMANSpecDataCopy.CreateInstance(__uuidof(FMANSpecData));
	if (FAILED(hr)) {
		LPOLESTR lpolestrCLSID;
		StringFromCLSID(__uuidof(FMANSpecData), &lpolestrCLSID);
		CW2A pszACLSID( lpolestrCLSID );
		std::cerr << "Could not create FMANSpecData" << pszACLSID << " instance for backup, hr("
			<< std::hex << hr << std::dec << ")" << std::endl;
		CoTaskMemFree(lpolestrCLSID);

		return false;
	}

	hr = m_ipFMANSpecDataToMerge.CreateInstance(__uuidof(FMANSpecData));
	if (FAILED(hr)) {
		LPOLESTR lpolestrCLSID;
		StringFromCLSID(__uuidof(FMANSpecData), &lpolestrCLSID);
		CW2A pszACLSID( lpolestrCLSID );
		std::cerr << "Could not create FMANSpecData" << pszACLSID << " instance for merging, hr("
			<< std::hex << hr << std::dec << ")" << std::endl;
		CoTaskMemFree(lpolestrCLSID);

		return false;
	}

	hr = m_ipPeakFinderFactory.CreateInstance(__uuidof(PeakFinderFactory));
	if (FAILED(hr)) {
		LPOLESTR lpolestrCLSID;
		StringFromCLSID(__uuidof(PeakFinderFactory), &lpolestrCLSID);
		CW2A pszACLSID( lpolestrCLSID );
		std::cerr << "Could not create PeakFinderFactory" << pszACLSID << " instance, hr("
			<< std::hex << hr << std::dec << ")" << std::endl;
		CoTaskMemFree(lpolestrCLSID);

		return false;
	}

	// this initialization must be the last statement
	// statement which can fail should not be done after initialization
	m_fInitialized = true;

	startMsgBoxMonitor();

	return true;
}

bool AnalystQS20Interface::termInterface(void) {

	if (m_fCOMInitialized) {
		SysFreeString(m_paramsString);

		m_ipFMANChromData = NULL;
		m_ipFMANSpecData = NULL;
		m_ipFMANWiffFile = NULL;
		m_ipPeakFinderFactory = NULL;
		m_ipPeakFinder = NULL;
		m_ipFMANSpecDataToMerge = NULL;
		m_ipFMANSpecDataCopy = NULL;

		m_fInitialized = false;

		CoUninitialize();
		m_fCOMInitialized = false;
	}

	return (!m_fInitialized);
}

inline void AnalystQS20Interface::initInfoLoader(void) {
	if (NULL!=m_pInstrumentInfoLoader) {
		delete m_pInstrumentInfoLoader;
	}
	m_pInstrumentInfoLoader = new AQS20InstrumentInfoLoader(m_ipFMANWiffFile.GetInterfacePtr(), m_iVerbose);
	if (NULL!=m_pInstrumentInfoParser) {
		delete m_pInstrumentInfoParser;
	}
	m_pInstrumentInfoParser = new InstrumentInfoParser(this->instrumentInfo_, m_iVerbose);
}

inline void AnalystQS20Interface::termInfoLoader(void) {
	if (NULL!=m_pInstrumentInfoLoader) {
		delete m_pInstrumentInfoLoader;
		m_pInstrumentInfoLoader = NULL;
	}
	if (NULL!=m_pInstrumentInfoParser) {
		delete m_pInstrumentInfoParser;
		m_pInstrumentInfoParser = NULL;
	}
}

bool AnalystQS20Interface::setInputFile(const std::string& filename) {
	//----------------------------------------
	{
		// check that we are using the right software to read the data
		std::string strLibraryVersion;
		int nSWRet = getLibraryVersion (m_enumLibraryType, strLibraryVersion);
		if (0 != nSWRet) {
			// We do not proceed as the library DLLs are not available
			std::cerr << "Error: "<<getLibraryName(m_enumLibraryType)<<" library is not installed." << std::endl;
			m_enumCompatibility = COMPATIBILITY_No_Library_Signature;
			if (!m_fAssumeLibrary) {
				return false;
			} else {
				std::cerr << "WARNING: Assumed library could potentially crash program if there is compatibility issue." << std::endl;
			}
		} else {
			if (m_iVerbose>=VERBOSE_INFO) {
				std::cout << "INFO: " << getLibraryName(m_enumLibraryType);
				if (!strLibraryVersion.empty()) {
					std::cout << " Version " << strLibraryVersion;
				}
				std::cout << " is installed." << std::endl;
			}
		}

		std::string strCreator;
		bool fStructuredLoaded = false;
		WiffStructureLoader wiffStructure(filename, m_iVerbose);
		if (wiffStructure.load(false)) {
			fStructuredLoaded = true;
			strCreator = wiffStructure.m_strCreator;
		} else {
			std::cerr << "WARNING: Attempt to fall back on existing handler to file." << std::endl;
			AQS20SoftwareInfoLoader softwareInfoLoader(filename, m_iVerbose);
			if (softwareInfoLoader.load()) {
				fStructuredLoaded = true;
				strCreator = softwareInfoLoader.m_strCreator;
			}
		}

		if (fStructuredLoaded) {
			SoftwareInfoParser softwareInfoParser(this->instrumentInfo_, m_iVerbose);
			softwareInfoParser.parse(strCreator);

			if (COMPATIBILITY_OK == (m_enumCompatibility = isCompatible(m_enumLibraryType,strCreator))) {
				// the file is created with "Analyst QS" software
				if (m_iVerbose>=VERBOSE_INFO) {
					std::cout << "INFO: " << filename.c_str() << " is created with " << strCreator.c_str() << std::endl;
				}
			} else {
				// we generally cannot read file created with Analyst
				// but seem to be be okay with "FMAN MAC" and "FMAN NT"
				// We will just warn the user if there is potential compatibility issue
				std::cerr << "WARNING: " << filename.c_str() << " is created with " << strCreator.c_str() << std::endl;
				std::cerr << "WARNING: There could be compatibility issue accessing it with "<<getLibraryName(m_enumLibraryType)<<" library" << std::endl;
			}
		} else {
			// we can't tell what software the file is created with
			m_enumCompatibility = COMPATIBILITY_No_File_Signature;
			std::cerr << "WARNING: Fail to retrieve " << filename.c_str() << "'s creator software." << std::endl;
			std::cerr << "WARNING: Skipping compatibility check" << std::endl;
		}

		if (wiffStructure.m_fHasWiffScanFile) {
			if (m_iVerbose>=VERBOSE_INFO) {
				std::cout << "INFO: It has an associated scan file with extension .wiff.scan" << std::endl;
			}
			if (wiffStructure.isWiffScanFileExist()) {
				if (m_iVerbose>=VERBOSE_INFO) {
					std::cout << "INFO: Associated scan file located." << std::endl;
				}
			} else {
				std::cerr << "WARNING: Fail to locate associated scan file.  Translation will likely fail." << std::endl;
				std::cerr << "WARNING: Please locate and place associated scan file '" << filename.c_str() << ".scan' in the same folder." << std::endl;
			}
		}

	}

	m_bstrWiffFileName = filename.c_str();
	m_ipFMANChromData->WiffFileName = m_bstrWiffFileName;

	IUnknownPtr ipUnknown(m_ipFMANChromData->GetWiffFileObject());
	ipUnknown->QueryInterface(&m_ipFMANWiffFile);

	m_ipFMANSpecData->WiffFileName = m_bstrWiffFileName;
	m_ipFMANSpecDataCopy->WiffFileName = m_bstrWiffFileName;
	m_ipFMANSpecDataToMerge->WiffFileName = m_bstrWiffFileName;

	// wiff file specific
	loadSampleList();

	// pre-create instance for sample specific object
	initInfoLoader();

	// sample's processing specific
	if (!computeWiffSHA1(filename)) return false;

	return true;
}

void AnalystQS20Interface::loadSampleList(void) {
	long lActualNumberOfSamples = m_ipFMANWiffFile->GetActualNumberOfSamples();

	if (m_iVerbose>=VERBOSE_INFO) {
		std::cout << "INFO: " << lActualNumberOfSamples << " sample(s) recorded." << std::endl;
	}
	for (long lSampleIndex=1; lSampleIndex<=lActualNumberOfSamples; lSampleIndex++) {
		try
		{
			_bstr_t bstrSampleName(m_ipFMANWiffFile->GetSampleName(lSampleIndex));
			m_samples.addSample((const char *)bstrSampleName, SampleTable::STATE_OKAY);
			if (m_iVerbose>=VERBOSE_INFO) {
				std::cout << "INFO: Sample#" << lSampleIndex << "(";
				std::cout << ((const char *) bstrSampleName);
				std::cout << ")" << std::endl;
			}
		} catch (_com_error &com_error) {
			std::string sampleName("(Fail to retrieve name)");
			m_samples.addSample(sampleName.c_str(), SampleTable::STATE_ERROR_NAME);
			if (m_iVerbose>=VERBOSE_INFO) {
				std::cout << "WARNING: Sample#" << lSampleIndex << "(";
				std::cout << (sampleName.c_str());
				std::cout << ") " << std::hex << com_error.Error() << std::dec << std::endl;
			}
		}
	}
}

Scan* AnalystQS20Interface::getScan(void) {
	if (firstTime_) {
		firstTime_ = false;
		return getFirstScan();
	} else {
		return getNextScan();
	}
}

//----------

Scan* AnalystQS20Interface::getFirstScan(void)
{
	initScanIteratorState ();
	if (isValidScan()) {
		// return cache result!!!
		return m_pScan;
	} else {
		return getNextScan ();
	}
}

Scan* AnalystQS20Interface::getNextScan(void)
{
	if (nextScanIteratorState()) {
		// create the scan instance
		while (!isValidScan()) {
			if (!nextScanIteratorState()) {
				return NULL;
			}
		}
		// return cache result!!!
		return m_pScan;
	}
	return NULL;
}

bool AnalystQS20Interface::isValidScan(void) {

	m_pScan = m_iteratorState.popMRMScan ();
	if (NULL != m_pScan) {
		curScanNum_++;
		return true;
	}

	m_pScan = m_iteratorState.popMergedScan ();
	if (NULL != m_pScan) {
		curScanNum_++;
		return true;
	}

	if (m_sampleInfo.getPeriod(m_iteratorState.m_lCurrentPeriodIndex).getExperiment(m_iteratorState.m_lCurrentExperimentIndex).m_chromatogramMeasurements.getMeasurementY(m_iteratorState.m_lCurrentDatapointIndex) > 0) {
		if (!isMRMScan(m_sampleInfo.getPeriod(m_iteratorState.m_lCurrentPeriodIndex).getExperiment(m_iteratorState.m_lCurrentExperimentIndex).m_msScan)) {
			double fxValue;
			fxValue = m_sampleInfo.getPeriod(m_iteratorState.m_lCurrentPeriodIndex).getExperiment(m_iteratorState.m_lCurrentExperimentIndex).m_chromatogramMeasurements.getMeasurementX(m_iteratorState.m_lCurrentDatapointIndex);
			fxValue *= 60;
			HRESULT hr = m_ipFMANSpecData->raw_SetSpectrum(m_sampleInfo.getId(), m_iteratorState.m_lCurrentPeriodIndex, m_iteratorState.m_lCurrentExperimentIndex, (float)fxValue, (float)fxValue);
			if (!SUCCEEDED(hr)) {
				std::cerr << "FATAL: Could not retrieve Spectrum("
					<< m_sampleInfo.getId() 
					<< ", " << m_iteratorState.m_lCurrentPeriodIndex
					<< ", " << m_iteratorState.m_lCurrentExperimentIndex
					<< ", " << fxValue
					<< ", " << fxValue << ") info for scan#" 
					<< (curScanNum_+1) << "." << std::endl;
				_com_issue_errorex(hr, m_ipFMANSpecData.GetInterfacePtr (), m_ipFMANSpecData.GetIID());
				return false;
			}
		}

		m_pScan = new Scan();
		processSpectrum(m_pScan);
		if (NULL == m_pScan) {
			return false;
		}

		curScanNum_++;

		long lMergedScanNum = m_scanOriginToMergeTable.getMergedScanNum(curScanNum_);
		if (lMergedScanNum != curScanNum_) {
			m_pScan->isMerged_ = true;
			m_pScan->mergedScanNum_ = lMergedScanNum;
		}

		return true;
	}
	return false;
}

inline void AnalystQS20Interface::processSpectrum(Scan *pScan) {

	m_sampleInfo.getPeriod(m_iteratorState.m_lCurrentPeriodIndex).getExperiment(m_iteratorState.m_lCurrentExperimentIndex).getSpectraInfo(pScan->scanType_, pScan->polarity_, pScan->msLevel_);

	if (m_DPSettings.m_fCoordinate) {
		pScan->nativeScanRef_.setCoordinateType(instrumentInfo_.manufacturer_);
		pScan->nativeScanRef_.addCoordinate(ABI_COORDINATE_SAMPLE, m_samples.getSampleIdName(m_sampleInfo.getId()));
		pScan->nativeScanRef_.addCoordinate(ABI_COORDINATE_PERIOD, toString(m_iteratorState.m_lCurrentPeriodIndex));
		pScan->nativeScanRef_.addCoordinate(ABI_COORDINATE_EXPERIMENT, toString(m_iteratorState.m_lCurrentExperimentIndex));
		pScan->nativeScanRef_.addCoordinate(ABI_COORDINATE_CYCLE, toString(m_iteratorState.m_lCurrentDatapointIndex));
	}

	if (isMRMScan(pScan->scanType_)) {
		//MRM experiment
		processMRMSpectrum (pScan);
	} else {
		processMSSpectrum (pScan);
	}
}

inline void AnalystQS20Interface::processMSSpectrum(Scan *pScan) {

	if (SUCCEEDED(m_ipFMANSpecData->raw_GetStartMass(&(pScan->startMZ_)))) {
		if (FAILED(m_ipFMANSpecData->raw_GetStopMass(&(pScan->endMZ_)))) {
			pScan->startMZ_ = pScan->endMZ_ = 0;
		}
	} else {
			pScan->startMZ_ = pScan->endMZ_ = 0;
	}

	if (1 == pScan->msLevel_) {
		m_dChromotographRTInSec = pScan->retentionTimeInSec_ = m_sampleInfo.getPeriod(m_iteratorState.m_lCurrentPeriodIndex).getExperiment(m_iteratorState.m_lCurrentExperimentIndex).m_chromatogramMeasurements.getMeasurementX(m_iteratorState.m_lCurrentDatapointIndex) * 60;
	} else {
		if (m_fUseExperiment0RT) {
			pScan->retentionTimeInSec_ = m_dChromotographRTInSec;
		} else {
			pScan->retentionTimeInSec_ = m_sampleInfo.getPeriod(m_iteratorState.m_lCurrentPeriodIndex).getExperiment(m_iteratorState.m_lCurrentExperimentIndex).m_chromatogramMeasurements.getMeasurementX(m_iteratorState.m_lCurrentDatapointIndex) * 60;
		}

		m_sampleInfo.getPeriod(m_iteratorState.m_lCurrentPeriodIndex).getExperiment(m_iteratorState.m_lCurrentExperimentIndex).m_chromatogramAuxMeasurements.getPrecursorInfoExAll (m_iteratorState.m_lCurrentDatapointIndex, pScan->precursorMZ_, pScan->precursorIntensity_, pScan->precursorCharge_, pScan->collisionEnergy_);

		if (0 == pScan->precursorMZ_) {
			_bstr_t bstrParamID(m_ipFMANSpecData->DataTitle);
			const char *szPrecursorMZ = strstr((const char *)bstrParamID, "(");
			if (NULL != szPrecursorMZ) {
				pScan->precursorMZ_ = atof(szPrecursorMZ+1);
			}
		}

		if (0 == pScan->precursorIntensity_) {
			pScan->precursorIntensity_ = m_sampleInfo.getPeriod(m_iteratorState.m_lCurrentPeriodIndex).getExperiment(m_iteratorState.m_lCurrentExperimentIndex).m_chromatogramMeasurements.getMeasurementY(m_iteratorState.m_lCurrentDatapointIndex);
		}

		if (0==pScan->precursorCharge_) {
			if (m_DPSettings.m_fGuessCharge) {
				pScan->precursorCharge_ = m_peaksCharge.getCharge(pScan->precursorMZ_);
#ifndef NODEBUGDETAIL
				if (m_iVerbose>=VERBOSE_DEBUGDETAIL) {
					std::cout << "Precursor(m/z=" << pScan->precursorMZ_ << ", guessed charge=" << pScan->precursorCharge_ << ")" << std::endl;
				}
#endif
			}
		}
	}

#ifndef NODEBUGDETAIL
	if (m_iVerbose>=VERBOSE_DEBUGDETAIL) {
		std::cout << "MS(time=" << pScan->retentionTimeInSec_
			<< ")" << std::endl;
	}
#endif

	processMSSpecData(pScan);
}

inline void AnalystQS20Interface::processMSSpecData(Scan *pScan) {
	double maxIntensity = 0;
	bool fToReport = true;
	if (1 == pScan->msLevel_) {
		if (m_DPSettings.m_fCentroidMS1) {
			centroidSpecData(false, maxIntensity, false, m_DPSettings.m_fGuessCharge);
		} else {
			double minIntensity;
			HRESULT hr = m_ipFMANSpecData->GetYValueRange (&minIntensity, &maxIntensity);
			centroidSpecData(false, maxIntensity, true, m_DPSettings.m_fGuessCharge);
		}
	} else {
		if (doCentroid_) {
			centroidSpecData(doDeisotope_, maxIntensity);
		} else {
			double minIntensity;
			HRESULT hr = m_ipFMANSpecData->GetYValueRange (&minIntensity, &maxIntensity);
		}
	}

	pScan->setNumDataPoints(0);
	pScan->totalIonCurrent_ = 0;
	pScan->basePeakMZ_ = 0;
	pScan->basePeakIntensity_ = 0;

	if (fToReport) {
		// Data Processing: filtering on absolute intensity or % of max intensity
		//                  apply to MS2 or higher
		double thresholdIntensityMin;
		double thresholdIntensityMax;
		if (1 == pScan->msLevel_) {
			// for MS1 data
			m_DPSettings.getPeakCutOffBoundMS1(maxIntensity, thresholdIntensityMin, thresholdIntensityMax);
		} else {
			// for MS2 and higher data
			m_DPSettings.getPeakCutOffBound(maxIntensity, thresholdIntensityMin, thresholdIntensityMax);
		}
		long lSpecDataPoints = m_ipFMANSpecData->GetNumberOfDataPoints();

		pScan->setNumDataPoints (lSpecDataPoints);

		double dpiXVal, dpiYVal;
		long lPeaksCount=0;
		if (thresholdIntensityMin > 0) {
			if (thresholdIntensityMax > 0) {
				// both lower (Min) and upper (Max) bound are set
				for (long dpi=1; dpi<=lSpecDataPoints; dpi++) {
					HRESULT hr = m_ipFMANSpecData->raw_GetDataPoint(dpi, &dpiXVal, &dpiYVal);
					if (dpiYVal>0) {
						if (dpiYVal<thresholdIntensityMin || dpiYVal>thresholdIntensityMax) {
							// noise or no peak
						} else {
							pScan->mzArray_[lPeaksCount] = dpiXVal;
							pScan->totalIonCurrent_ += (pScan->intensityArray_[lPeaksCount]=dpiYVal);
							lPeaksCount++;

							if (pScan->basePeakIntensity_<dpiYVal) {
								pScan->basePeakMZ_ = dpiXVal;
								pScan->basePeakIntensity_ = dpiYVal;
							}
						}
					}
				}
			} else {
				// only a lower bound (Min) is set
				for (long dpi=1; dpi<=lSpecDataPoints; dpi++) {
					HRESULT hr = m_ipFMANSpecData->raw_GetDataPoint(dpi, &dpiXVal, &dpiYVal);
					if (dpiYVal>0) {
						if (dpiYVal<thresholdIntensityMin) {
							// noise or no peak
						} else {
							pScan->mzArray_[lPeaksCount] = dpiXVal;
							pScan->totalIonCurrent_ += (pScan->intensityArray_[lPeaksCount]=dpiYVal);
							lPeaksCount++;

							if (pScan->basePeakIntensity_<dpiYVal) {
								pScan->basePeakMZ_ = dpiXVal;
								pScan->basePeakIntensity_ = dpiYVal;
							}
						}
					}
				}
			}
		} else if (thresholdIntensityMax > 0) {
			// only an upper bound (Max) is set
			for (long dpi=1; dpi<=lSpecDataPoints; dpi++) {
				HRESULT hr = m_ipFMANSpecData->raw_GetDataPoint(dpi, &dpiXVal, &dpiYVal);
				if (dpiYVal>0) {
					if (dpiYVal>thresholdIntensityMax) {
						// noise or no peak
					} else {
						pScan->mzArray_[lPeaksCount] = dpiXVal;
						pScan->totalIonCurrent_ += (pScan->intensityArray_[lPeaksCount]=dpiYVal);
						lPeaksCount++;

						if (pScan->basePeakIntensity_<dpiYVal) {
							pScan->basePeakMZ_ = dpiXVal;
							pScan->basePeakIntensity_ = dpiYVal;
						}
					}
				}
			}
		} else {
			for (long dpi=1; dpi<=lSpecDataPoints; dpi++) {
				HRESULT hr = m_ipFMANSpecData->raw_GetDataPoint(dpi, &dpiXVal, &dpiYVal);
				if (dpiYVal>0.0) {
					pScan->mzArray_[lPeaksCount] = dpiXVal;
					pScan->totalIonCurrent_ += (pScan->intensityArray_[lPeaksCount]=dpiYVal);
					lPeaksCount++;

					if (pScan->basePeakIntensity_<dpiYVal) {
						pScan->basePeakMZ_ = dpiXVal;
						pScan->basePeakIntensity_ = dpiYVal;
					}
				}
			}
		}

		// Data Processing: filtering on min. peak count
		if (1 < pScan->msLevel_ && m_DPSettings.m_iFilterMinPeakCount > 0) {
			if (lPeaksCount<m_DPSettings.m_iFilterMinPeakCount) {
				//The number of peaks is not meeting the min. threshold
				//We ignore all peaks in this spectra then
				lPeaksCount = 0;
			}
		}
		pScan->resetNumDataPoints (lPeaksCount);
		if (lPeaksCount > 0) {
			pScan->minObservedMZ_ = pScan->mzArray_[0];
			pScan->maxObservedMZ_ = pScan->mzArray_[lPeaksCount-1];
		} else {
			pScan->minObservedMZ_ = pScan->maxObservedMZ_ = 0;
		}
	} else {
		pScan->minObservedMZ_ = pScan->maxObservedMZ_ = 0;
	}
}

inline void AnalystQS20Interface::centroidSpecData(bool doDeisotope, double &maxIntensity, bool copy, bool keepCharge) {

	maxIntensity = 0;

	unsigned int control_word_fp;
	control_word_fp = _controlfp(_PC_64, _MCW_PC);

#ifndef NODEBUGDETAIL
	//FIXME: do we have the information in some other form, rather than function call???
	if (m_iVerbose>=VERBOSE_DEBUGDETAIL) {
		std::cout << "Centroid(" << m_ipFMANSpecData->GetNumberOfDataPoints();
	}
#endif
	if (copy) {
		m_ipFMANSpecDataCopy = m_ipFMANSpecData->Copy();
	}
	if (keepCharge) {
		m_peaksCharge.init();
	}
	IUnknownPtr ipUnknownPL(m_ipPeakFinder->FindPeaks(SPECTRUM, m_ipFMANSpecData, -1, 0, m_paramsString));
	IPeakList2Ptr ipPeakList;
	ipUnknownPL->QueryInterface(&ipPeakList);

	HRESULT hr;
	// go thru' the peaklist to construct the final FMANSpecData
	m_ipFMANSpecData->SetNumberOfDataPoints(0);
	m_ipFMANSpecData->XValuesAreSorted = 0;
	long lNumPeaks = ipPeakList->getNumPeaks();
#ifndef NODEBUGDETAIL
	if (m_iVerbose>=VERBOSE_DEBUGDETAIL) {
		std::cout << "," << lNumPeaks;
	}
#endif

	double precursorCentroidValue;
	double precursorCentroidIntensity;
	_variant_t isMonoIsotopic;
	_variant_t chargeState;
	long pointNumber=0;
	bool toAdd=false;
	bool fHeaderReported=false;
	for (long peakIndex=1; peakIndex<=lNumPeaks; peakIndex++) {

		ipPeakList->getPeakDataAsDouble(peakIndex, PEAK_DATA_CENTROID_VALUE, &precursorCentroidValue);
		ipPeakList->getPeakDataAsDouble(peakIndex, PEAK_DATA_AREA, &precursorCentroidIntensity);
		isMonoIsotopic = ipPeakList->getPeakData (peakIndex, PEAK_DATA_IS_MONOISOTOPIC);
		chargeState = ipPeakList->getPeakData (peakIndex, PEAK_DATA_CHARGE_STATE);

		if (doDeisotope) {
			toAdd = (0!=isMonoIsotopic.lVal);
		} else {
			toAdd = true;
		}

		if (toAdd) {
			if (keepCharge) {
				double mzStart,mzEnd, mzIntensity;
				mzStart=mzEnd=mzIntensity=0;
				ipPeakList->getStartPoint(peakIndex, &mzStart, &mzIntensity);
				ipPeakList->getEndPoint(peakIndex, &mzEnd, &mzIntensity);
				m_peaksCharge.add(precursorCentroidValue, mzStart, mzEnd, chargeState.iVal);
			}

			m_ipFMANSpecData->AppendDataPoint(precursorCentroidValue, precursorCentroidIntensity);
			pointNumber++;
			//with some specific .wiff file, this return E_INVALIDARG
			if (FAILED(hr=m_ipFMANSpecData->raw_SetUserValue(pointNumber, isMonoIsotopic.lVal))) {
				if (!fHeaderReported) {
					std::cerr << "COM error " << std::hex << hr << std::dec 
						<< " spectrum("
						<< m_sampleInfo.getId() 
						<< ", " << m_iteratorState.m_lCurrentPeriodIndex
						<< ", " << m_iteratorState.m_lCurrentExperimentIndex
						<< ", " << m_iteratorState.m_lCurrentDatapointIndex
						<< ") Scan#" << curScanNum_;
					printCOMErrorString(hr);
					fHeaderReported = true;
				}
				std::cerr << " peak#" << peakIndex;
			}
		} else {
			long lpointNumber=0;
			ipPeakList->getPeakDataAsDouble(peakIndex, PEAK_DATA_MONOISOTOPIC_MASS, &precursorCentroidValue);
			m_ipFMANSpecData->GetClosestPointNumberForXValue(precursorCentroidValue, CLOSEST_POINT, &lpointNumber);
			double yValue;
			HRESULT hr = m_ipFMANSpecData->raw_GetDataPointYValue(lpointNumber, &yValue);
			precursorCentroidIntensity+=yValue;
			m_ipFMANSpecData->SetDataPoint(lpointNumber, precursorCentroidValue, precursorCentroidIntensity);
		}

		if (precursorCentroidIntensity>maxIntensity) {
			maxIntensity = precursorCentroidIntensity;
		}
	}
	m_ipFMANSpecData->XValuesAreSorted = -1;
	m_ipFMANSpecData->put_IsCentroided(-1);

#ifndef NODEBUGDETAIL
	//FIXME: can we avoid this function call???
	if (m_iVerbose>=VERBOSE_DEBUGDETAIL) {
		std::cout << "," << m_ipFMANSpecData->GetNumberOfDataPoints() << ")";
	}
#endif

	if (copy) {
		m_ipFMANSpecData = m_ipFMANSpecDataCopy->Copy();
	}

	if (fHeaderReported) {
		std::cerr << std::endl;
	}
}

inline void AnalystQS20Interface::processMRMSpectrum(Scan *pScan) {

	MRMExperimentInfo &mrmExperimentInfo = m_sampleInfo.getPeriod(m_iteratorState.m_lCurrentPeriodIndex).getExperiment(m_iteratorState.m_lCurrentExperimentIndex).m_mrmExperimentInfo;
#ifndef NODEBUGDETAIL
	double dPauseBetweenMassRanges;
	mrmExperimentInfo.getPauseBetweenMassRanges(dPauseBetweenMassRanges);
#endif

#ifndef NODEBUGDETAIL
	if (m_iVerbose>=VERBOSE_DEBUGDETAIL) {
		std::cout << "MRM(time=" << m_sampleInfo.getPeriod(m_iteratorState.m_lCurrentPeriodIndex).getExperiment(m_iteratorState.m_lCurrentExperimentIndex).m_chromatogramMeasurements.getMeasurementX(m_iteratorState.m_lCurrentDatapointIndex)
		<< ")" << std::endl;
	}
#endif

	double thresholdIntensityMin;
	double thresholdIntensityMax;
	m_DPSettings.getPeakCutOffBound(thresholdIntensityMin, thresholdIntensityMax);
	long lSpecDataPoints = mrmExperimentInfo.getNumberOfTransitions();
	long lNumberOfReadings = mrmExperimentInfo.getTransitionMeasurementStart (m_iteratorState.m_lCurrentDatapointIndex);

	long lTransitionIndex;
	double dpiXVal, dpiYVal;
	double dDwellTime;
	double dQstepMass;
	double dQstartMass;
	double dCE;
	if (thresholdIntensityMin > 0.0) {
		if (thresholdIntensityMax > 0.0) {
			// both lower (Min) and upper (Max) bound are set
			for (long dpi = 0; dpi < lNumberOfReadings; dpi++) {

				mrmExperimentInfo.getTransitionMeasurement (m_iteratorState.m_lCurrentDatapointIndex, 
					dpi, lTransitionIndex, dpiXVal, dpiYVal);

				mrmExperimentInfo.getTransitionInfo(lTransitionIndex, 
					dDwellTime, dQstepMass, dQstartMass, dCE);

	#ifndef NODEBUGDETAIL
				if (m_iVerbose>=VERBOSE_DEBUGDETAIL) {
					std::cout << "MRM(" << dpi 
						<< "," << dpiXVal
						<< "," << dpiYVal
						<< "," << dDwellTime
						<< "," << dPauseBetweenMassRanges
						<< ")" << std::endl;
				}
	#endif

				if (dpiYVal>0) {
					if (dpiYVal<thresholdIntensityMin || dpiYVal>thresholdIntensityMax) {
						// noise or no peak
					} else {
						// (x,y) = (ipMassRange->GetQstepMass(), dpiYVal)
						Scan *pMRMScan = new Scan ();

						//we use the cached value
						pMRMScan->polarity_ = pScan->polarity_;
						pMRMScan->scanType_ = pScan->scanType_;
						pMRMScan->msLevel_ = pScan->msLevel_;

						pMRMScan->setNumDataPoints(1);
						pMRMScan->mzArray_[0] 
							= pMRMScan->minObservedMZ_ 
							= pMRMScan->maxObservedMZ_ 
							= dQstepMass;
						pMRMScan->intensityArray_[0] = dpiYVal;
						pMRMScan->totalIonCurrent_ += dpiYVal;

						pMRMScan->basePeakIntensity_ = dpiYVal;
						pMRMScan->basePeakMZ_ 
							= pMRMScan->startMZ_ 
							= pMRMScan->endMZ_ 
							= dQstepMass;
						pMRMScan->precursorIntensity_ = 0;
						pMRMScan->precursorMZ_ = dQstartMass;
						pMRMScan->retentionTimeInSec_ = dpiXVal;
						pMRMScan->collisionEnergy_ = dCE;

						pMRMScan->nativeScanRef_ = pScan->nativeScanRef_;

						m_iteratorState.pushMRMScan(pMRMScan);
					}
				}
			}
		} else {
			// only a lower bound (Min) is set
			for (long dpi = 0; dpi < lNumberOfReadings; dpi++) {

				mrmExperimentInfo.getTransitionMeasurement (m_iteratorState.m_lCurrentDatapointIndex, 
					dpi, lTransitionIndex, dpiXVal, dpiYVal);

				mrmExperimentInfo.getTransitionInfo(lTransitionIndex, 
					dDwellTime, dQstepMass, dQstartMass, dCE);

	#ifndef NODEBUGDETAIL
				if (m_iVerbose>=VERBOSE_DEBUGDETAIL) {
					std::cout << "MRM(" << dpi 
						<< "," << dpiXVal
						<< "," << dpiYVal
						<< "," << dDwellTime
						<< "," << dPauseBetweenMassRanges
						<< ")" << std::endl;
				}
	#endif

				if (dpiYVal>0) {
					if (dpiYVal<thresholdIntensityMin) {
						// noise or no peak
					} else {
						// (x,y) = (ipMassRange->GetQstepMass(), dpiYVal)
						Scan *pMRMScan = new Scan ();

						//we use the cached value
						pMRMScan->polarity_ = pScan->polarity_;
						pMRMScan->scanType_ = pScan->scanType_;
						pMRMScan->msLevel_ = pScan->msLevel_;

						pMRMScan->setNumDataPoints(1);
						pMRMScan->mzArray_[0] 
							= pMRMScan->minObservedMZ_ 
							= pMRMScan->maxObservedMZ_ 
							= dQstepMass;
						pMRMScan->intensityArray_[0] = dpiYVal;
						pMRMScan->totalIonCurrent_ += dpiYVal;

						pMRMScan->basePeakIntensity_ = dpiYVal;
						pMRMScan->basePeakMZ_ 
							= pMRMScan->startMZ_ 
							= pMRMScan->endMZ_ 
							= dQstepMass;
						pMRMScan->precursorIntensity_ = 0;
						pMRMScan->precursorMZ_ = dQstartMass;
						pMRMScan->retentionTimeInSec_ = dpiXVal;
						pMRMScan->collisionEnergy_ = dCE;

						pMRMScan->nativeScanRef_ = pScan->nativeScanRef_;

						m_iteratorState.pushMRMScan(pMRMScan);
					}
				}
			}
		}
	} else if (thresholdIntensityMax > 0.0) {
		// only an upper bound (Max) is set
		for (long dpi = 0; dpi < lNumberOfReadings; dpi++) {

			mrmExperimentInfo.getTransitionMeasurement (m_iteratorState.m_lCurrentDatapointIndex, 
				dpi, lTransitionIndex, dpiXVal, dpiYVal);

			mrmExperimentInfo.getTransitionInfo(lTransitionIndex, 
				dDwellTime, dQstepMass, dQstartMass, dCE);

#ifndef NODEBUGDETAIL
			if (m_iVerbose>=VERBOSE_DEBUGDETAIL) {
				std::cout << "MRM(" << dpi 
					<< "," << dpiXVal
					<< "," << dpiYVal
					<< "," << dDwellTime
					<< "," << dPauseBetweenMassRanges
					<< ")" << std::endl;
			}
#endif

			if (dpiYVal>0) {
				if (dpiYVal>thresholdIntensityMax) {
					// noise or no peak
				} else {
					// (x,y) = (ipMassRange->GetQstepMass(), dpiYVal)
					Scan *pMRMScan = new Scan ();

					//we use the cached value
					pMRMScan->polarity_ = pScan->polarity_;
					pMRMScan->scanType_ = pScan->scanType_;
					pMRMScan->msLevel_ = pScan->msLevel_;

					pMRMScan->setNumDataPoints(1);
					pMRMScan->mzArray_[0] 
						= pMRMScan->minObservedMZ_ 
						= pMRMScan->maxObservedMZ_ 
						= dQstepMass;
					pMRMScan->intensityArray_[0] = dpiYVal;
					pMRMScan->totalIonCurrent_ += dpiYVal;

					pMRMScan->basePeakIntensity_ = dpiYVal;
					pMRMScan->basePeakMZ_ 
						= pMRMScan->startMZ_ 
						= pMRMScan->endMZ_ 
						= dQstepMass;
					pMRMScan->precursorIntensity_ = 0;
					pMRMScan->precursorMZ_ = dQstartMass;
					pMRMScan->retentionTimeInSec_ = dpiXVal;
					pMRMScan->collisionEnergy_ = dCE;

					pMRMScan->nativeScanRef_ = pScan->nativeScanRef_;

					m_iteratorState.pushMRMScan(pMRMScan);
				}
			}
		}
	} else {
		for (long dpi = 0; dpi < lNumberOfReadings; dpi++) {

			mrmExperimentInfo.getTransitionMeasurement (m_iteratorState.m_lCurrentDatapointIndex, 
				dpi, lTransitionIndex, dpiXVal, dpiYVal);

			mrmExperimentInfo.getTransitionInfo(lTransitionIndex, 
				dDwellTime, dQstepMass, dQstartMass, dCE);

#ifndef NODEBUGDETAIL
			if (m_iVerbose>=VERBOSE_DEBUGDETAIL) {
				std::cout << "MRM(" << dpi 
					<< "," << dpiXVal
					<< "," << dpiYVal
					<< "," << dDwellTime
					<< "," << dPauseBetweenMassRanges
					<< ")" << std::endl;
			}
#endif

			if (dpiYVal>0) {
				// (x,y) = (ipMassRange->GetQstepMass(), dpiYVal)
				Scan *pMRMScan = new Scan ();

				//we use the cached value
				pMRMScan->polarity_ = pScan->polarity_;
				pMRMScan->scanType_ = pScan->scanType_;
				pMRMScan->msLevel_ = pScan->msLevel_;

				pMRMScan->setNumDataPoints(1);
				pMRMScan->mzArray_[0] 
					= pMRMScan->minObservedMZ_ 
					= pMRMScan->maxObservedMZ_ 
					= dQstepMass;
				pMRMScan->intensityArray_[0] = dpiYVal;
				pMRMScan->totalIonCurrent_ += dpiYVal;

				pMRMScan->basePeakIntensity_ = dpiYVal;
				pMRMScan->basePeakMZ_ 
					= pMRMScan->startMZ_ 
					= pMRMScan->endMZ_ 
					= dQstepMass;
				pMRMScan->precursorIntensity_ = 0;
				pMRMScan->precursorMZ_ = dQstartMass;
				pMRMScan->retentionTimeInSec_ = dpiXVal;
				pMRMScan->collisionEnergy_ = dCE;

				pMRMScan->nativeScanRef_ = pScan->nativeScanRef_;

				m_iteratorState.pushMRMScan(pMRMScan);
			}
		}
	}

	delete m_pScan;
	m_pScan = NULL;
	if (m_iteratorState.isMRMScanAvailable()) {
		m_pScan = m_iteratorState.popMRMScan ();
	}
}

void AnalystQS20Interface::mergeScans (
	long lPeriodIndex, const ScanTable &scanTable, 
	long &lMergedScanCount, long &lTotalScansConsidered, MergedScanTable &mergedScans) {

	lMergedScanCount = lTotalScansConsidered = 0;

	std::vector<bool> experimentsMergable;
	m_sampleInfo.getPeriod(lPeriodIndex).getExperimentMergeStates (experimentsMergable);

	// we need precursor mz, and charge
	getSamplePrecursorInfo (m_sampleInfo.getId(), lPeriodIndex);
	if (!m_sampleInfo.isDDEAvailable()) {
		// we cannot continue if we do not have at least the precursor mz
		return;
	}

	MergeCandidates sampleMergeCandidates;
	sampleMergeCandidates.clear();

	if (0==m_DPSettings.m_dGroupPrecursorMassTolerance) return;

	/*
	when m_DPSettings.m_iGroupMinCycles = 1, 
	we are not interested  to report spectra with a single scan [considered not merged]
	*/
	int effectiveMinTotalSpectras = (m_DPSettings.m_iGroupMinCycles>1) ? m_DPSettings.m_iGroupMinCycles : 2;

	double dPrecursorMZ;
	double dPrecursorIntensity;
	int nPrecursorCharge;
	long lScanNumber;
	long lNumberOfDataPoints=m_sampleInfo.getPeriod(lPeriodIndex).getExperiment(0).getNumberOfDataPoints();
	long lNumberOfExperiments=m_sampleInfo.getPeriod(lPeriodIndex).getNumberOfExperiments();
#if 0
	double dMascotPrecision=0.1;
#endif
	for (long lDataPointsIndex=1; lDataPointsIndex<=lNumberOfDataPoints; lDataPointsIndex++) {
		for (long lExperimentIndex=1; lExperimentIndex<lNumberOfExperiments; lExperimentIndex++) {
			// scan type is not mergable ?
			if (!experimentsMergable[lExperimentIndex]) continue;

			// precursor MZ is not available?
			m_sampleInfo.getPeriod(lPeriodIndex).getExperiment(lExperimentIndex).m_chromatogramAuxMeasurements.getPrecursorMZCharge(lDataPointsIndex, dPrecursorMZ, dPrecursorIntensity, nPrecursorCharge);
			if (0==dPrecursorMZ) continue;
			if (0==dPrecursorIntensity) continue;

			// scan number not allocated?
			scanTable.getPrecursorScanNumber(lExperimentIndex, lDataPointsIndex, lScanNumber);
			if (0==lScanNumber) continue;

#if 0
			//TODO: on AnalystQS Mascot.dll uses 1 decimal place for precursor MZ
			//      if we do not follow, we will have different merged scans selection
			dPrecursorMZ = (int((dPrecursorMZ/dMascotPrecision)+0.5))*dMascotPrecision;
#endif

			// this scan has cleared the threshold, candidate for merge checking
			sampleMergeCandidates.push_back(MergeCandidate(lExperimentIndex, lDataPointsIndex, dPrecursorMZ, nPrecursorCharge, lScanNumber));
		}
	}

	// go thru' the candidate and merge any possible
	long lMergedScanNumber = scanTable.getLastScanNumberAssigned();
	MergeCandidates::size_type numMergeCandidates=sampleMergeCandidates.size();
	for(MergeCandidates::size_type i=0; i<numMergeCandidates; i++) {
		MergeCandidate &mergeCandidate = sampleMergeCandidates[i];

		if (m_iVerbose>=VERBOSE_DEBUGDETAIL) {
			std::cout << "\nProcessing seed ";
			mergeCandidate.dump();
		}

		// if a scan is already merged, it is excluded from future merging
		if (mergeCandidate.m_fMerged) continue;

		MergedScan mergedScan(mergeCandidate, i);
		long lLastDataPointIndex = mergeCandidate.m_lDataPointIndex;
		int nPrecursorCharge = mergeCandidate.m_nPrecursorCharge;
		for(MergeCandidates::size_type j=i+1; j<numMergeCandidates; j++) {
			MergeCandidate &candidateToMerge = sampleMergeCandidates[j];

#ifndef NODEBUGDETAIL
			if (m_iVerbose>=VERBOSE_DEBUGDETAIL) {
				std::cout << "\n\tChecking ";
				candidateToMerge.dump();
			}

			if (m_iVerbose>=VERBOSE_DEBUGDETAIL) {
				std::cout << "\t\t" << (candidateToMerge.m_fMerged ? "merged" : "avail");
			}
#endif

			// if a scan is already merged, it is excluded from future merging
			if (candidateToMerge.m_fMerged) continue;

#ifndef NODEBUGDETAIL
			if (m_iVerbose>=VERBOSE_DEBUGDETAIL) {
				std::cout << " Cycle Span " << (candidateToMerge.m_lDataPointIndex - lLastDataPointIndex);
			}
#endif

			//if ((candidateToMerge.m_lDataPointIndex - mergeCandidate.m_lDataPointIndex) > m_DPSettings.m_iGroupMaxCyclesBetween) {
			if ((candidateToMerge.m_lDataPointIndex - lLastDataPointIndex) > m_DPSettings.m_iGroupMaxCyclesBetween) {
				// exceed the cycle span in group, no further potential candidate
				break;
			}

#ifndef NODEBUGDETAIL
			if (m_iVerbose>=VERBOSE_DEBUGDETAIL) {
				std::cout << " Charge " << nPrecursorCharge << " " << candidateToMerge.m_nPrecursorCharge;
			}
#endif

			// TODO: merging should consider same charge, but not unknown charge?
			if (0 != nPrecursorCharge) {
				if (0!=candidateToMerge.m_nPrecursorCharge && candidateToMerge.m_nPrecursorCharge != nPrecursorCharge) {
					// the precursor at different charge are not combined, not a candidate
					continue;
				}
			}

#ifndef NODEBUGDETAIL
			if (m_iVerbose>=VERBOSE_DEBUGDETAIL) {
				std::cout << " PrecursorMZ " << candidateToMerge.m_dPrecursorMZ << " " << (fabs(candidateToMerge.m_dPrecursorMZ - mergeCandidate.m_dPrecursorMZ));
			}
#endif

			if (fabs(candidateToMerge.m_dPrecursorMZ - mergeCandidate.m_dPrecursorMZ) > m_DPSettings.m_dGroupPrecursorMassTolerance) {
				// the precursor MZ is not within the tolerance, not a candidate
				continue;
			}

#ifndef NODEBUGDETAIL
			if (m_iVerbose>=VERBOSE_DEBUGDETAIL) {
				std::cout << " ADDED" << std::endl;
			}
#endif

			// pass the above criteria, so the scan can be merged
			mergedScan.mergeScan (candidateToMerge, j);
			lLastDataPointIndex = candidateToMerge.m_lDataPointIndex;
			if (0 == nPrecursorCharge) {
				nPrecursorCharge = candidateToMerge.m_nPrecursorCharge;
			}

		}

		if (mergedScan.getNumberOfMergedScan()>1) {
			lTotalScansConsidered++;
		}

		if (mergedScan.getNumberOfMergedScan()>=effectiveMinTotalSpectras) {
			// scans merged, considered them as "used for merge"
			// link these scan origins with the final merged scan number
			lMergedScanNumber++;
			for (long k=0; k < mergedScan.getNumberOfMergedScan(); k++) {
				sampleMergeCandidates[mergedScan.getCandidateId(k)].m_fMerged = true;
				m_scanOriginToMergeTable.add (mergedScan.getMergedScanNumber(k), lMergedScanNumber);
			}

			mergedScans.cacheSampleMergedScan(mergedScan);
			lMergedScanCount++;
		}
	}

	//Sanity check
	if (m_iVerbose>=VERBOSE_DEBUG) {
		std::cout << "DEBUG: merged scans details" << std::endl;
		mergedScans.dump();
	}
}

ScanPtr AnalystQS20Interface::instantiateMergedScans (long lPeriodIndex, const MergedScan &mergedScan) {

	std::auto_ptr<Scan> apScan(new Scan());

	long lExperimentIndex=mergedScan.m_mergedExperimentIndex[0];
	long lDataPointIndex=mergedScan.m_mergedDataPointIndex[0];

	apScan->setNumScanOrigins((int)mergedScan.m_mergedScanNumber.size());
	apScan->scanOriginNums[0] = mergedScan.m_mergedScanNumber[0];
	apScan->scanOriginParentFileIDs[0] = sha1Report_;
	apScan->isMerged_ = true;

	float fxValue;
	fxValue = (float)(m_dChromotographRTInSec = apScan->retentionTimeInSec_ = m_sampleInfo.getPeriod(lPeriodIndex).getExperiment(0).m_chromatogramMeasurements.getMeasurementX(lDataPointIndex) * 60);
	HRESULT hr = m_ipFMANSpecData->raw_SetSpectrum(m_sampleInfo.getId(), lPeriodIndex, lExperimentIndex, fxValue, fxValue);
	if (FAILED(hr)) {
		std::cerr << "FATAL: Could not retrieve Spectrum("
			<< m_sampleInfo.getId() 
			<< ", " << lPeriodIndex
			<< ", " << lExperimentIndex
			<< ", " << fxValue
			<< ", " << fxValue << ") info." << std::endl;
		_com_issue_errorex(hr, m_ipFMANSpecData.GetInterfacePtr (), m_ipFMANSpecData.GetIID());
	}

	//-----

	m_sampleInfo.getPeriod(lPeriodIndex).getExperiment(lExperimentIndex).getSpectraInfo(apScan->scanType_, apScan->polarity_, apScan->msLevel_);

	m_sampleInfo.getPeriod(lPeriodIndex).getExperiment(lExperimentIndex).m_chromatogramAuxMeasurements.getPrecursorInfoExAll (lDataPointIndex, apScan->precursorMZ_, apScan->precursorIntensity_, apScan->precursorCharge_, apScan->collisionEnergy_);

	if (SUCCEEDED(m_ipFMANSpecData->raw_GetStartMass(&(apScan->startMZ_)))) {
		if (FAILED(m_ipFMANSpecData->raw_GetStopMass(&(apScan->endMZ_)))) {
			apScan->startMZ_ = apScan->endMZ_ = 0;
		}
	} else {
			apScan->startMZ_ = apScan->endMZ_ = 0;
	}

	//-----

	double dAddTolerance = 0.001; // TODO: should be a user-defined settings

	for (long i=1; i<mergedScan.getNumberOfMergedScan (); i++) {
		lExperimentIndex=mergedScan.m_mergedExperimentIndex[i];
		lDataPointIndex=mergedScan.m_mergedDataPointIndex[i];

		apScan->scanOriginNums[i] = mergedScan.m_mergedScanNumber[i];
		apScan->scanOriginParentFileIDs[i] = sha1Report_;

		fxValue = (float) m_sampleInfo.getPeriod(lPeriodIndex).getExperiment(lExperimentIndex).m_chromatogramMeasurements.getMeasurementX(lDataPointIndex) * 60;
		hr = m_ipFMANSpecDataToMerge->raw_SetSpectrum(m_sampleInfo.getId(), lPeriodIndex, lExperimentIndex, fxValue, fxValue);
		if (FAILED(hr)) {
			std::cerr << "FATAL: Could not retrieve Spectrum("
				<< m_sampleInfo.getId() 
				<< ", " << lPeriodIndex
				<< ", " << lExperimentIndex
				<< ", " << fxValue
				<< ", " << fxValue << ") for merging." << std::endl;
			_com_issue_errorex(hr, m_ipFMANSpecDataToMerge.GetInterfacePtr (), m_ipFMANSpecDataToMerge.GetIID());
		}

		hr = m_ipFMANSpecData->Add(m_ipFMANSpecDataToMerge, dAddTolerance);
		if (FAILED(hr)) {
			std::cerr << "FATAL: Could not add Spectrum("
				<< m_sampleInfo.getId() 
				<< ", " << lPeriodIndex
				<< ", " << lExperimentIndex
				<< ", " << fxValue
				<< ", " << fxValue << ") for merging." << std::endl;
			_com_issue_errorex(hr, m_ipFMANSpecDataToMerge.GetInterfacePtr (), m_ipFMANSpecDataToMerge.GetIID());
		}
	}

	processMSSpecData(apScan.get());

	return apScan.release();
}

void AnalystQS20Interface::getSamplePrecursorInfo (long lSampleIndex, long lPeriodIndex) {

	if (lSampleIndex==m_sampleInfo.getId() && m_sampleInfo.isDDEAvailable()) {
		return;
	}

	getSamplePrecursorInfoImpl (lSampleIndex, lPeriodIndex);
#ifndef NODEBUGDETAIL
	// Sanity check!
	if (m_iVerbose>=VERBOSE_DEBUGDETAIL) {
		std::cout << "DEBUG: sample DDE details" << std::endl;
		for (long lPeriodIndex=0; lPeriodIndex < m_sampleInfo.getNumberOfPeriods(); lPeriodIndex++) {
			PeriodInfo &period = m_sampleInfo.getPeriod(lPeriodIndex);
			for (long lExperimentIndex=1; lExperimentIndex<period.getNumberOfExperiments(); lExperimentIndex++) {
				ExperimentInfo &experiment = period.getExperiment(lExperimentIndex);
				experiment.m_chromatogramAuxMeasurements.dump();
			}
		}
	}
#endif
}

void AnalystQS20Interface::getSamplePrecursorInfoImpl (long lSampleIndex, long lPeriodIndex) {

	if (m_sampleInfo.isDDEAvailable()) {
		return;
	}

	long lNumberOfDataPoints = 0;
	long lActualNumberOfExperiments = 0;
	{
		PeriodInfo &period = m_sampleInfo.getPeriod(0);
		lActualNumberOfExperiments = period.getNumberOfExperiments();
		lNumberOfDataPoints = period.getExperiment(0).getNumberOfDataPoints ();
	}

	IUnknownPtr ipUnknownFileManagerObject(m_ipFMANWiffFile->GetFileManagerObject());
	IFMWIFFPtr ipIFMWIFF;
	HRESULT hr = ipUnknownFileManagerObject->QueryInterface(&ipIFMWIFF);
	unsigned long dwFileHandle=0;
	hr = ipIFMWIFF->FMANGetWIFFDriverFileHandle(&dwFileHandle);

	IUnknownPtr ipUnknownWTS;
	hr = ipIFMWIFF->FMANWIFFGetWTS (&ipUnknownWTS);

	IWIFFTSvrDDERealTimeDataExPtr ipWIFFTSvrDDERealTimeDataEx(ipUnknownWTS);

	try {
		// TODO: WCH
		// DDERealTimeData has no associated period, but
		// we do not have sample data to test this behaviour
		long lStreamDataObject=0;
		hr = ipWIFFTSvrDDERealTimeDataEx->OpenArrStrCLS_DDERealTimeDataEx(dwFileHandle, &lStreamDataObject, lSampleIndex);

		if (lNumberOfDataPoints>0 && lActualNumberOfExperiments>1) {
			long lDDEExIndex=0;
			double dDDEMass=0; 
			long lDDETime=0; 
			short sDDECharge=0; 
			float fDDECollisionEnergy=0, fCollisionEnergyForMS3=0; 
			double dDDEIntensity=0; 
			short sDDEScanType=0; 
			long lDDEPeriodIndex=0, lDDEExperimIndex=0, lDDECycleIndex=0, lDDEParentMassIndex=0; 
			double dDDEDynamicFillTime=0, dDDEExReserved2=0; 
			long lDDEExReserved3=0, lDDEExReserved4=0;
			try {
				do {
					hr = ipWIFFTSvrDDERealTimeDataEx->ReadArrStrCLS_DDERealTimeDataEx(lStreamDataObject, 
						&lDDEExIndex, &dDDEMass, &lDDETime, &sDDECharge, 
						&fDDECollisionEnergy, &fCollisionEnergyForMS3, &dDDEIntensity,
						&sDDEScanType, &lDDEPeriodIndex, &lDDEExperimIndex, &lDDECycleIndex,
						&lDDEParentMassIndex, &dDDEDynamicFillTime, 
						&dDDEExReserved2, &lDDEExReserved3, &lDDEExReserved4);
					if (SUCCEEDED(hr)) {
						if (dDDEMass>0) {
							m_sampleInfo.getPeriod(lDDEPeriodIndex).getExperiment(lDDEExperimIndex).m_chromatogramAuxMeasurements.cacheAuxMeasurementExAll(
								lDDECycleIndex+1, dDDEMass, dDDEIntensity, sDDECharge, fDDECollisionEnergy);
						}
					} else {
						std::cerr << "DDERealTimeDataEx(): hr="  << std::hex << hr << std::dec << std::endl;
						break;
					}
				} while (true);
			} catch (_com_error &com_error) {
				// do nothing
				//std::cerr << "Error: com_error in DDEEx processing, hr=" << std::hex << hr << std::dec << std::endl;
				_VARIABLE_UNUSED(com_error);
			}
			if (lDDEExIndex>0) {
				m_sampleInfo.DDEExCached();
			}
		}

		hr = ipWIFFTSvrDDERealTimeDataEx->CloseArrStrCLS_DDERealTimeDataEx(lStreamDataObject);
	} catch (_com_error &com_error) {
		// do nothing
		//std::cerr << "Error: com_error in DDEEx processing, hr=" << std::hex << hr << std::dec << std::endl;
		_VARIABLE_UNUSED(com_error);
	}

	if (m_sampleInfo.isDDEAvailable()) {
		return;
	}

	IWIFFTSvrDDERealTimeDataPtr ipWIFFTSvrDDERealTimeData(ipUnknownWTS);

	try {
		// TODO: WCH
		// DDERealTimeData has no associated period, but
		// we do not have sample data to test this behaviour
		long lStreamDataObject=0;
		hr = ipWIFFTSvrDDERealTimeData->OpenArrStrCLS_DDERealTimeData(dwFileHandle, &lStreamDataObject, lSampleIndex);

		if (lNumberOfDataPoints>0 && lActualNumberOfExperiments>1) {
			double dDDERTPrecursorMass=0;
			long lDDERTIndex=0;
			long lDDERTCount=0;
			double dDDERTReserved1=0;
			double dDDERTReserved2=0;

			try {
				for (long lPeriodIndex=0; lPeriodIndex < m_sampleInfo.getNumberOfPeriods(); lPeriodIndex++) {
					PeriodInfo &period = m_sampleInfo.getPeriod(lPeriodIndex);
					for (long lDataPointIndex=1; lDataPointIndex<=lNumberOfDataPoints; lDataPointIndex++) {
						for (long lExperimentIndex=1; lExperimentIndex<lActualNumberOfExperiments; lExperimentIndex++) {
							hr = ipWIFFTSvrDDERealTimeData->ReadArrStrCLS_DDERealTimeData(lStreamDataObject, 
								&dDDERTPrecursorMass, &lDDERTIndex, &lDDERTCount, &dDDERTReserved1, &dDDERTReserved2);
							if (dDDERTPrecursorMass>0) {
								period.getExperiment(lExperimentIndex).
									m_chromatogramAuxMeasurements.cacheAuxMeasurement(
									lDataPointIndex, dDDERTPrecursorMass, dDDERTReserved1);
							}
						}
					}
				}
				m_sampleInfo.DDECached();
			} catch (_com_error &com_error) {
				// do nothing
				//std::cerr << "Error: com_error in DDE processing, hr=" << std::hex << hr << std::dec << std::endl;
				_VARIABLE_UNUSED(com_error);
			}
		}
		hr = ipWIFFTSvrDDERealTimeData->CloseArrStrCLS_DDERealTimeData(lStreamDataObject);
	} catch (_com_error &com_error) {
		// do nothing
		//std::cerr << "Error: com_error in DDE processing, hr=" << std::hex << hr << std::dec << std::endl;
		_VARIABLE_UNUSED(com_error);
	}
}

inline void AnalystQS20Interface::loadMergedScans (long lPeriodIndex) {
	MergedScanTableCache::const_iterator iterCache= m_sampleMergedScansCache.find(SamplePeriodKey(lPeriodIndex));
	if (iterCache != m_sampleMergedScansCache.end()) {
		std::cout << "Computing merge scans";
		long lIndex=0;
		for(MergedScanTable::MergedScans::const_iterator iter=iterCache->second.m_experimentsMergedScans.begin();
			iter != iterCache->second.m_experimentsMergedScans.end();
			++iter) {
			lIndex++;
			if ((lIndex % 100) == 0) {
				std::cout << "M" << lIndex;
			}
			else if ((lIndex % 10) == 0) {
				std::cout << ".";
			}
			m_iteratorState.pushMergedScan(instantiateMergedScans (lPeriodIndex, *iter));
		}
		if (0!=(lIndex % 100)) {
			std::cout << "*M" << lIndex << "*";
		}
		if (m_iVerbose>=VERBOSE_DEBUG) {
			if (lIndex>0) std::cout << "DEBUG: merged scan peak list" << std::endl;
			lIndex=0;
			for(std::deque<ScanPtr>::const_iterator iter=m_iteratorState.m_queueMergedScans.begin();
				iter != m_iteratorState.m_queueMergedScans.end();
				++iter) {
				lIndex++;
				ScanPtr pScan = *iter;
				std::cout << "Merged Scan#" << lIndex << ": " << pScan->precursorMZ_ 
					<< ", " << pScan->precursorIntensity_ 
					<< ", " << pScan->precursorCharge_ << std::endl;
				for (int i=0; i<pScan->getNumDataPoints(); i++) {
					std::cout << "pk#" << (i+1) 
						<< "\t" << pScan->mzArray_[i] 
						<< "\t" << pScan->intensityArray_[i] 
						<< std::endl;
				}
			}			
		}
	}
}

inline void AnalystQS20Interface::initScanIteratorState () {

	curScanNum_ = 0;

	m_iteratorState.init();

	getSamplePrecursorInfo (m_sampleInfo.getId(), m_iteratorState.m_lCurrentPeriodIndex);

	//m_iteratorState.initNewDatapoints();

	IUnknownPtr ipUnknownPF(m_ipPeakFinderFactory->CreatePeakFinder (m_bstrWiffFileName, m_sampleInfo.getId(), FACTORY_SPECTRUM, &m_paramsString));
	ipUnknownPF->QueryInterface(&m_ipPeakFinder);

	m_fUseExperiment0RT = m_sampleInfo.getPeriod(m_iteratorState.m_lCurrentPeriodIndex).useExperiment0RetentionTime();


	// set up the merged scan
	loadMergedScans (m_iteratorState.m_lCurrentPeriodIndex);
}

inline bool AnalystQS20Interface::nextScanIteratorState () {

	ScanIteratorState::IteratorStateType state = m_iteratorState.next ();

	if (ScanIteratorState::STATE_NO_TRANSITION==state) {

		return true;

	} else if (ScanIteratorState::STATE_NEW_PERIOD==state) {
		// we have moved to a new period
		// number of experiments, and number of data points can changed

		getSamplePrecursorInfo (m_sampleInfo.getId(), m_iteratorState.m_lCurrentPeriodIndex);

		m_iteratorState.initNewPeriod();

		m_fUseExperiment0RT = m_sampleInfo.getPeriod(m_iteratorState.m_lCurrentPeriodIndex).useExperiment0RetentionTime();

		//FIXME: do we still need to reposition the stream?
		// set up the merged scan
		loadMergedScans (m_iteratorState.m_lCurrentPeriodIndex);
		// reposition data stream
		HRESULT hr = m_ipFMANChromData->SetToTIC(
			m_sampleInfo.getId(), 
			m_iteratorState.m_lCurrentPeriodIndex, 0);
		if (FAILED(hr)) {
			std::cerr << "FATAL: Chromatogram.TIC("
				<<m_sampleInfo.getId()<<", "
				<<m_iteratorState.m_lCurrentPeriodIndex<<", 0), COM error " 
				<< std::hex << hr << std::dec << std::endl;
			printCOMErrorString(hr);
			_com_issue_errorex(hr, m_ipFMANChromData.GetInterfacePtr (), m_ipFMANChromData.GetIID());
		}

		return true;

	} else if (ScanIteratorState::STATE_MRM_CONTINUE==state) {
		// MRM scans are cached, so no computation needed
		return true;

	} else if (ScanIteratorState::STATE_MERGEDSCAN_CONTINUE==state) {
		// MRM scans are cached, so no computation needed
		return true;

	} else if (ScanIteratorState::STATE_ENDED==state) {
		return false;

	}

	return false;
}

void AnalystQS20Interface::loadInstrumentInfo(long lSampleId) {
	if (m_pInstrumentInfoLoader->load(lSampleId)) {
		m_pInstrumentInfoParser->parse(m_pInstrumentInfoLoader->m_strLog, m_pInstrumentInfoLoader->m_lModelId);
	}
}

void AnalystQS20Interface::loadSampleInfo(long lSampleId) {
	// get MSRunTime - get be generated from scan count processing
	// compute the scan count

	if (lSampleId != m_sampleInfo.getId ()) {

		totalNumScans_ = 0;

		_bstr_t bstrParamID("CE");

		bool fToWarn = false;
		bool fMCAData = false;
		bool fToMergeScan = true; //local???
		double dMRMThresholdMin; //local or member?
		double dMRMThresholdMax;//local or member?
		m_DPSettings.getPeakCutOffBound(dMRMThresholdMin, dMRMThresholdMax);

		long lActualNumberOfPeriods;
		try
		{
			lActualNumberOfPeriods = m_ipFMANWiffFile->GetActualNumberOfPeriods(lSampleId);
			if (lActualNumberOfPeriods < 0) {
				std::cerr << "WARNING: Number of period in Sample#"<< lSampleId << " is negative!" << std::endl;
				lActualNumberOfPeriods = 0;
			}
		} catch (_com_error &com_error) {
			_VARIABLE_UNUSED(com_error);
			lActualNumberOfPeriods = 0;
			if (m_iVerbose>=VERBOSE_INFO) {
				std::cout << "WARNING: Sample#" << lSampleId << " Failed to get number of periods." << std::endl;
			}
		}

		m_sampleInfo.init (lSampleId, lActualNumberOfPeriods);

		for (long lPeriodIndex=0; lPeriodIndex<lActualNumberOfPeriods; lPeriodIndex++) {

			PeriodInfo &period = m_sampleInfo.getPeriod(lPeriodIndex);

			long lActualNumberOfExperiments = m_ipFMANWiffFile->GetNumberOfExperiments(lSampleId,lPeriodIndex);
			if (m_iVerbose>=VERBOSE_INFO) {
				std::cout << "INFO:    Period#" << lPeriodIndex << ", has " << lActualNumberOfExperiments << " experiment(s) recorded." << std::endl;
			}
			period.init(lActualNumberOfExperiments);

			long lNumberOfDataPoints;
			HRESULT hr = m_ipFMANChromData->raw_SetToTIC(lSampleId, lPeriodIndex, 0);
			if (FAILED(hr)) {
				std::cerr << "Error: Chromatogram.TIC("<<lSampleId<<", "<<lPeriodIndex<<", 0), COM error " 
					<< std::hex << hr << std::dec << std::endl;
				printCOMErrorString (hr);
				lNumberOfDataPoints = 0;
			} else {
				lNumberOfDataPoints = m_ipFMANChromData->GetNumberOfDataPoints ();
			}

			long lNumberOfTransitions = 0;
			bool fIsMRMExperiments = false;

			for (long lExperimentIndex=0; lExperimentIndex<lActualNumberOfExperiments; lExperimentIndex++) {
				ExperimentInfo &experiment = period.getExperiment (lExperimentIndex);
				try {
					IUnknownPtr ipUnknown(m_ipFMANWiffFile->GetExperimentObject(lSampleId, lPeriodIndex, lExperimentIndex));
					IExperimentPtr ipExperiment;
					ipUnknown->QueryInterface(&ipExperiment);

					long lUseMCAScans = ipExperiment->UseMCAScans;
					experiment.setInfo(ipExperiment->ScanType, ipExperiment->Polarity, lUseMCAScans);
					experiment.init (lNumberOfDataPoints);

					if (isMRMScan(ipExperiment->ScanType)) {
						fIsMRMExperiments = true;
						lNumberOfTransitions = ipExperiment->GetMassRangesCount();
						experiment.init (ipExperiment->PauseBetweenMassRanges, lNumberOfTransitions, lNumberOfDataPoints);

						MRMExperimentInfo &mrmExperimentInfo = experiment.m_mrmExperimentInfo;
						for (long lTransitionIndex=0; lTransitionIndex<lNumberOfTransitions; lTransitionIndex++) {
							IUnknownPtr ipUnknown(ipExperiment->GetMassRange(lTransitionIndex));
							IMassRangePtr ipMassRange;
							ipUnknown->QueryInterface(&ipMassRange);
							double dCE = 0;
							if (ipMassRange->GetMassDepParamCount()>0) {
								IUnknownPtr ipUnknown(ipMassRange->GetMassDepParamTbl());
								IParamDataCollPtr ipPDC;
								ipUnknown->QueryInterface(&ipPDC);
								short sIdx=0;
								IUnknownPtr ipUnknownParameter(ipPDC->FindParameter(bstrParamID, &sIdx));
								IParameterDataPtr ipPD;
								ipUnknownParameter->QueryInterface(&ipPD);
								dCE = ipPD->GetstartVal();
							}
							mrmExperimentInfo.cacheTransitionInfo (
								lTransitionIndex+1, 
								ipMassRange->GetDwellTime(), 
								ipMassRange->GetQstepMass(), ipMassRange->GetQstartMass(),
								dCE);
						}
					}

					if (0!=lUseMCAScans) {
						fMCAData = true;
					}

				} catch (_com_error &com_error) {
					_VARIABLE_UNUSED(com_error);
					fToWarn = true;
				}
			}

			m_sampleScanMap.initNewSample (lPeriodIndex, lActualNumberOfExperiments, lNumberOfDataPoints, totalNumScans_);

			for (long lExperimentIndex=0; lExperimentIndex<lActualNumberOfExperiments; lExperimentIndex++) {
				ExperimentInfo &experiment = period.getExperiment (lExperimentIndex);

				HRESULT hr = m_ipFMANChromData->raw_SetToTIC(lSampleId, lPeriodIndex, lExperimentIndex);
				if (FAILED(hr)) {
					std::cerr << "Error: Chromatogram.TIC("<<lSampleId<<", "<<lPeriodIndex<<", "<<lExperimentIndex<<"), COM error " 
						<< std::hex << hr << std::dec << std::endl;
					printCOMErrorString(hr);
				}

				if (m_iVerbose>=VERBOSE_INFO) {
					std::cout << "INFO:       Experiment#" << lExperimentIndex << ":"
						<< " Scan(" 
						<< toString(experiment.m_msScan).c_str()
						<< "/" 
						<< getPolarityString(experiment.m_msPolarity)
						<< ")";
				}

				long lScansCount = 0;

				double dPauseBetweenMassRanges = 0;
				std::vector<double> transitionsDwellTime;
				if (MRM == experiment.m_msScan) {
					// MRM readings are processed differently
					ChromatogramMeasurements &chromatogramMeasurements = experiment.m_chromatogramMeasurements;
					MRMExperimentInfo &mrmExperimentInfo = experiment.m_mrmExperimentInfo;

					fToMergeScan = false;

					double dXTime, dYInts;
					for (long dpi=1; dpi<=lNumberOfDataPoints; dpi++) {
						hr = m_ipFMANChromData->raw_GetDataPoint(dpi, &dXTime, &dYInts);
						chromatogramMeasurements.cacheMeasurement (dpi, dXTime, dYInts);
						if (dYInts>0) {
							// MRM scan count determination
							double dPhonyRetention = (dXTime) * 60;
							float fxValue = (float) dPhonyRetention;
							HRESULT hr = m_ipFMANSpecData->raw_SetSpectrum(lSampleId, lPeriodIndex, lExperimentIndex, fxValue, fxValue);
							if (FAILED(hr)) {
								// best estimate possible
								lScansCount++;
								continue;
							}

							mrmExperimentInfo.cacheMeasurementStart (dpi);
							MRMExperimentInfo::MRMTransitionSettings &mrmTransitionSettings = mrmExperimentInfo.m_MRMTransitionSettings;

							long lSpecDataPoints = m_ipFMANSpecData->GetNumberOfDataPoints();
							double dYVals;
							if (dMRMThresholdMin>0) {
								if (dMRMThresholdMax>0) {
									// both lower (Min) and upper (Max) bound are set
									// thresholding is active, check how many clear the threshold
									for (long transId=1; transId<=lSpecDataPoints; transId++) {
										HRESULT hr = m_ipFMANSpecData->raw_GetDataPointYValue(transId, &dYVals);
										if (dYVals>0) {
											dPhonyRetention += ((mrmTransitionSettings[transId].m_dDwellTime+mrmExperimentInfo.m_pauseBetweenMassRanges)/1000.0);
											mrmExperimentInfo.cacheTransitionMeasurement (
												dpi, transId, dPhonyRetention, dYVals);

											if (dYVals<dMRMThresholdMin || dYVals>dMRMThresholdMax) {
											} else {
												lScansCount++;
											}
										}
									}
								} else {
									// only a lower bound (Min) is set
									// thresholding is active, check how many clear the threshold
									for (long transId=1; transId<=lSpecDataPoints; transId++) {
										HRESULT hr = m_ipFMANSpecData->raw_GetDataPointYValue(transId, &dYVals);
										if (dYVals>0) {
											dPhonyRetention += ((mrmTransitionSettings[transId].m_dDwellTime+mrmExperimentInfo.m_pauseBetweenMassRanges)/1000.0);
											mrmExperimentInfo.cacheTransitionMeasurement (
												dpi, transId, dPhonyRetention, dYVals);

											if (dYVals<dMRMThresholdMin) {
											} else {
												lScansCount++;
											}
										}
									}
								}
							} else if (dMRMThresholdMax>0) {
								// only an upper bound (Max) is set
								// thresholding is active, check how many clear the threshold
								for (long transId=1; transId<=lSpecDataPoints; transId++) {
									HRESULT hr = m_ipFMANSpecData->raw_GetDataPointYValue(transId, &dYVals);
									if (dYVals>0) {
										dPhonyRetention += ((mrmTransitionSettings[transId].m_dDwellTime+mrmExperimentInfo.m_pauseBetweenMassRanges)/1000.0);
										mrmExperimentInfo.cacheTransitionMeasurement (
											dpi, transId, dPhonyRetention, dYVals);

										if (dYVals>dMRMThresholdMax) {
										} else {
											lScansCount++;
										}
									}
								}
							} else {
								for (long transId=1; transId<=lSpecDataPoints; transId++) {
									HRESULT hr = m_ipFMANSpecData->raw_GetDataPointYValue(transId, &dYVals);
									if (dYVals>0) {
										lScansCount++;

										dPhonyRetention += ((mrmTransitionSettings[transId].m_dDwellTime+mrmExperimentInfo.m_pauseBetweenMassRanges)/1000.0);
										mrmExperimentInfo.cacheTransitionMeasurement (
											dpi, transId, dPhonyRetention, dYVals);
									}
								}
							}
							mrmExperimentInfo.cacheMeasurementEnd (dpi);
						}
					} // dpi

				} else {

					// non MRM reading

					double xValue, yValue;
					ChromatogramMeasurements &chromatogramMeasurements = experiment.m_chromatogramMeasurements;
					for (long lDataPointsIndex=1; lDataPointsIndex<=lNumberOfDataPoints; lDataPointsIndex++) {
						HRESULT hr = m_ipFMANChromData->raw_GetDataPoint(lDataPointsIndex, &xValue, &yValue);
						chromatogramMeasurements.cacheMeasurement (lDataPointsIndex, xValue, yValue);
						if (yValue>0) {
							// non-MRM scan count determination
							m_sampleScanMap.cacheSampleScanNumber (lExperimentIndex, lDataPointsIndex);
							lScansCount++;
						}
					}
				}

				if (m_iVerbose>=VERBOSE_INFO) {
					std::cout << ", " << lScansCount << "/" << lNumberOfDataPoints;
					if (0!=experiment.m_nUseMCAScans) {
						std::cout << "-(MCA)->1/1";
					}
					std::cout << std::endl;
				}

				totalNumScans_ += lScansCount;
			} // ExperimentIndex

			m_sampleScanMap.sampleCached(lSampleId, lPeriodIndex);
			m_sampleScanMap.reorderScanNumbers();

			// compute the merged scans for all experiments
			// in the same (SampleIndex, PeriodIndex)
			if (fToMergeScan && m_DPSettings.m_dGroupPrecursorMassTolerance>0) {
				if (m_iVerbose>=VERBOSE_INFO) {
					std::cout << "INFO:       Merged scan: searching for candidates...";
				}
				long lMergedScanCount=0, lTotalScansConsidered=0;
				MergedScanTable mergedScans;
				mergedScans.initNewSample(lPeriodIndex);
				mergeScans(lPeriodIndex, m_sampleScanMap, 
					lMergedScanCount, lTotalScansConsidered, mergedScans);
				if (lMergedScanCount>0) {
					m_sampleMergedScansCache[mergedScans.getKey()] = mergedScans;
				}
				if (m_iVerbose>=VERBOSE_INFO) {
					std::cout << " " << lMergedScanCount << "/" << lTotalScansConsidered << std::endl;
				}
				totalNumScans_ += lMergedScanCount;
			}

				// TODO: for debugging
			//period.dump();

		} // PeriodIndex

		m_sampleInfo.getTimeBoundInSec (startTimeInSec_, endTimeInSec_);
		firstScanNumber_ = 1;
		lastScanNumber_ = totalNumScans_;

		if (m_iVerbose>=VERBOSE_INFO) {
			std::cout << "INFO: Allocated " << totalNumScans_ << " scan slot(s) via full sweep." << std::endl;
		}

		// perform a prefetch so that we can tell where we will get Precursor information
		if (totalNumScans_ > 0) {
			getSamplePrecursorInfo (lSampleId, 0);
		}
		if (m_iVerbose>=VERBOSE_INFO) {
			switch (m_sampleInfo.getType()) {
				case SampleInfo::TYPE_DDEEX:
					std::cout << "INFO: Using Precursor info from DDEEX data stream." << std::endl;
					break;
				case SampleInfo::TYPE_DDE:
					std::cout << "INFO: Using Precursor info from DDE data stream." << std::endl;
					break;
				case SampleInfo::TYPE_NONE:
				default:
					break;
			}
		}

		if (fToWarn) {
			std::cerr << "WARNING: Your installed library has issue accessing experiment information." << std::endl;
			std::cerr << "WARNING: Translation is unlikely to complete successfully." << std::endl;
			std::cerr << "WARNING: Please check that you have the right version of the Analyst software installed." << std::endl;
		}

		if (fMCAData) {
			std::cerr << "WARNING: Each spectra of MACs is reported separately." << std::endl;
			std::cerr << "WARNING: The summing of MACs is not supported currently." << std::endl;
		}

#ifndef NODEBUGDETAIL
		// Sanity check!
		if (m_iVerbose>=VERBOSE_DEBUGDETAIL) {
			for (long lPeriodIndex=0; lPeriodIndex<m_sampleInfo.getNumberOfPeriods(); lPeriodIndex++) {
				PeriodInfo &period = m_sampleInfo.getPeriod(lPeriodIndex);
				std::cout << "DEBUG: Period#" << (lPeriodIndex+1) << std::endl;
				period.dump();
			}
		}
#endif
	}
}

//======================================================================
AQS20InstrumentInfoLoader::AQS20InstrumentInfoLoader(AnalystQS20::IFMANWiffFile *pIFMANWiffFile, int iVerbose)
:	m_pIFMANWiffFile(pIFMANWiffFile),
	m_iVerbose(iVerbose),
	m_lSampleId(0),
	m_lModelId(0)
{
}

AQS20InstrumentInfoLoader::~AQS20InstrumentInfoLoader(void)
{
}

bool AQS20InstrumentInfoLoader::load(long lSampleId)
{
	if (m_lSampleId != lSampleId) {
		m_lModelId = m_lSampleId = 0;
		m_strLog.clear();
		if (getSampleLog(lSampleId) && getMSModel(lSampleId)) {
			m_lSampleId = lSampleId;
			return true;
		}
	}

	return true;
}

bool AQS20InstrumentInfoLoader::getSampleLog(long lSampleId)
{
	IUnknownPtr ipUnknownFileManagerObject(m_pIFMANWiffFile->GetFileManagerObject());

	IFMProcessingPtr ipIFMProcessing;
	HRESULT hr = ipUnknownFileManagerObject->QueryInterface(&ipIFMProcessing);

	long lKeyTypeIndex = 0;	// value 10 is taken based on trace
	short sSampleId = (short) lSampleId; // FIXME: ABI uses short here!
	hr = ipIFMProcessing->FMProcEnumLogInit(sSampleId, lKeyTypeIndex);

	LPWSTR lpwstr=NULL;
	do
	{
		lpwstr=NULL;
		long lKeyType=0, lTimeOff=0;
		hr = ipIFMProcessing->FMProcEnumLogNext(sSampleId, &lpwstr, &lKeyType, &lTimeOff);

		if (NULL != lpwstr) {
			USES_CONVERSION;
			std::string strLogs = W2A(lpwstr);
			::SysFreeString(lpwstr);

			if (m_iVerbose>=VERBOSE_DEBUG) {
				std::cout << "DEBUG: Sample log" << std::endl;
				std::cout << "DEBUG: " << strLogs.c_str() << std::endl;
			}

			// ASSUMPTION: keep first log with "Mass Spectrometer:"
			if (m_strLog.empty()) {
				const std::string prefix("Mass Spectrometer:");
				if (strLogs.npos != strLogs.find(prefix)) {
					m_strLog = strLogs;
					if (m_iVerbose<VERBOSE_DEBUG) {
						break;
					}
				}
			}
		}
	} while (lpwstr!=NULL);
	hr = ipIFMProcessing->FMProcEnumLogReset(sSampleId);

	return true;
}

bool AQS20InstrumentInfoLoader::getMSModel(long lSampleId)
{
	try {
		IUnknownPtr ipUnknown(m_pIFMANWiffFile->GetMassSpecMethod(lSampleId));
		IMassSpecMethodPtr ipMassSpecMethod;
		HRESULT hr = ipUnknown->QueryInterface(&ipMassSpecMethod);
		long numQuads;
		BSTR configVer=NULL;
		ipMassSpecMethod->GetMassSpecConfigInfo(&numQuads, &m_lModelId, &configVer);
		if (NULL!=configVer) {
			SysFreeString(configVer);
		}
		if (m_iVerbose>=VERBOSE_DEBUG) {
			std::cout << "DEBUG: MassSpecConfigInfo(" << numQuads << ", " << m_lModelId << ")";
		}
	} catch (_com_error &com_error) {
		std::cerr << "WARNING: Failed to retrieve Mass Spectrometry Method info. COM error " << std::hex << com_error.Error() << std::dec << std::endl;
		printCOMErrorString(com_error.Error());
	}

	return true;
}

//======================================================================

AQS20SoftwareInfoLoader::AQS20SoftwareInfoLoader(const std::string &strFilename, int iVerbose)
:	m_strFilename(strFilename),
	m_iVerbose(iVerbose)
{
}

AQS20SoftwareInfoLoader::~AQS20SoftwareInfoLoader(void)
{
}


bool AQS20SoftwareInfoLoader::load(void)
{
	m_strCreator.clear();

	IFMANWiffFilePtr ipFMANWiffFile;
	HRESULT hr = ipFMANWiffFile.CreateInstance(__uuidof(FMANWiffFile));
	if (FAILED(hr)) {
		LPOLESTR lpolestrCLSID;
		StringFromCLSID(__uuidof(FMANWiffFile), &lpolestrCLSID);
		CW2A pszACLSID( lpolestrCLSID );
		std::cerr << "Could not create FMANWiffFile" << pszACLSID << " instance, hr("
			<< std::hex << hr << std::dec <<")" << std::endl;
		CoTaskMemFree(lpolestrCLSID);
		return false;
	}

	_bstr_t wiffFileName(m_strFilename.c_str());
	hr = ipFMANWiffFile->OpenWiffFile(wiffFileName);
	if (FAILED(hr)) {
		std::cerr << "ipFMANWiffFile->OpenWiffFile(" << m_strFilename.c_str() << ") returns " 
			<< std::hex << hr << std::dec << std::endl;
		return false;
	}

	bool fStatus = false;

	IUnknownPtr ipUnknownFileManagerObject;
	ipUnknownFileManagerObject = ipFMANWiffFile->GetFileManagerObject();

	IFMWIFFPtr ipIFMWIFF;
	hr = ipUnknownFileManagerObject->QueryInterface(&ipIFMWIFF);
	if (SUCCEEDED(hr)) {
		unsigned long dwFileHandle=0;
		hr = ipIFMWIFF->FMANGetWIFFDriverFileHandle(&dwFileHandle);
		if (SUCCEEDED(hr)) {
			IUnknownPtr ipUnknownWTS;
			hr = ipIFMWIFF->FMANWIFFGetWTS (&ipUnknownWTS);
			if (SUCCEEDED(hr)) {
				IWIFFTransServerPtr ipIWIFFTransServer(ipUnknownWTS);

				void *pObject=NULL;
				hr = ipIWIFFTransServer->QueryInterface(__uuidof(IWIFFTSvrFileRec_Str), &pObject);
				if (SUCCEEDED(hr)) {
					IWIFFTSvrFileRec_StrPtr ipIWIFFTSvrFileRec_Str((IWIFFTSvrFileRec_Str*)pObject);

					long lStreamDataObject=0;
					hr = ipIWIFFTSvrFileRec_Str->OpenArrStrCLS_FileRec_Str(dwFileHandle, &lStreamDataObject);
					if (SUCCEEDED(hr)) {

						LPWSTR lpwstr=NULL;
						hr = ipIWIFFTSvrFileRec_Str->raw_GetFieldCLS_csSignature(dwFileHandle, &lpwstr);
						if (SUCCEEDED(hr)) {
							USES_CONVERSION;
							m_strCreator = W2A(lpwstr);
							fStatus = true;
						} else {
							std::cerr << "IWIFFTSvrFileRec_Str->raw_GetFieldCLS_csSignature() returns " 
								<< std::hex << hr << std::dec << std::endl;
						}
						::CoTaskMemFree(lpwstr);

						hr = ipIWIFFTSvrFileRec_Str->CloseArrStrCLS_FileRec_Str(lStreamDataObject);
						if (FAILED(hr)) {
							std::cerr << "IWIFFTSvrFileRec_Str->CloseArrStrCLS_FileRec_Str() returns " 
								<< std::hex << hr << std::dec << std::endl;
						}
					} else {
						std::cerr << "IWIFFTSvrFileRec_Str->OpenArrStrCLS_FileRec_Str() returns " 
							<< std::hex << hr << std::dec << std::endl;
					}
				}
			} else {
				std::cerr << "IFMWIFF->FMANWIFFGetWTS() returns " 
					<< std::hex << hr << std::dec << std::endl;
			}
		} else {
			std::cerr << "IFMWIFF->FMANGetWIFFDriverFileHandle() returns " 
				<< std::hex << hr << std::dec << std::endl;
		}
	}

	hr = ipFMANWiffFile->CloseWiffFile();
	if (FAILED(hr)) {
		std::cerr << "ipFMANWiffFile->CloseWiffFile(" << m_strFilename.c_str() << ") returns " 
			<< std::hex << hr << std::dec << std::endl;
	}

	return fStatus;
}

//======================================================================
