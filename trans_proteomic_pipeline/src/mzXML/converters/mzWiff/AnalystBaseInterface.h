// -*- mode: c++ -*-


/*
    File: AnalystBaseInterface.h
    Description: shared implementation across Analyst & AnalystQS interfaces
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


#pragma once

#include <string>
#include "mzXML/common/SHA1.h"
#include "mzXML/common/InstrumentInterface.h"
#include "AnalystImpl.h"

class MsgBoxCloser;
class AnalystBaseInterface : public InstrumentInterface {
protected:
	LibraryTagType m_enumLibraryType;
	CompatibilityType m_enumCompatibility;
	std::string m_InterfaceVersion;

	bool firstTime_;

	int m_iVerbose;
	bool m_fAssumeLibrary;

	bool m_fInitialized;
	bool m_fCOMInitialized;

	SHA1 sha1_; // md5sum calculator
	char sha1Report_[1024]; // sha1 hash

	//_bstr_t     m_bstrWiffFileName;

	MsgBoxCloser *m_pMsgBoxCloser;

	SampleTable            m_samples;
	SampleInfo             m_sampleInfo;

	ScanTable              m_sampleScanMap;
	MergedScanTableCache   m_sampleMergedScansCache;
	ScanOriginToMergeTable m_scanOriginToMergeTable;

	ScanIteratorState      m_iteratorState;

	double                 m_dChromotographRTInSec;
	bool                   m_fUseExperiment0RT;

	//TODO:temporarily, have to change the interface and logics
	Scan *m_pScan;

	PeaksCharge                        m_peaksCharge;

protected:
	virtual bool termInterface(void) = 0;

	//Scan* getFirstScan(void); // not sure?
	//Scan* getNextScan(void); // not sure?
	//bool isValidScan(void); // not sure?

	//void processSpectrum(Scan *pScan); // not sure?
	//void processMSSpectrum(Scan *pScan); // not sure?
	//void processMRMSpectrum(Scan *pScan); // not sure?
	//void processMSSpecData(Scan *pScan); // not sure?
	//void centroidSpecData(bool doDeisotope, double &maxIntensity); // not sure?

	//void getSamplePrecursorInfo (long lSampleIndex, long lPeriodIndex);
	//void getSamplePrecursorInfo (DDEData &ddeData, long lSampleIndex, long lPeriodIndex);
	//void mergeScans (long lSampleIndex, long lPeriodIndex, const ScanTable &scanTable, long &lMergedScanCount, long &lTotalScansConsidered, MergedScanTable &mergedScans);
	//ScanPtr instantiateMergedScans (long lSampleIndex, long lPeriodIndex, const MergedScan &mergedScan);
	//void loadMergedScans (long lSampleIndex, long lPeriodIndex);

protected:
	void startMsgBoxMonitor(void);
	void stopMsgBoxMonitor(void);
	inline bool computeWiffSHA1(const std::string& fileName)
	{
		sha1_.Reset();
		sha1Report_[0] = 0;
		if (0!=m_DPSettings.m_dGroupPrecursorMassTolerance) {
			if ( !(sha1_.HashFile(fileName.c_str()))) {
				std::cerr << "ERRORO: Cannot open file '" << fileName.c_str() << "' for sha-1 calculation" <<std::endl;
				return false;// Cannot open file for sha1
			}
			sha1_.Final();
			sha1_.ReportHash(sha1Report_, SHA1::REPORT_HEX);
		}
		return true;
	}

protected:
	virtual void loadSampleList(void) = 0;
	virtual void loadInstrumentInfo(long lSampleId) = 0;
	virtual void loadSampleInfo(long lSampleId) = 0;

public:
	AnalystBaseInterface(void);
	virtual ~AnalystBaseInterface(void);

	virtual bool initInterface(void) = 0;
	virtual bool setInputFile(const std::string& fileName) = 0;
	virtual void setCentroiding(bool centroid); // okay
	virtual void setDeisotoping(bool deisotope); // okay
	virtual void setCompression(bool compression); // okay
	virtual void setVerbose(bool verbose); // okay
	virtual void setShotgunFragmentation(bool sf) {}
	virtual void setLockspray(bool ls) {}
	virtual Scan* getScan(void) = 0;

public:
	MSDataProcessing m_DPSettings;

	void setVerbose(int iVerbose);
	void assumeLibrary(bool assume);
	size_t getNumberOfSamples(void) const;
	bool setSample(long lSampleId);
	CompatibilityType getCompatibilityTest(void) { return m_enumCompatibility; }
	const std::string &getInterfaceVersion(void) const { return m_InterfaceVersion; }

};

//======================================================================
