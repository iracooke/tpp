// -*- mode: c++ -*-


/*
    File: MassHunterInterface.h
    Description: Encapsulation for Agilent MassHunter interface.
    Date: July 25, 2007

    Copyright (C) 2007 Natalie Tasman, ISB Seattle


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


#include "stdafx.h"

#include <vector>
#include <string>
#include <map>
#include "mzXML/common/InstrumentInterface.h"
#include "mzXML/converters/trapper/MHDACWrapper/MHDACWrapper.h"


// NOTE: "MH" namespace defined/imported from tlb in MHDACWrapper.h


class MassHunterInterface : public InstrumentInterface {
private:
	std::string inputFileName_;
	bool verbose_;

	// dll interface stuff here
	
	MHDACWrapper* MHDACWrapper_;

	long firstScanNumber_;
	long lastScanNumber_;
	bool firstTime_;

		

	// COM objects which must scope beyond one member function
	MH::IMsdrDataReader* pMSDataReader_;
	MH::BDA::IBDAChromData* pChromData_; // gives TIC x RT
	MH::BDA::IBDAChromData* pChromDataBasePeaks_; // gives BPI x RT
	SAFEARRAY *pSafeArrayRetentionTimes_;
	SAFEARRAY *pSafeArrayTotalIonCurrents_;
	SAFEARRAY *pSafeArrayBasePeakIntensities_;
	SAFEARRAY *pSafeArrayChromData_;
	// note! ChromData x array is double, while y is float.
	// void casting on safearray access misses this, if you're not careful.
	double* retentionTimeArray_;
	float* totalIonCurrentArray_;
	float* basePeakIntensityArray_;

	std::map<long, long> nativeToSequentialScanNums_;
	
public:
	long curScanNum_;

public:
	MassHunterInterface(MHDACWrapper* mhdacWrapper);
	~MassHunterInterface(void);

	virtual bool initInterface(void);
	virtual bool setInputFile(const std::string& fileName);
	virtual void setCentroiding(bool centroid);
	virtual void setDeisotoping(bool deisotope);
	virtual void setCompression(bool compression);
	virtual void setShotgunFragmentation(bool sf) {}
	virtual void setLockspray(bool ls) {}
	virtual Scan* getScan(void);

	// add this to InstrumentInterface
	virtual void setVerbose(bool verbose);

	// TODO: make these private with accessors, or clean up code to just get converterargs obj
		// deisotoping paramaters, available in MHDAC 1.2.1
	bool requirePeptideLikeAbundanceProfile_;
	double relativeTolerance_;
	double absoluteTolerance_;
	bool limitChargeState_;
	int maxChargeState_;

	SpectraModeType spectraMode_; // for the whole file
};


