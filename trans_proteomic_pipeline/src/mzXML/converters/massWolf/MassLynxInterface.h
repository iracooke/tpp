// -*- mode: c++ -*-


/*
    File: MassLynxInterface.h
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



#pragma once

#include <vector>
#include <string>
#include "mzXML/common/InstrumentInterface.h"
#include "mzXML/common/Scan.h"

typedef struct{
	int		funcNum;
	int		scanNum;
	int     msLevel;
	long	numPeaksInScan;
	float	retentionTimeInSec; // in seconds
	float	lowMass;
	float	highMass ;
	float	TIC;
	float	basePeakMass;
	float	basePeakIntensity;
	bool    isCalibrated;
} MassLynxScanHeader;


class MassLynxInterface : public InstrumentInterface {
private:
	std::string inputFileName_;
	bool verbose_;

	// dll interface stuff here
	long firstScanNumber_;
	long lastScanNumber_;
	bool firstTime_;

	int	 totalNumFunctions_;
	std::vector<MSScanType> functionTypes_;

	// Scan information
	std::vector<MassLynxScanHeader> scanHeaderVec_;

	// thresholding
	bool threshold_;
	bool thresholdDiscard_;
	double inclusiveCutoff_;

protected:
	virtual void printInfo();
	virtual bool processFile();

public:
	long curScanNum_;

public:
	MassLynxInterface(void);
	~MassLynxInterface(void);

	virtual bool initInterface(void);
	virtual bool setInputFile(const std::string& fileName);
	virtual void setCentroiding(bool centroid);
	virtual void setDeisotoping(bool deisotope);
	virtual void setCompression(bool compression);
	virtual void setShotgunFragmentation(bool sf);
	virtual void setLockspray(bool ls);
	virtual Scan* getScan(void);

	// add this to InstrumentInterface
	virtual void setVerbose(bool verbose);

	void setThresholdInclusiveCutoff(double inclusiveCutoff);
	void setThresholdDiscard(bool discard); // otherwise, zero-out intensities

};


