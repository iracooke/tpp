// -*- mode: c++ -*-


/*
    File: AnalystQS20Interface.h
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


#pragma once

#include <string>
#include "mzXML/common/SHA1.h"
#include "wiffqs20lib/analystqs20tlbs.h"
#include "AnalystBaseInterface.h"

class AQS20InstrumentInfoLoader
{
public:
	AQS20InstrumentInfoLoader(AnalystQS20::IFMANWiffFile *pIFMANWiffFile, int iVerbose);
	~AQS20InstrumentInfoLoader(void);
public:
	bool load(long lSampleId);
public:
	long m_lSampleId;
	long m_lModelId;
	std::string m_strLog;

private:
	bool getSampleLog(long lSampleId);
	bool getMSModel(long lSampleId);
private:
	AnalystQS20::IFMANWiffFile *m_pIFMANWiffFile;
	int m_iVerbose;
};

class AQS20SoftwareInfoLoader
{
public:
	AQS20SoftwareInfoLoader(const std::string &strFilename, int iVerbose);
	~AQS20SoftwareInfoLoader(void);
public:
	inline bool load(void);
public:
	std::string m_strCreator;
private:
	std::string m_strFilename;
	int m_iVerbose;
};

class AnalystQS20Interface : public AnalystBaseInterface {
private:
	_bstr_t     m_bstrWiffFileName;  //DECIDED: AnalystBaseInterface?

	AnalystQS20::IFMANChromDataPtr     m_ipFMANChromData;
	AnalystQS20::IFMANSpecDataPtr      m_ipFMANSpecData;
	AnalystQS20::IFMANWiffFilePtr      m_ipFMANWiffFile;
	AnalystQS20::IPeakFinderFactoryPtr m_ipPeakFinderFactory;
	AnalystQS20::IPeakFinder2Ptr       m_ipPeakFinder;
	BSTR                             m_paramsString;
	AnalystQS20::IFMANSpecDataPtr      m_ipFMANSpecDataToMerge;
	AnalystQS20::IFMANSpecDataPtr      m_ipFMANSpecDataCopy;

private:
	void initScanIteratorState (void);
	bool nextScanIteratorState (void);

private:
	Scan* getFirstScan(void);
	Scan* getNextScan(void);
	bool isValidScan(void);

	void processSpectrum(Scan *pScan);
	void processMSSpectrum(Scan *pScan);
	void processMRMSpectrum(Scan *pScan);
	void processMSSpecData(Scan *pScan);
	void centroidSpecData(bool doDeisotope, double &maxIntensity, bool copy=false, bool keepCharge=false);

	void getSamplePrecursorInfo (long lSampleIndex, long lPeriodIndex);
	void getSamplePrecursorInfoImpl (long lSampleIndex, long lPeriodIndex);
	void mergeScans (long lPeriodIndex, const ScanTable &scanTable, long &lMergedScanCount, long &lTotalScansConsidered, MergedScanTable &mergedScans);
	ScanPtr instantiateMergedScans (long lPeriodIndex, const MergedScan &mergedScan);
	void loadMergedScans (long lPeriodIndex);

private:
	void initInfoLoader(void);
	void termInfoLoader(void);
private:
	AQS20InstrumentInfoLoader *m_pInstrumentInfoLoader;
	InstrumentInfoParser    *m_pInstrumentInfoParser;

public:
	AnalystQS20Interface(void);
	virtual ~AnalystQS20Interface(void);

	virtual bool initInterface(void);
	virtual bool setInputFile(const std::string& fileName);
	virtual Scan* getScan(void);

protected:
	virtual bool termInterface(void);
	virtual void loadSampleList(void);
	virtual void loadInstrumentInfo(long lSampleId);
	virtual void loadSampleInfo(long lSampleId);

};
