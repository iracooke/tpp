// -*- mode: c++ -*-


/*
    File: AnalystImpl.h
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
#include <math.h>
#include "mzXML/common/MSTypes.h"

#ifndef _VARIABLE_UNUSED
#define _VARIABLE_UNUSED(x) (void)x
#endif

enum LibraryTagType
{
	LIBRARY_Analyst = 0,
	LIBRARY_AnalystQS = 1
};

enum CompatibilityType
{
	COMPATIBILITY_OK = 0,
	COMPATIBILITY_Expect_Analyst = -1,
	COMPATIBILITY_Expect_AnalystQS = -2,
	COMPATIBILITY_Expect_Newer_Version = -3,
	COMPATIBILITY_Expect_Unknown = -4,
	COMPATIBILITY_No_File_Signature = -5,
	COMPATIBILITY_No_Library_Signature = -6,
	COMPATIBILITY_No_Library_Available = -7
};

enum VerboseModeType
{
	VERBOSE_NONE = 0,
	VERBOSE_INFO = 1,       // provide informational log
	VERBOSE_DEBUG = 2,      // provide debug logs
	VERBOSE_DEBUGDETAIL = 3 // provide full detail debug log
};

bool checkPath(const std::string &pathname);
void printCOMErrorString (HRESULT hr);
int getLibraryVersion (enum LibraryTagType whichLibrary, std::string &strVersion);
const char *getLibraryName (enum LibraryTagType whichLibrary);
CompatibilityType isCompatible (enum LibraryTagType whichLibrary, std::string &strSignature);
void getSoftwareVersion (const std::string &strSignature, std::string &strSoftware, std::string &strVersion);
MSAcquisitionSoftwareType softwareToMSAcquisitionSoftwareType(std::string &strSoftware);
int getModelById (long lId, std::string &strModel);
MSInstrumentModelType modelIdToMSInstrumentModel(long lId);
void getSampleLogEntry(const std::string &strSampleLog, const std::string &strKey, std::string &strValue);

int getMSLevel(long lScanType);
const char *getPolarityString(long lPolarity);
const char *getPolarityString(MSPolarityType msPolarity);
const char *getScanTypeString(long lScanType);
MSScanType getMSScanType(long lScanType);

class AnalystBaseInterface;
AnalystBaseInterface *getAnalystInterface(bool fVerbose);
AnalystBaseInterface *getAnalystQSInterface(bool fVerbose);

inline bool isMRMScan(long lScanType)
{
	return (4 == lScanType);
}
inline bool isMRMScan(MSScanType scanType)
{
	return (MRM == scanType);
}

inline bool isMSScanTypeMergable(MSScanType msScanType)
{
	//TODO: what else can we merged?
	return (TOFMS2==msScanType || EPI==msScanType);
}

typedef long SamplePeriodKey;

#include <vector>
#include <sstream>

class SampleTable {
public:
	enum SampleStateType
	{
		STATE_OKAY = 0,
		STATE_ERROR_NAME = 1,
		STATE_ERROR_PERIOD = 2
	};
public:
	std::vector<std::string> m_sampleNames;
	std::vector<std::string> m_sampleIdNames;
	std::vector<SampleStateType> m_sampleStates;  // 0 implies okay

public:
	SampleTable() { init (); }
	inline void init()
	{
		m_sampleNames.clear();
		m_sampleNames.push_back(""); // index starts with 1, so add an empty 0th element
		m_sampleIdNames.clear();
		m_sampleIdNames.push_back(""); // index starts with 1, so add an empty 0th element
		m_sampleStates.clear();
		m_sampleStates.push_back(STATE_OKAY); // index starts with 1, so add an empty 0th element
	}
	inline void addSample(const char *name, SampleStateType nStatus)
	{
		std::string strName(name);
		m_sampleNames.push_back(strName);

		strName = "(";
		{
			std::ostringstream converter;
			converter << m_sampleNames.size();
			strName += converter.str();
		}
		strName.append(") ").append(name);
		m_sampleIdNames.push_back(strName);

		m_sampleStates.push_back(nStatus);
	}
	inline long getNumberOfSamples() const
	{
		return (long)m_sampleNames.size()-1;
	}
	inline std::string getSampleName(int i) const
	{
		return m_sampleNames[i];
	}
	inline std::string getSampleIdName(int i) const
	{
		return m_sampleNames[i];
	}
	inline SampleStateType getSampleState(int i) const
	{
		return m_sampleStates[i];
	}
	inline bool isStateOK(int i) const
	{
		return (STATE_OKAY == m_sampleStates[i]);
	}
};

class MRMTransitionSetting {
public:
	double m_dDwellTime;
	double m_dQstepMass;
	double m_dQstartMass;
	double m_dCE;
public:
	MRMTransitionSetting()
		:	m_dDwellTime(0), m_dQstepMass(0), m_dQstartMass(0), m_dCE(0)
	{
	}
	MRMTransitionSetting(double dDwellTime, double dQstepMass, double dQstartMass, double dCE)
		:	m_dDwellTime(dDwellTime), 
			m_dQstepMass(dQstepMass), 
			m_dQstartMass(dQstartMass), 
			m_dCE(dCE)
	{
	}
	inline void getInfo(double &dDwellTime, double &dQstepMass, double &dQstartMass, double &dCE) const
	{
		dDwellTime  = m_dDwellTime;
		dQstepMass  = m_dQstepMass;
		dQstartMass = m_dQstartMass;
		dCE         = m_dCE;
	}
	inline void dump(void) const
	{
		std::cout << "MRMTransitionSetting(";
		std::cout << m_dDwellTime;
		std::cout << ", " << m_dQstepMass;
		std::cout << ", " << m_dQstartMass;
		std::cout << ", " << m_dCE;
		std::cout << ")";
		std::cout << std::endl;
	}
};

class MRMTransitionReadout {
public:
	long m_lTransitionIndex;
	double m_dRT;
	double m_dIntensity;
public:
	MRMTransitionReadout()
		:	m_lTransitionIndex(0), m_dRT(0), m_dIntensity(0)
	{
	}
	MRMTransitionReadout(long lTransitionIndex, double dRT, double dIntensity)
		:	m_lTransitionIndex(lTransitionIndex), m_dRT(dRT), m_dIntensity(dIntensity)
	{
	}
	inline void getReadout(long &lTransitionIndex, double &dRT, double &dIntensity) const
	{
		lTransitionIndex = m_lTransitionIndex;
		dRT              = m_dRT;
		dIntensity       = m_dIntensity;
	}
};

class MRMTransitionMeasurements {
	typedef std::vector<MRMTransitionReadout>  MRMMeasurements;
public:
	MRMMeasurements m_measurements;
public:
	MRMTransitionMeasurements()
	{
	}
	inline void newMeasurements(long lNumberOfTransitions)
	{
		m_measurements.resize (0);  // clear any possible old entries
		m_measurements.reserve (lNumberOfTransitions);  // pre-allocate
	}
	inline void cacheMeasurement(long lTransitionIndex, double dTime, double dIntensity)
	{
		m_measurements.push_back(
			MRMTransitionReadout(lTransitionIndex, dTime, dIntensity));
	}
	inline void endMeasurements()
	{
		//m_measurements.resize (m_measurements.size ());
	}
	inline long size() const
	{
		return (long)m_measurements.size();
	}
	inline bool isEmpty() const
	{
		return m_measurements.empty();
	}
	inline void getMeasurements(long index, long &lTransitionIndex, double &dTime, double &dIntensity) const
	{
		m_measurements[index].getReadout (lTransitionIndex, dTime, dIntensity);
	}
};

class MRMExperimentInfo {
	typedef std::vector<MRMTransitionMeasurements> MRMTransitionScans;
	typedef std::vector<long>                      MRMReadIndex;
public:
	typedef std::vector<MRMTransitionSetting>      MRMTransitionSettings;

public:
	double                 m_pauseBetweenMassRanges;
	MRMTransitionSettings  m_MRMTransitionSettings;
	MRMTransitionScans     m_MRMReadings;
	MRMReadIndex           m_MRMReadIndexes;

public:
	MRMExperimentInfo()
		:	m_pauseBetweenMassRanges(0)
	{
	}
	inline void init(double dPauseBetweenMassRanges, long lNumTransitions, long lNumDataPoints)
	{
		m_pauseBetweenMassRanges = dPauseBetweenMassRanges;
		m_MRMTransitionSettings.resize(lNumTransitions+1);
		m_MRMReadings.clear();
		m_MRMReadings.resize(lNumDataPoints+1);
		m_MRMReadIndexes.resize(lNumDataPoints+1, 0);
	}
	inline long getNumberOfTransitions() const
	{
		return (long)m_MRMTransitionSettings.size();
	}
	inline long getNumberOfDataPoints() const
	{
		return (long)m_MRMReadIndexes.size();
	}
	inline void cacheTransitionInfo(long lTransitionIndex, double dDwellTime, double dQstepMass, double dQstartMass, double dCE)
	{
		m_MRMTransitionSettings[lTransitionIndex] = MRMTransitionSetting(dDwellTime, dQstepMass, dQstartMass, dCE);
	}
	inline void cacheMeasurementStart(long lDataPointIndex)
	{
	}
	inline void cacheTransitionMeasurement(long lDataPointIndex, long lTransitionIndex, double dTime, double dIntensity)
	{
		m_MRMReadings[lDataPointIndex].cacheMeasurement(lTransitionIndex, dTime, dIntensity);
	}
	inline void cacheMeasurementEnd(long lDataPointIndex)
	{
		if (!m_MRMReadings[lDataPointIndex].isEmpty()) {
			m_MRMReadIndexes[lDataPointIndex] = lDataPointIndex;
		}
	}
	inline void getPauseBetweenMassRanges(double &dPauseBetweenMassRanges) const
	{
		dPauseBetweenMassRanges = m_pauseBetweenMassRanges;
	}
	inline void getTransitionInfo(long lTransitionIndex, double &dDwellTime, double &dQstepMass, double &dQstartMass, double &dCE) const
	{
		m_MRMTransitionSettings[lTransitionIndex].getInfo(dDwellTime, dQstepMass, dQstartMass, dCE);
	}
	inline long getTransitionMeasurementStart(long lDataPointIndex) const
	{
		return m_MRMReadings[lDataPointIndex].size();
	}
	inline void getTransitionMeasurement(long lDataPointIndex, long lMeasureIndex, long &lTransitionIndex, double &dTime, double &dIntensity) const
	{
		m_MRMReadings[lDataPointIndex].getMeasurements(lMeasureIndex, lTransitionIndex, dTime, dIntensity);
	}
	inline void dump(void) {
		std::cout << "MRM Settings" << std::endl;
		for (long tranid=1; tranid<getNumberOfTransitions(); tranid++) {
			std::cout << tranid;
			std::cout << ": " << m_MRMTransitionSettings[tranid].m_dDwellTime;
			std::cout << ", " << m_MRMTransitionSettings[tranid].m_dQstartMass;
			std::cout << ", " << m_MRMTransitionSettings[tranid].m_dQstepMass;
			std::cout << ", " << m_MRMTransitionSettings[tranid].m_dCE;
			std::cout << std::endl;
		}
		std::cout << "MRM Readings" << std::endl;
		long lTransitionIndex;
		double dTime, dIntensity;
		for (long dpi=1; dpi<getNumberOfDataPoints(); dpi++) {
			std::cout << dpi << ": ";
			long numberOfTransitions = getTransitionMeasurementStart(dpi);
			for (long tranid=0; tranid<numberOfTransitions; tranid++) {
				getTransitionMeasurement(dpi, tranid, lTransitionIndex, dTime, dIntensity);
				std::cout << ((0==tranid) ? "" : "; ") << lTransitionIndex;
				std::cout << ", " << dTime;
				std::cout << ", " << dIntensity;
			}
			std::cout << std::endl;
		}
	}
};

class ChromatogramReadout {
public:
	double m_dRT;
	double m_dIntensity;
public:
	ChromatogramReadout()
		:	m_dRT(0), m_dIntensity(0)
	{
	}
	ChromatogramReadout(double dRT, double dIntensity)
		:	m_dRT(dRT), m_dIntensity(dIntensity)
	{
	}
	inline void getReadout(double &dRT, double &dIntensity) const
	{
		dRT        = m_dRT;
		dIntensity = m_dIntensity;
	}
	inline void setReadout(double dRT, double dIntensity)
	{
		m_dRT        = dRT;
		m_dIntensity = dIntensity;
	}
};

class ChromatogramMeasurements {
	typedef std::vector<ChromatogramReadout>  Measurements;
public:
	Measurements m_measurements;
public:
	ChromatogramMeasurements()
	{
	}
	inline void newMeasurements(long lNumberOfMeasurements)
	{
		m_measurements.resize (lNumberOfMeasurements+1);
	}
	inline void cacheMeasurement(long index, double dTime, double dIntensity)
	{
		m_measurements[index].m_dRT = dTime;
		m_measurements[index].m_dIntensity = dIntensity;
	}
	inline void endMeasurements()
	{
		//m_measurements.resize (m_measurements.size ());
	}
	inline long size() const
	{
		return (long)m_measurements.size() - 1;
	}
	inline void getMeasurements(long index, double &dTime, double &dIntensity) const
	{
		m_measurements[index].getReadout (dTime, dIntensity);
	}
	inline double getMeasurementXFirst(void) const
	{
		return m_measurements[1].m_dRT;
	}
	inline double getMeasurementXLast(void) const
	{
		return m_measurements[size()].m_dRT;
	}
	inline double getMeasurementX(long index) const
	{
		return m_measurements[index].m_dRT;
	}
	inline double getMeasurementY(long index) const
	{
		return m_measurements[index].m_dIntensity;
	}
	inline void dump(void) const
	{
		std::cout << "Chromatogram Info" << std::endl;
		for (long index=1; index<size(); index++) {
			std::cout << index;
			std::cout << ":" << getMeasurementX(index);
			std::cout << "," << getMeasurementY(index);
			std::cout << std::endl;
		}
	}
};

class ChromatogramAuxReadout {
public:
	double m_dPrecursorMZ;
	double m_dPrecursorIntensity;
	int    m_nPrecursorCharge;
	double m_dPrecursorCE;
public:
	ChromatogramAuxReadout()
		:	m_dPrecursorMZ(0), m_dPrecursorIntensity(0), m_nPrecursorCharge(0), m_dPrecursorCE(0)
	{
	}
	ChromatogramAuxReadout(double dPrecursorMZ, double dPrecursorIntensity, int nPrecursorCharge, double dPrecursorCE)
		:	m_dPrecursorMZ(dPrecursorMZ), m_dPrecursorIntensity(dPrecursorIntensity), m_nPrecursorCharge(nPrecursorCharge), m_dPrecursorCE(dPrecursorCE)
	{
	}
	inline void setPrecursorInfo(double dPrecursorMZ, double dPrecursorIntensity)
	{
		m_dPrecursorMZ = dPrecursorMZ;
		m_dPrecursorIntensity = dPrecursorIntensity;
	}
	inline void setPrecursorInfoEx(int nPrecursorCharge, double dPrecursorCE)
	{
		m_nPrecursorCharge = nPrecursorCharge;
		m_dPrecursorCE = dPrecursorCE;
	}
	inline void setPrecursorInfoExAll(double dPrecursorMZ, double dPrecursorIntensity, int nPrecursorCharge, double dPrecursorCE)
	{
		setPrecursorInfo(dPrecursorMZ, dPrecursorIntensity);
		setPrecursorInfoEx(nPrecursorCharge, dPrecursorCE);
	}
	inline void getPrecursorInfo(double &dPrecursorMZ, double &dPrecursorIntensity) const
	{
		dPrecursorMZ = m_dPrecursorMZ;
		dPrecursorIntensity = m_dPrecursorIntensity;
	}
	inline void getPrecursorInfoEx(int &nPrecursorCharge, double &dPrecursorCE) const
	{
		nPrecursorCharge = m_nPrecursorCharge;
		dPrecursorCE = m_dPrecursorCE;
	}
	inline void getPrecursorMZCharge(double &dPrecursorMZ, double &dPrecursorIntensity, int &nPrecursorCharge) const
	{
		dPrecursorMZ = m_dPrecursorMZ;
		dPrecursorIntensity = m_dPrecursorIntensity;
		nPrecursorCharge = m_nPrecursorCharge;
	}
};

class ChromatogramAuxMeasurements {
	typedef std::vector<ChromatogramAuxReadout>  Measurements;
public:
	Measurements m_measurements;
public:
	ChromatogramAuxMeasurements()
	{
	}
	inline void newAuxMeasurements(long lNumberOfMeasurements)
	{
		m_measurements.resize (lNumberOfMeasurements+1);
	}
	inline void cacheAuxMeasurement(long index, double dPrecursorMZ, double dPrecursorIntensity)
	{
		m_measurements[index].setPrecursorInfo(dPrecursorMZ, dPrecursorIntensity);
	}
	inline void cacheAuxMeasurementEx(long index, int nPrecursorCharge, double dPrecursorCE)
	{
		m_measurements[index].setPrecursorInfoEx(nPrecursorCharge, dPrecursorCE);
	}
	inline void cacheAuxMeasurementExAll(long index, double dPrecursorMZ, double dPrecursorIntensity, int nPrecursorCharge, double dPrecursorCE)
	{
		m_measurements[index].setPrecursorInfoExAll(dPrecursorMZ, dPrecursorIntensity, nPrecursorCharge, dPrecursorCE);
	}
	inline void endAuxMeasurements()
	{
		//m_measurements.resize (m_measurements.size ());
	}
	inline long size() const
	{
		return (long)m_measurements.size() - 1;
	}
	inline void getPrecursorInfo(long index, double &dPrecursorMZ, double &dPrecursorIntensity) const
	{
		m_measurements[index].getPrecursorInfo (dPrecursorMZ, dPrecursorIntensity);
	}
	inline void getPrecursorInfoEx(long index, int &nPrecursorCharge, double &dPrecursorCE) const
	{
		m_measurements[index].getPrecursorInfoEx (nPrecursorCharge, dPrecursorCE);
	}
	inline void getPrecursorInfoExAll(long index, double &dPrecursorMZ, double &dPrecursorIntensity, int &nPrecursorCharge, double &dPrecursorCE) const
	{
		getPrecursorInfo(index, dPrecursorMZ, dPrecursorIntensity);
		getPrecursorInfoEx(index, nPrecursorCharge, dPrecursorCE);
	}
	inline void getPrecursorMZCharge(long index, double &dPrecursorMZ, double &dPrecursorIntensity, int &nPrecursorCharge) const
	{
		m_measurements[index].getPrecursorMZCharge (dPrecursorMZ, dPrecursorIntensity, nPrecursorCharge);
	}
	inline void dump(void) const
	{
		std::cout << "Chromatogram Auxiliary Info" << std::endl;
		for (long index=1; index<size(); index++) {
			std::cout << index;
			std::cout << ":" << m_measurements[index].m_dPrecursorMZ;
			std::cout << "," << m_measurements[index].m_dPrecursorIntensity;
			std::cout << "," << m_measurements[index].m_nPrecursorCharge;
			std::cout << "," << m_measurements[index].m_dPrecursorCE;
			std::cout << std::endl;
		}
	}
};

class ExperimentInfo {
public:
	MSScanType     m_msScan;
	MSPolarityType m_msPolarity;
	long           m_nUseMCAScans;
	int            m_nMsLevel;

	ChromatogramMeasurements    m_chromatogramMeasurements;
	ChromatogramAuxMeasurements m_chromatogramAuxMeasurements;
	MRMExperimentInfo           m_mrmExperimentInfo;

public:
	ExperimentInfo() 
		: m_msScan(SCAN_UNDEF), m_msPolarity(POLARITY_UNDEF), m_nUseMCAScans(0), m_nMsLevel(0)
	{}
	ExperimentInfo(MSScanType msScan, MSPolarityType msPolarity, long nUseMCAScans, int nMsLevel, long nNumberOfDataPoints)
		: m_msScan(msScan), m_msPolarity(msPolarity), 
			m_nUseMCAScans(nUseMCAScans), m_nMsLevel(nMsLevel)
	{
	}
	inline void init(long lNumberOfDataPoints)
	{
		m_chromatogramMeasurements.newMeasurements (lNumberOfDataPoints);
		m_chromatogramAuxMeasurements.newAuxMeasurements (lNumberOfDataPoints);
	}
	inline void init(double dPauseBetweenMassRanges, long lNumberOfTransitions, long lNumberOfDataPoints)
	{
		m_mrmExperimentInfo.init (dPauseBetweenMassRanges, lNumberOfTransitions, lNumberOfDataPoints);
	}
	inline void setInfo(long lScanType, long lPolarity, long nUseMCAScans)
	{
		m_msScan  = getMSScanType(lScanType);
		m_msPolarity  = (0==lPolarity?POSITIVE:NEGATIVE);
		m_nUseMCAScans = nUseMCAScans;
		m_nMsLevel = getMSLevel(lScanType);
	}
	inline void getInfo(MSScanType &msScan, MSPolarityType &msPolarity, long &nUseMCAScans) const
	{
		msScan = m_msScan;
		msPolarity = m_msPolarity;
		nUseMCAScans = m_nUseMCAScans;
	}
	inline void getSpectraInfo(MSScanType &msScan, MSPolarityType &msPolarity, int &nMsLevel) const
	{
		msScan = m_msScan;
		msPolarity = m_msPolarity;
		nMsLevel = m_nMsLevel;
	}
	inline long getNumberOfDataPoints(void) const {
		return m_chromatogramMeasurements.size ();
	}
	inline long getNumberOfTransitions(void) const {
		return m_mrmExperimentInfo.getNumberOfTransitions();
	}
	inline void dump(void) {
		std::cout << "Experiment(" << m_msScan << "," << m_msPolarity << "," << m_nUseMCAScans << "," << m_nMsLevel << ")" << std::endl;
		m_chromatogramMeasurements.dump();
		m_chromatogramAuxMeasurements.dump();
		m_mrmExperimentInfo.dump();
	}
};

class PeriodInfo {
	typedef std::vector<ExperimentInfo> ExperimentsInfo;
public:
	ExperimentsInfo m_experimentsInfo;
public:
	PeriodInfo(void)
	{
	}
	inline void init(long lNumberOfExperiments)
	{
		m_experimentsInfo.resize(lNumberOfExperiments);
	}
	inline long getNumberOfExperiments (void) const
	{
		return (long)m_experimentsInfo.size();
	}
	inline ExperimentInfo &getExperiment(long lExperimentId) {
		return m_experimentsInfo[lExperimentId];
	}
	inline bool useExperiment0RetentionTime (void) const {
		return (1==m_experimentsInfo[0].m_nMsLevel);
	}
	inline bool getExperimentMergeStates(std::vector<bool> &experimentsMergable) const
	{
		experimentsMergable.resize(getNumberOfExperiments(), false);
		bool fMergable=false;
		for(long lExperimentIndex=0; lExperimentIndex<getNumberOfExperiments(); lExperimentIndex++) {
			if (isMSScanTypeMergable(m_experimentsInfo[lExperimentIndex].m_msScan)) {
				experimentsMergable[lExperimentIndex]=fMergable=true;
			}
		}
		return fMergable;
	}
	inline void dump(void) {
		for(long lExperimentIndex=0; lExperimentIndex<getNumberOfExperiments(); lExperimentIndex++) {
			std::cout << "Experiment#" << (lExperimentIndex+1) << std::endl;
			m_experimentsInfo[lExperimentIndex].dump();
		}
	}
};

class SampleInfo {
	typedef std::vector<PeriodInfo> PeriodsInfo;
public:
	long m_lSampleId;
	PeriodsInfo m_periodsInfo;
	bool m_fDDERealTimeData;
	bool m_fDDERealTimeDataEx;

	enum DDEDataType
	{
		TYPE_NONE = 0,
		TYPE_DDE = 1,
		TYPE_DDEEX = 2
	};
public:
	SampleInfo()
		:	m_lSampleId(0),
		m_fDDERealTimeData(false),
		m_fDDERealTimeDataEx(false)
	{
	}
	inline long getId () const
	{
		return m_lSampleId;
	}
	inline void init(long lSampleId, long lNumberOfPeriods)
	{
		m_lSampleId = lSampleId;
		m_periodsInfo.resize(lNumberOfPeriods);
	}
	inline long getNumberOfPeriods () const
	{
		return (long)m_periodsInfo.size();
	}
	inline PeriodInfo &getPeriod(long lPeriodId) {
		return m_periodsInfo[lPeriodId];
	}
	inline void getTimeBoundInSec(double &startTime, double &endTime) const {
		if (m_periodsInfo.empty()) {
			startTime = 0;
			endTime = 0;
		} else {
			if (m_periodsInfo[0].m_experimentsInfo.empty()) {
				startTime = 0;
			} else {
				startTime = m_periodsInfo[0].m_experimentsInfo[0].m_chromatogramMeasurements.getMeasurementXFirst();
				startTime *= 60;
			}
			if (m_periodsInfo[m_periodsInfo.size()-1].m_experimentsInfo.empty()) {
				endTime = 0;
			} else {
				//endTime = m_periodsInfo[m_periodsInfo.size()-1].m_experimentsInfo[m_periodsInfo[m_periodsInfo.size()-1].m_experimentsInfo.size()-1].m_chromatogramMeasurements.getMeasurementXLast();
				endTime = m_periodsInfo[m_periodsInfo.size()-1].m_experimentsInfo[0].m_chromatogramMeasurements.getMeasurementXLast();
				endTime *= 60;
			}
		}
	}
	inline void DDECached (void) {
		m_fDDERealTimeData = true;
	}
	inline void DDEExCached (void) {
		m_fDDERealTimeData = m_fDDERealTimeDataEx = true;
	}
	inline void resetDDEState (void) {
		m_fDDERealTimeData = m_fDDERealTimeDataEx = false;
	}
	inline bool isDDEAvailable (void) const {
		return m_fDDERealTimeData;
	}
	inline DDEDataType getType(void)
	{
		if (m_fDDERealTimeDataEx) return TYPE_DDEEX;
		if (m_fDDERealTimeData) return TYPE_DDE;
		return TYPE_NONE;
	}
};

class MergeCandidate {
public:
	long   m_lExperimentIndex;
	long   m_lDataPointIndex;
	double m_dPrecursorMZ;
	int    m_nPrecursorCharge;
	long   m_lScanNumber;
	bool   m_fMerged;
public:
	MergeCandidate()
	{
		m_lExperimentIndex = 0;
		m_lDataPointIndex = 0;
		m_dPrecursorMZ = 0;
		m_nPrecursorCharge = 0;
		m_lScanNumber = 0;
		m_fMerged = false;
	}
	MergeCandidate(long lExperimentIndex, long lDataPointIndex, double dPrecursorMZ, int nPrecursorCharge, long lScanNumber)
	{
		m_lExperimentIndex = lExperimentIndex;
		m_lDataPointIndex = lDataPointIndex;
		m_dPrecursorMZ = dPrecursorMZ;
		m_nPrecursorCharge = nPrecursorCharge;
		m_lScanNumber = lScanNumber;
		m_fMerged = false;
	}
	inline void dump(void) const
	{
		std::cout << "MergeCandidate(" << m_lExperimentIndex << ": " << m_lDataPointIndex;
		std::cout << ", " << m_dPrecursorMZ;
		std::cout << ", " << m_nPrecursorCharge;
		std::cout << ", " << m_lScanNumber;
		std::cout << ", " << (m_fMerged ? "merged" : "avail");
		std::cout << ")";
		std::cout << std::endl;
	}
};

typedef std::vector<MergeCandidate> MergeCandidates;

class MergedScan {
public:
	long                m_lExperimentIndex;
	long                m_lDataPointIndex;
	double              m_dPrecursorMZ;
	int                 m_nPrecursorCharge;
	long                m_lScanNumber;
	std::vector<long>   m_mergedExperimentIndex;
	std::vector<long>   m_mergedDataPointIndex;
	std::vector<long>   m_mergedScanNumber;
	std::vector<size_t> m_mergedScanCandidateIndex;

public:
	MergedScan() { init (); }
	MergedScan(const MergeCandidate &firstScan, size_t candidateId)
	{
		m_lExperimentIndex = firstScan.m_lExperimentIndex;
		m_lDataPointIndex = firstScan.m_lDataPointIndex;
		m_dPrecursorMZ = firstScan.m_dPrecursorMZ;
		m_nPrecursorCharge = firstScan.m_nPrecursorCharge;
		m_lScanNumber = firstScan.m_lScanNumber;
		mergeScan(firstScan, candidateId);
	}
	inline void init()
	{
		m_lExperimentIndex = 0;
		m_lDataPointIndex = 0;
		m_dPrecursorMZ = 0;
		m_nPrecursorCharge = 0;
		m_lScanNumber = 0;
		m_mergedExperimentIndex.clear();
		m_mergedDataPointIndex.clear();
		m_mergedScanNumber.clear();
		m_mergedScanCandidateIndex.clear();
	}
	inline void mergeScan(const MergeCandidate &scanToMerge, size_t candidateId)
	{
		m_mergedExperimentIndex.push_back(scanToMerge.m_lExperimentIndex);
		m_mergedDataPointIndex.push_back(scanToMerge.m_lDataPointIndex);
		m_mergedScanNumber.push_back(scanToMerge.m_lScanNumber);
		m_mergedScanCandidateIndex.push_back(candidateId);
	}
	inline long getNumberOfMergedScan() const
	{
		return (long)m_mergedExperimentIndex.size();
	}
	inline long getCandidateId(int i) const
	{
		return m_mergedScanCandidateIndex[i];
	}
	inline long getMergedScanNumber(int i) const
	{
		return m_mergedScanNumber[i];
	}
};

class MergedScanTable {
public:
	typedef std::vector<MergedScan> MergedScans;

public:
	long        m_lPeriodIndex;
	MergedScans m_experimentsMergedScans;

public:
	MergedScanTable() { init(); }
	~MergedScanTable() { }

	inline SamplePeriodKey getKey(void) const
	{
		return SamplePeriodKey(m_lPeriodIndex);
	}
	inline void init(void)
	{
		initNewSample(0);
	}
	inline void initNewSample(long lPeriodIndex)
	{
		m_lPeriodIndex = lPeriodIndex;
		m_experimentsMergedScans.clear();
	}
	inline void cacheSampleMergedScan(const MergedScan &mergedScan)
	{
		m_experimentsMergedScans.push_back(mergedScan);
	}
	inline long getNumberOfMergedScan() const
	{
		return (long)m_experimentsMergedScans.size();
	}
	inline void dump(void) const
	{
		if (m_experimentsMergedScans.size()>0) {
			std::cout << "#id: expt#, dp#, scan#, MZ, charge {expt#1, dp#1, scan#1; expt#2, dp#2, scan#2; ...}" << std::endl;
			long lIndex=0;
			for(MergedScans::const_iterator iter=m_experimentsMergedScans.begin();
				iter != m_experimentsMergedScans.end();
				++iter) {
				lIndex++;
				std::cout << "Merged Scan#" << lIndex << ": " << iter->m_lExperimentIndex;
				std::cout << ", " << iter->m_lDataPointIndex;
				std::cout << ", " << iter->m_lScanNumber;
				std::cout << ", " << iter->m_dPrecursorMZ;
				std::cout << ", " << iter->m_nPrecursorCharge;
				std::cout << " {";
				for (long i=0; i<iter->getNumberOfMergedScan(); i++) {
					if (i!=0) std::cout << ";";
					std::cout << " " << iter->m_mergedExperimentIndex[i];
					std::cout << "," << iter->m_mergedDataPointIndex[i];
					std::cout << "," << iter->m_mergedScanNumber[i];
				}
				std::cout << "}" << std::endl;
			}
		}
	}
};

class PeaksCharge {
public:
	std::vector<double> m_peaksMZ;
	std::vector<int> m_peaksCharge;
public:
	PeaksCharge() { init (); }
	inline void init()
	{
		m_peaksMZ.clear();
		m_peaksCharge.clear();
	}
	inline void add(double mz, double mzStart, double mzEnd, int charge)
	{
		m_peaksMZ.push_back(mzStart);
		m_peaksCharge.push_back(charge);
		m_peaksMZ.push_back(mzEnd);
		m_peaksCharge.push_back(charge);
	}
	inline int getCharge(double mz)
	{
		int nCharge = 0;
		std::vector<double>::const_iterator iterMZ=m_peaksMZ.begin();
		std::vector<int>::const_iterator iterCharge=m_peaksCharge.begin();
		double diff;
		if (iterMZ != m_peaksMZ.end()) {
			diff = fabs(*iterMZ - mz);
			nCharge = *iterCharge;
		}

		for(; iterMZ!=m_peaksMZ.end() && iterCharge!=m_peaksCharge.end() ; ++iterMZ, ++iterCharge) {
			double currDiff = *iterMZ - mz;
			if (fabs(currDiff) < diff) {
				diff = fabs(currDiff);
				nCharge = *iterCharge;
			} else if (*iterMZ>mz) {
				break;
			}
		}
		return nCharge;
	}
};

#include <map>
typedef std::map<SamplePeriodKey,MergedScanTable > MergedScanTableCache;

#include "mzXML/common/Scan.h"
#include <deque>
typedef Scan * ScanPtr;

class ScanIteratorState {
public:
	enum IteratorStateType
	{
		STATE_NO_TRANSITION = 0,
		STATE_ENDED = 1,
		STATE_NEW_SAMPLE = 2,
		STATE_NEW_PERIOD = 3,
		STATE_MRM_CONTINUE = 4,
		STATE_MERGEDSCAN_CONTINUE = 5
	};

	long   m_lCurrentPeriodIndex;
	long   m_lCurrentExperimentIndex;
	long   m_lCurrentDatapointIndex;
	long   m_lMaxSampleIndex;
	long   m_lMaxPeriodIndex;
	long   m_lMaxExperimentIndex;
	long   m_lMaxDatapointIndex;
	bool   m_fIterationEnded;

	bool                m_fMRMScans;
	std::deque<ScanPtr> m_queueMRMScans;

	bool                m_fMergedScans;
	bool                m_fServeMergedScans;
	IteratorStateType   m_stateAfterMerge;
	std::deque<ScanPtr> m_queueMergedScans;

	SampleInfo &m_sampleInfo;

public:
	ScanIteratorState(SampleInfo &sampleInfo) : m_sampleInfo(sampleInfo)
	{
		init();
	}
	~ScanIteratorState() { clearMRMScan(); }

	inline SamplePeriodKey getKey(void) const
	{
		return SamplePeriodKey(m_lCurrentPeriodIndex);
	}
	//Sample index starts with 1, Data point index starts with 1
	//Period index starts with 0, Experiment index starts with 0
	inline void setSample(SampleInfo &sampleInfo) {
		m_sampleInfo = sampleInfo;
	}
	inline void init(void)
	{
		m_fIterationEnded = false;
		m_lCurrentPeriodIndex = 0;
		m_lCurrentExperimentIndex = 0;
		m_lCurrentDatapointIndex = 1;

		initNewSample();
	}
	inline void initNewSample()
	{
		m_lMaxPeriodIndex = m_sampleInfo.getNumberOfPeriods();
		initNewPeriod();
	}
	inline void initNewPeriod()
	{
		if (m_lMaxPeriodIndex > 0) {
			PeriodInfo &period = m_sampleInfo.getPeriod(m_lCurrentPeriodIndex);
			if ((m_lMaxExperimentIndex = period.getNumberOfExperiments()) > 0) {
				m_lMaxDatapointIndex = period.getExperiment(0).getNumberOfDataPoints();
			} else {
				m_lMaxDatapointIndex = 0;
			}
		} else {
			m_lMaxExperimentIndex = 0;
			m_lMaxDatapointIndex = 0;
		}
		clearMRMScan();
		clearMergedScan();
	}
	inline bool isEnd(void)
	{
		return m_fIterationEnded;
	}
	inline void pushMRMScan(Scan *pScan)
	{
		m_queueMRMScans.push_back(pScan);
		m_fMRMScans = true;
	}
	inline Scan *popMRMScan(void)
	{
		if (m_fMRMScans) {
			Scan *pScan = m_queueMRMScans.front(); 
			m_queueMRMScans.pop_front();
			m_fMRMScans = (m_queueMRMScans.size() > 0);
			return pScan;
		} else {
			return NULL;
		}
	}
	inline void clearMRMScan(void)
	{
		std::deque<ScanPtr>::const_iterator iter = m_queueMRMScans.begin();
		for (; iter != m_queueMRMScans.end(); ++iter) {
			delete *iter;
		}
		m_queueMRMScans.clear();
		m_fMRMScans = false;
	}
	inline bool isMRMScanAvailable(void)
	{
		return m_fMRMScans;
	}
	//-----
	inline void pushMergedScan(Scan *pScan)
	{
		m_queueMergedScans.push_back(pScan);
		m_fMergedScans = true;
	}
	inline Scan *popMergedScan(void)
	{
		if (m_fServeMergedScans && m_fMergedScans) {
			Scan *pScan = m_queueMergedScans.front(); 
			m_queueMergedScans.pop_front();
			m_fMergedScans = (m_queueMergedScans.size() > 0);
			return pScan;
		} else {
			return NULL;
		}
	}
	inline void clearMergedScan(void)
	{
		std::deque<ScanPtr>::const_iterator iter = m_queueMergedScans.begin();
		for (; iter != m_queueMergedScans.end(); ++iter) {
			delete *iter;
		}
		m_queueMergedScans.clear();
		m_fMergedScans = false;
		m_fServeMergedScans = false;
		m_stateAfterMerge = STATE_NO_TRANSITION;
	}
	inline bool isMergedScanAvailable(void)
	{
		return m_fMergedScans;
	}
	//-----
	IteratorStateType next(void)
	{
		if (m_fIterationEnded && !m_fServeMergedScans) return STATE_ENDED;

		if (!m_queueMRMScans.empty ()) return STATE_MRM_CONTINUE;

		if (m_fServeMergedScans) {
			if (!m_queueMergedScans.empty ()) return STATE_MERGEDSCAN_CONTINUE;

			m_fServeMergedScans = false;
			return m_stateAfterMerge;
		}

		IteratorStateType state = nextScan ();
		
		while (STATE_NO_TRANSITION == state && m_sampleInfo.getPeriod(m_lCurrentPeriodIndex).getExperiment(m_lCurrentExperimentIndex).m_chromatogramMeasurements.getMeasurementY(m_lCurrentDatapointIndex)==0) {
			state = nextScan ();
		}
		//TODO: check how about new period and sample?
		return state;
	}
	IteratorStateType nextScan(void)
	{
		m_lCurrentExperimentIndex++;
		if (m_lCurrentExperimentIndex >= m_lMaxExperimentIndex) {
			m_lCurrentExperimentIndex = 0;
			m_lCurrentDatapointIndex++;
			if (m_lCurrentDatapointIndex > m_lMaxDatapointIndex) { // data point is 1-based
				m_lCurrentDatapointIndex = 1;
				m_lCurrentPeriodIndex++;
				if (m_lCurrentPeriodIndex >= m_lMaxPeriodIndex) {
					if (isMergedScanAvailable()) {
						m_fServeMergedScans = true;
						m_fIterationEnded = true;
						m_stateAfterMerge = STATE_ENDED;
						return STATE_MERGEDSCAN_CONTINUE;
					} else {
						m_fIterationEnded = true;
						return STATE_ENDED;
					}
				} else {
					// we have moved to a new period
					// number of experiments, and number of data points can changed
					if (isMergedScanAvailable()) {
						m_fServeMergedScans = true;
						m_stateAfterMerge = STATE_NEW_PERIOD;
						return STATE_MERGEDSCAN_CONTINUE;
					} else {
						return STATE_NEW_PERIOD;
					}
				}
			}
		}
		return STATE_NO_TRANSITION;
	}
};

class ScanTable {
	typedef std::vector<long>    ScanNumbers;

public:
	bool m_fScanMapped;
	long m_lPeriodIndex;
	long m_lNumExperiments;
	long m_lNumDataPoints;
	long m_lLastScanNumber;
	long m_lStartScanNumber;
	std::vector<ScanNumbers> m_experimentsScanNumbers;

public:
	ScanTable() { init(); }
	~ScanTable() { }

	inline SamplePeriodKey getKey(void) const
	{
		return SamplePeriodKey(m_lPeriodIndex);
	}
	inline void init(void)
	{
		m_fScanMapped = false;
		m_lPeriodIndex = 0;
		m_lNumExperiments = 0;
		m_lNumDataPoints = 0;
		m_lStartScanNumber = m_lLastScanNumber = 0;
	}
	inline bool isAvailable(long lPeriodIndex)
	{
		return (m_fScanMapped);
	}
	inline long getNumberOfExperiments()
	{
		return m_lNumExperiments;
	}
	inline long getNumberOfDataPoints()
	{
		return m_lNumDataPoints;
	}
	inline void initNewSample(long lPeriodIndex, long lNumExperiments, long lNumDataPoints, long lLastScanNumber)
	{
		m_fScanMapped = false;
		m_experimentsScanNumbers.resize(lNumExperiments);
		for (long lExperimentIndex=0; lExperimentIndex<lNumExperiments; lExperimentIndex++) {
			m_experimentsScanNumbers[lExperimentIndex].resize(lNumDataPoints+1, 0);
		}
		m_lNumExperiments = lNumExperiments;
		m_lNumDataPoints = lNumDataPoints;
		m_lStartScanNumber = m_lLastScanNumber = lLastScanNumber;
	}
	inline void cacheSampleScanNumber(long lExperimentIndex, long lDataPointIndex)
	{
		m_lLastScanNumber++;
		m_experimentsScanNumbers[lExperimentIndex][lDataPointIndex]=m_lLastScanNumber;
	}
	inline void sampleCached(long lSampleIndex, long lPeriodIndex)
	{
		m_fScanMapped = true;
		m_lPeriodIndex = lPeriodIndex;
	}
	inline void getPrecursorScanNumber(long lExperimentIndex, long lDatapointIndex, long &lScanNumber) const
	{
		if (m_fScanMapped) {
			lScanNumber = m_experimentsScanNumbers[lExperimentIndex][lDatapointIndex];
		} else {
			lScanNumber = 0;
		}
	}
	inline void reorderScanNumbers()
	{
		if (!m_fScanMapped) return;

		long lCurrentScanNumber = m_lStartScanNumber;
		for (long lDataPointIndex=1; lDataPointIndex<=m_lNumDataPoints; lDataPointIndex++) {
			for (long lExperimentIndex=0; lExperimentIndex<m_lNumExperiments; lExperimentIndex++) {
				if (0!=m_experimentsScanNumbers[lExperimentIndex][lDataPointIndex]) {
					m_experimentsScanNumbers[lExperimentIndex][lDataPointIndex] = ++lCurrentScanNumber;
				}
			}
		}
	}
	inline long getLastScanNumberAssigned() const
	{
		return m_lLastScanNumber;
	}
};

class ScanOriginToMergeTable {
	typedef std::map<long, long> ScanOriginToMergeEntries;

public:
	ScanOriginToMergeEntries m_scanOriginToMergeEntries;

	ScanOriginToMergeTable() { }
	~ScanOriginToMergeTable() { }

	inline void add(long lScanOriginNum, long lMergedScanNum)
	{
		m_scanOriginToMergeEntries[lScanOriginNum] = lMergedScanNum;
	}
	inline long getMergedScanNum(long lScanOriginNum) const
	{
		std::map<long, long>::const_iterator iter = m_scanOriginToMergeEntries.find(lScanOriginNum);
		if (iter == m_scanOriginToMergeEntries.end()) {
			return lScanOriginNum;
		} else {
			return iter->second;
		}
	}
};

//======================================================================

class MSDataProcessing {
public:
	double m_dPeakCutoffMin;
	double m_dPeakCutoffPercentageMin;
	double m_dPeakCutoffMax;
	double m_dPeakCutoffPercentageMax;
	double m_dPeakCutoffMinMS1;
	double m_dPeakCutoffPercentageMinMS1;
	double m_dPeakCutoffMaxMS1;
	double m_dPeakCutoffPercentageMaxMS1;

	double m_dGroupPrecursorMassTolerance;
	int    m_iGroupMaxCyclesBetween;
	int    m_iGroupMinCycles;
	//future work
	bool   m_fGuessCharge;
	//END-future work

	int    m_iFilterMinPeakCount;

	bool   m_fCentroidMS1;
	bool   m_fCoordinate;

public:
	MSDataProcessing() { init(); }
	~MSDataProcessing() { }

	inline void init(void)
	{
		m_dPeakCutoffMin = 0.0;
		m_dPeakCutoffPercentageMin = 10;
		m_dPeakCutoffMax = 0.0;
		m_dPeakCutoffPercentageMax = 0.0;

		m_dPeakCutoffMinMS1 = 0.0;
		m_dPeakCutoffPercentageMinMS1 = 10;
		m_dPeakCutoffMaxMS1 = 0.0;
		m_dPeakCutoffPercentageMaxMS1 = 0.0;

		m_dGroupPrecursorMassTolerance = 1.0;
		m_iGroupMaxCyclesBetween = 10;
		m_iGroupMinCycles = 1;
		m_fGuessCharge = false;

		m_iFilterMinPeakCount = 10;

		m_fCentroidMS1 = false;

		m_fCoordinate = false;
	}

	// for MRM (absolute value)
	inline void getPeakCutOffBound(double &dMinCutOff, double &dMaxCutOff)
	{
		dMinCutOff = m_dPeakCutoffMin>0 ? m_dPeakCutoffMin : 0;
		dMaxCutOff = m_dPeakCutoffMax>0 ? m_dPeakCutoffMax : 0;
	}

	// for MS2, MS3, etc
	inline void getPeakCutOffBound(double dMaxIntensity, double &dMinCutOff, double &dMaxCutOff)
	{
		if (m_dPeakCutoffPercentageMin>0 || m_dPeakCutoffPercentageMax>0 ) {
			dMinCutOff = m_dPeakCutoffPercentageMin>0 ? (m_dPeakCutoffPercentageMin*dMaxIntensity/100.0) : 0;
			dMaxCutOff = m_dPeakCutoffPercentageMax>0 ? (m_dPeakCutoffPercentageMax*dMaxIntensity/100.0) : 0;
			return;
		}

		dMinCutOff = m_dPeakCutoffMin>0 ? m_dPeakCutoffMin : 0;
		dMaxCutOff = m_dPeakCutoffMax>0 ? m_dPeakCutoffMax : 0;
	}

	// for MS1
	inline void getPeakCutOffBoundMS1(double dMaxIntensity, double &dMinCutOff, double &dMaxCutOff)
	{
		if (m_dPeakCutoffPercentageMinMS1>0 || m_dPeakCutoffPercentageMaxMS1>0 ) {
			dMinCutOff = m_dPeakCutoffPercentageMinMS1>0 ? (m_dPeakCutoffPercentageMinMS1*dMaxIntensity/100.0) : 0;
			dMaxCutOff = m_dPeakCutoffPercentageMaxMS1>0 ? (m_dPeakCutoffPercentageMaxMS1*dMaxIntensity/100.0) : 0;
			return;
		}

		dMinCutOff = m_dPeakCutoffMinMS1>0 ? m_dPeakCutoffMinMS1 : 0;
		dMaxCutOff = m_dPeakCutoffMaxMS1>0 ? m_dPeakCutoffMaxMS1 : 0;
	}
};

//======================================================================

#include "mzXML/common/InstrumentInfo.h"

class InstrumentInfoParser
{
public:
	InstrumentInfoParser(InstrumentInfo &instrumentInfo, int iVerbose);
	~InstrumentInfoParser(void);
public:
	bool parse(const std::string &strLog, long lModelId);
private:
	void getMSManufacturer(const std::string &strLog, std::string &strManufacturer);
	void getMSSerialNumber(const std::string &strLog, std::string &strSerialNumber);
	void getMSHardwareVersion(const std::string &strLog, std::string &strHardwareVersion);
	void getMSIonisation(const std::string &strLog, std::string &strIonisation);
	void getMSMassAnalyzer(const std::string &strLog, std::string &strMassAnalyzer);
	void getMSDetector(const std::string &strLog, std::string &strDetector);
private:
	int m_iVerbose;
	InstrumentInfo &m_instrumentInfo;
};

class SoftwareInfoParser
{
public:
	SoftwareInfoParser(InstrumentInfo &instrumentInfo, int iVerbose);
	~SoftwareInfoParser(void);
public:
	bool parse(const std::string &strCreator);
private:
	int m_iVerbose;
	InstrumentInfo &m_instrumentInfo;
};

class WiffStructureLoader
{
public:
	WiffStructureLoader(const std::string &strFilename, int iVerbose);
	~WiffStructureLoader(void);
public:
	bool load(bool fReportStructure);
	bool isWiffScanFileExist();
public:
	std::string m_strCreator;
	bool        m_fHasWiffScanFile;
private:
	typedef std::vector<std::string> TextLines;
	HRESULT ReadStorage(IStorage* pIStorage, int nLevel, bool fReportStructure);
	void ExplainError (HRESULT hr);
	void LogStructurePrefix (int nLevel, const char* szCategory, const char *szName);
	void ExtractStreamText (const char *szName, const TCHAR *tchData, DWORD dwRead, TextLines &textLines);
private:
	static const int MAX_TEXT;
	std::string m_strFilename;
	int         m_iVerbose;
	int         m_iSampleTag;
	int         m_iSampleScanTag;
	bool        m_loaded;
};

//======================================================================
