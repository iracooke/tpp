/*
Program       : xml_out.h for Agilent2mzXML
Author        : David Shteynberg <dshteynb@systemsbiology.org> 
Date          : 01.30.06 
                                                                       
Copyright (C) 2006 David Shteynberg

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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

David Shteynberg
Institute for Systems Biology
401 Terry Avenue North 
Seattle, WA  98109  USA
akeller@systemsbiology.org

Institute for Systems Biology, hereby disclaims all copyright interest 
in Agilent2mzXML written by David Shteynberg

*/

#if !defined(__XML_OUT)
#define __XML_OUT

#include <comdef.h>
#include <conio.h>
#import "C:/WINDOWS/SYSTEM32/TrapDataAccess.dll" rename_namespace("ADAL")


#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <winsock2.h>	//hotonl
#include <string>
#include <io.h>			// _findfirst, _open
#include <fcntl.h>		// _open 

#include "AgilentRAW.h"
#include "sha1.h"
#include "base64.h"

#define LITTLE_ENDIAN
#define MAX_FILE_READ_BUFFER 8000
#define MIN_PEAKS_PER_SPECTRA 0
#define	A2X_VERSION 1.0

using namespace std;

typedef struct _datapeak
{
	double dMass;
	double dIntensity;
} DataPeak;

class XmlOut
{
public:

	// Constructor destructor
	XmlOut( char* pAcquisitionFileName , char* doCentroid );
	//XmlOut( char* pAcquisitionFileName );
	~XmlOut();

	// Getter setter
	// Set the type of controller. This should be the first MS controller
	inline setController( long controllerType , long controllerNum )
	{
		//m_aRawfile.SetCurrentController( controllerType , controllerNum );
		
		// Get the tot number of scans
		m_aRawfile.GetFirstSpectrumNumber( &m_firstScan );
		m_aRawfile.GetLastSpectrumNumber( &m_lastScan );

		// Allocate space for the index
		// TODO remember to free this mem!
		m_index = new __int64 [ (m_lastScan - m_firstScan) + 1];
	}

	// Get the number for the first scan
	inline long getFirstScanNumber()
	{
		return m_firstScan;
	}

	// Get the number for the last scan
	inline long getLastScanNumber()
	{
		return m_lastScan;
	}

	// Write the header information
	inline writeHeader()
	{
		std::cout << "Processing header\n";
		m_prepareHeader();
		m_writeXmlHeader();
	}

	// Write a scan element
	inline writeScan( long scanNum )
	{
		m_scanNum = scanNum;
		m_prepareScan();
		//Moved to m_writeXmlScan() m_addScanToIndex();	
		m_writeXmlScan();
	}

	inline finalizeXml()
	{
		m_writeScanClosing();
		std::cout << "Writing the index\n";
		m_writeXmlIndex();
		m_writeXmlSha1();
		delete [] m_index;
		delete [] m_xmlName;
		delete [] m_pAcquisitionFileName;
//		delete [] m_polarity;

	}
		
private:

	// methods
	void m_prepareHeader();
	void XmlOut::m_convertToURI( char* &p_URI);
	void m_writeXmlHeader();
	void m_prepareScan();
	void m_getFilterField( int fieldNumber , char* szTemp , char* retValue );
	void m_addScanToIndex();
	void m_writeScanClosing();		// Terminate the scan element afte the last scan element
	void m_writeXmlScan();
	void m_writeXmlIndex();
	void m_writeXmlSha1();

	// variables
	AgilentRAW m_aRawfile;			// Create an instance fo the COleDispatchDriver object
	__int64*	m_index;
	char*	m_xmlName;
	BOOL	m_centroid;
	int		m_fieldCorrection;

		// Header information
	char*	m_pAcquisitionFileName;
	char	m_hostName[256];
	long	m_firstScan;
	long	m_lastScan;
	double	m_startTime;
	double	m_endTime;
	CSHA1	m_Csha1;
	char	m_pReport[1024];		// sha1 hash
	BSTR	m_bstrInstModel;
	BSTR	m_bstrInstSoftVersion;

		// Scan information
	long	m_scanNum;
	BSTR	m_bstrFilter;
	long	m_msLevel , m_msLevelPrevScan , m_msLevelFirstScan;
	char*	m_polarity;
	char*	m_scanType;
	long	m_numPeaks;
	double	m_RT;
	double	m_lowMass;
	double	m_highMass ;
	double	m_TIC;
	double	m_basePeakMass;
	double	m_basePeakIntensity;
	long	m_channel;
	long	m_uniformTime;
	double	m_frequency;
	double	m_CE;	// RT of the survey scan from which the precursor was selected
	double	m_precursorIntensity;
	double	m_precursorMz;
	int     m_precursorCharge;
	char	*m_pEncoded , *m_pEncodedPrevScan;

	std::ofstream m_fout;
	int	m_idxFout;

	static int M_accurateMasses;
	static int M_inaccurateMasses;
	static int M_charges[10];

};
#endif // __XML_OUT
