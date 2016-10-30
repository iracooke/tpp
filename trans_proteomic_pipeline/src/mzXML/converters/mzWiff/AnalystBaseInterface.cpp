// -*- mode: c++ -*-


/*
    File: AnalystBaseInterface.cpp
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




#include <iostream>

#include "stdafx.h"
#include "AnalystBaseInterface.h"
#include "MsgBoxCloser.h"

#include "mzXML/common/Scan.h"
#include "mzXML/common/MSUtilities.h"

AnalystBaseInterface::AnalystBaseInterface(void) :
	m_enumLibraryType(LIBRARY_Analyst),
	m_fInitialized(false),
	m_fCOMInitialized(false),
	m_pScan(NULL),
	m_dChromotographRTInSec(-1),
	m_fUseExperiment0RT(true),
	m_iVerbose(0),
	m_pMsgBoxCloser(NULL),
	m_fAssumeLibrary(false),
	m_iteratorState(m_sampleInfo),
	m_enumCompatibility(COMPATIBILITY_OK)
{
	// InstrumentInterface members
	totalNumScans_ = 0;
	curScanNum_ = -1;
	firstScanNumber_ = 1;
	lastScanNumber_ = 0;
	doCompression_ =false;
	doCentroid_=false;
	doDeisotope_=false;
	accurateMasses_=0;
	inaccurateMasses_=0;
	// store counts for up to +15
	chargeCounts_.clear();
	chargeCounts_.resize(16, 0);
	instrumentInfo_.manufacturer_ = ABI_SCIEX;
	instrumentInfo_.analyzerList_.push_back(ANALYZER_UNDEF);

	// AnalystBaseInterface members
	startTimeInSec_ = -1;
	endTimeInSec_ = -1;

	firstTime_ = true;
}

AnalystBaseInterface::~AnalystBaseInterface(void)
{
	stopMsgBoxMonitor();
}

void AnalystBaseInterface::setCentroiding(bool centroid) {
	doCentroid_ = centroid;
}

void AnalystBaseInterface::setDeisotoping(bool deisotope) {
	doDeisotope_ = deisotope;
}

void AnalystBaseInterface::setCompression(bool compression) {
	doCompression_ = compression;
}

void AnalystBaseInterface::setVerbose(bool verbose) {
	verbose_ = verbose;
}

void AnalystBaseInterface::setVerbose(int iVerbose) {
	m_iVerbose = iVerbose;
}

void AnalystBaseInterface::assumeLibrary(bool assume) {
	m_fAssumeLibrary = assume;
}

//--------------------------------------------------------------------------------

size_t AnalystBaseInterface::getNumberOfSamples(void) const {
	return m_samples.getNumberOfSamples();
}

bool AnalystBaseInterface::setSample(long lSampleId) {
	// sample specific
	loadInstrumentInfo (lSampleId);

	loadSampleInfo (lSampleId);

	firstTime_ = true;

	//WCH: this is not necessary as getFirstScan() will have done so via getScan()
	//initScanIteratorState();
	return true;
}

void AnalystBaseInterface::startMsgBoxMonitor(void) {
	if (NULL==m_pMsgBoxCloser) {
		m_pMsgBoxCloser = new MsgBoxCloser();
	}
	if (NULL!=m_pMsgBoxCloser) {
		//TODO: uncomment after benchmark testing
		int nErr=0;
		if (0!=(nErr = m_pMsgBoxCloser->start())) {
			std::cerr << "WARNING: Fail to start ABI Dialog monitor (Error#" << nErr << "), conversion might be halt by warning dialog" << std::endl;
		}
	} else {
		std::cerr << "WARNING: Fail to create ABI Dialog monitor, conversion might be halt by warning dialog" << std::endl;
	}
}

void AnalystBaseInterface::stopMsgBoxMonitor(void) {
	if (NULL!=m_pMsgBoxCloser) {
		m_pMsgBoxCloser->stop();
		delete m_pMsgBoxCloser;
		m_pMsgBoxCloser = NULL;
	}
}
