// -*- mode: c++ -*-


/*
    File: ProgramOptions.h
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


#pragma once

#include <string>

using namespace std;

class ProgramOptions
{
public:
	bool   m_fAnalystLibQS;
	bool   m_fAnalystLib;
	string m_strModel;
	string m_strIonisation;
	string m_strMSType;
	string m_strDetector;
	bool   m_fUseWiffFileInformation;
	bool   m_fCompression;
	bool   m_fGzipOutput;
	bool   m_fCentroid;
	bool   m_fCentroidMS1;
	double m_dPeakCutoffMin;
	double m_dPeakCutoffPercentageMin;
	double m_dPeakCutoffMax;
	double m_dPeakCutoffPercentageMax;
	double m_dPeakCutoffMinMS1;
	double m_dPeakCutoffPercentageMinMS1;
	double m_dPeakCutoffMaxMS1;
	double m_dPeakCutoffPercentageMaxMS1;
	bool   m_fDeisotope;
	double m_dGroupPrecursorMassTolerance;
	int    m_iGroupMaxCyclesBetween;
	int    m_iGroupMinCycles;
	bool   m_fGuessCharge;
	int    m_iFilterMinPeakCount;
	int    m_iVerbose;
	bool   m_fmzMLMode;
	bool   m_fmzXMLMode;
	bool   m_fCoordinate;
	string m_inputFileName;
	string m_outputFileName;
	int    m_iSampleStartId;
	int    m_iSampleEndId;
	bool   m_fOnlyCompatible;
	bool   m_fShowWiffTree;

	ProgramOptions ();
	void printArgs(void);
	bool parseArgs(int argc, char* argv[]);

private:
	bool setOption(const char *arg);
};
