/*
Program       : xml_out.cpp for Agilent2mzXML
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

#include "stdafx.h"
#include "xml_out.h"

#define _countof(e) (sizeof(e) / sizeof(e[0]))

int XmlOut::M_accurateMasses;
int XmlOut::M_inaccurateMasses;
int XmlOut::M_charges[10];

XmlOut::XmlOut(char* pAcquisitionFileName , char* doCentroid ) 
//XmlOut::XmlOut(char* pAcquisitionFileName) 
{
	
	m_pAcquisitionFileName = new char[ strlen( pAcquisitionFileName ) + 100];
	strcpy( m_pAcquisitionFileName , pAcquisitionFileName );
	
	//RETURN_ON_BAD_HR(CoInitialize( NULL ));		// Initializes the COM library on the current thread 
								// and identifies the concurrency model as single-thread 
								// apartment (STA)
	
	m_aRawfile.Init();
	m_aRawfile.Open(m_pAcquisitionFileName );

	// -4 to skip ".raw"
	m_xmlName = new char[strlen(m_pAcquisitionFileName) + 10];	// +10 just to be sure with the
																// extension change

	char* dot = strrchr( m_pAcquisitionFileName , '.' );
	int lenToExt = strlen( m_pAcquisitionFileName ) - (dot ? strlen(dot) : 0);
	strncpy( m_xmlName , m_pAcquisitionFileName , lenToExt );
	strcpy( m_xmlName + lenToExt , ".mzXML" );

	cout << "Saving output to " << m_xmlName << endl;
	
	if( !strcmp( doCentroid , "c" ) )
	{
		cout << "Converting to centroid\n";
		m_centroid = TRUE;
	}
	else if( !strcmp( doCentroid , "p" ) )
	{
		m_centroid = FALSE;
	}
	else
	{
		cerr << doCentroid << " unknown option\n";
		exit(-1);
	}

	// Open output mzXML file
	m_fout.open( m_xmlName );
	if( !m_fout.good() )
	{
		cerr << "Error opening output file\n";
	}
	
	// For the indexing we are going to need an fd stream
	// so that we have a 64 bits ftell function
	if( !(m_idxFout = _open( m_xmlName , _O_RDONLY)) )
	{
		cerr << "Error opening outupt file for indexing\n";
		exit(1);
	}
}
XmlOut::~XmlOut()
{
	// Close the files
	m_fout.close();
	_close( m_idxFout );
	m_aRawfile.Close();
}

void XmlOut::m_prepareHeader()
{
	// get the start and the end time of the run
	//m_aRawfile.GetStartTime( &m_startTime );
	//m_aRawfile.GetEndTime( &m_endTime );

	// get the name of the parent file and sha1 it
	cout << "Calculating sha1-sum of RAW" << endl;
	m_pReport[0] = 0;
	m_Csha1.Reset();
	strcat(m_pAcquisitionFileName, "/Analysis.yep");
	if( !(m_Csha1.HashFile( m_pAcquisitionFileName )) )
    {
      cout << "Cannot open file " << m_pAcquisitionFileName << " for sha-1 calculation\n";
      exit(-1);		// Cannot open file
    }
	m_Csha1.Final();
	m_Csha1.ReportHash( m_pReport, CSHA1::REPORT_HEX );

	// get the instrument model
	//m_bstrInstModel = NULL;
	//m_aRawfile.GetInstModel( &m_bstrInstModel );

	// get acquisition software version
	//m_bstrInstSoftVersion = NULL;
	//m_aRawfile.GetInstSoftwareVersion( &m_bstrInstSoftVersion );

	// get hostname for the URIs
	DWORD  namelen = 256;
	
	BOOL ret = GetComputerName( m_hostName , &namelen);
}

void XmlOut::m_convertToURI( char * &p_URI)
{	
	char	*pos , *pos2;
	int numspaces = 0;
	// Change the first part of the path (the volume name) to the hostname of the localmachine
	if( (pos = strstr( m_pAcquisitionFileName , ":" )) )
	{
		pos++;	// skip the :
		strcpy( p_URI , m_hostName );
		strcpy( p_URI + strlen(m_hostName) , pos );
	}
	else if( pos = strstr( m_pAcquisitionFileName , "\\" ))
	{ // network path
		if( pos = m_pAcquisitionFileName )
		{ // path starts with "\\"
			strcpy( p_URI , m_pAcquisitionFileName + 2 );
		}
	}
	else
	{ // if the path used was not absolute
		strcpy( p_URI , m_pAcquisitionFileName );
	}
	
	
	// Since we are on Windows change all the \ in /
	pos2 = p_URI;
	while( (pos = strstr(  pos2 , " " )) )
	{
		numspaces++;
		pos2 = pos + 1;
	}
	
	char* tmp_p_URI = new char[ strlen( m_hostName ) + strlen( m_pAcquisitionFileName ) + numspaces*2 + 1 ];
	char* oldP = p_URI;
	// Since we are on Windows change all the \ in /
	pos2 = p_URI;
	while( (pos = strstr(  pos2 , "\\" )) )
	{
		*pos = '/';
		pos2 = pos + 1;
	}
	strcpy(tmp_p_URI, p_URI);

	// Encode the spaces
	pos2 = p_URI;
	char* pos3 = tmp_p_URI;
	char *tmp = new char[strlen(pos2)+1];
	while( (pos = strstr(pos3 , " " )) )
	{
		strcpy(tmp, pos+1);
		strcpy(pos+3, tmp); 
		*pos = '%'; pos++;
		*pos = '2'; pos++;
		*pos = '0';	pos++;
		pos3 = pos;
	}
	p_URI = tmp_p_URI;
	delete tmp;	
	delete oldP;
}

void XmlOut::m_writeXmlHeader()
{
	char	lf = 0xA;

	m_fout	<< "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>" << lf
			<< "<mzXML" << lf
			<< " xmlns=\"http://sashimi.sourceforge.net/schema_revision/mzXML_2.0\"" << lf
			<< " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << lf
			<< " xsi:schemaLocation=\"http://sashimi.sourceforge.net/schema_revision/mzXML_2.0 http://sashimi.sourceforge.net/schema_revision/mzXML_2.0/mzXML_idx_2.0.xsd\">" << lf;

	double startTime = m_aRawfile.GetRetentionTime(m_firstScan);
	double endTime = m_aRawfile.GetRetentionTime(m_lastScan);
	m_fout	<< "<msRun scanCount=\"" << ((m_lastScan - m_firstScan) + 1) << "\"" << lf
			<< "       startTime=\"PT" << startTime << "S\"" << lf
			<< "       endTime=\"PT" << endTime << "S\">" << lf; 	
	

	char	*p_URI;
	p_URI = new char[ strlen( m_hostName ) + strlen( m_pAcquisitionFileName ) + 1];
	m_convertToURI( p_URI );

	m_fout	<< "<parentFile fileName=\"file://" << p_URI << "\"" << lf
			<< "            fileType=\"RAWData\"" << lf
			<< "            fileSha1=\"" << m_pReport << "\"/>" << lf;

	delete [] p_URI;

	
  	m_fout	<< "<dataProcessing centroided=\"" << m_centroid << "\">" << lf
  
			<< "<software type=\"conversion\"" << lf
			<< "          name=\"Agilent2mzXML\"" << lf
			<< "          version=\"" << A2X_VERSION << "\"/>" << lf ;

  if( MIN_PEAKS_PER_SPECTRA > 0 )
    {
      // Note the use of the namevaluetype element!
		m_fout	<< "<processingOperation name=\"min_peaks_per_spectra\"" << lf
				<< "                     value=\"" << MIN_PEAKS_PER_SPECTRA << "\"/>" << lf;
      // And the comment field to give a little bit more information about the meaning of
      // the last element.
		m_fout	<< "<comment>Scans with total number of peaks less than min_peaks_per_spectra were not included in this XML file</comment>" << lf;
    }


	m_fout	<< "</dataProcessing>" << lf;

	//SysFreeString( m_bstrInstSoftVersion );
}


void XmlOut::m_prepareScan()
{
	
	char	lf = 0xA;	
	m_msLevel = m_aRawfile.GetMSLevelForSpectrumIdx(m_scanNum);

	if( m_scanNum > m_firstScan )
	{
		int close = m_msLevelPrevScan;
//		if (m_numPeaks <= 0) close--; 	
		while( close >= m_msLevel)
		{
			m_fout << "</scan>" << lf;
			close--;
		}
		m_msLevelPrevScan = m_msLevel-1;
	}
	else
	{	// If first scan, save the level so that at the end of the XML file
		// we know how to close all the scan elements
		m_msLevelFirstScan = m_msLevel;
	}

	
	// get the precursorMz 
	if( m_msLevel > 1 )
	{
		bool isInaccurate = false;
		// first the precursorMz
		m_precursorMz = m_aRawfile.GetPrecursorMz(m_scanNum);
		

		if( m_precursorMz != 0 )
		{
			M_accurateMasses++;
		}
		else
		{ // the OCX returned 0
			
			cerr << "The OCX returned 0 precursor mass." << endl;
		}


		m_precursorCharge = 0;
		M_charges[m_precursorCharge]++;
			    
	}

//	if (m_numPakets == 0) // scan is empty
//	{
//		m_numPakets = 1;
	
//		m_pEncoded = new char [ 15 ];
//		// No need to do byte ordering or base 64 since we save a single 0 0 pair
//		strcpy( m_pEncoded , "AAAAAAAAAAA=\0" );
//	}

}

void XmlOut::m_getFilterField( int fieldNumber , char* szTemp , char* retValue )
{
	// Fields are separted by a space
	// Fields are numbered starting from 0!
	char* fieldBegin;
	fieldBegin = szTemp;
	for( int n = 0 ; n < fieldNumber ; n++ )
	{
		fieldBegin = strstr( fieldBegin , " " );
		fieldBegin++ ;
	}
	
	if( (fieldNumber - m_fieldCorrection == 3) && (*fieldBegin == 'd') )
	{ // We are looking for the scan type which is the third field
	  // Unless this is a dependent scan (in which case the scanType
	  // is preceded by a d (e.g. dependent))
		fieldBegin += 2;		// skip "d "
	}

	char* fieldEnd;
	fieldEnd = strstr( fieldBegin , " " );
	
	int	offset = fieldEnd - fieldBegin;
	strncpy( retValue , fieldBegin , offset );
	retValue[offset] = '\0';
}

void XmlOut::m_addScanToIndex()
{
	if (m_numPeaks > 0) {
		// syncronize stream	
		m_fout.flush();	
		_lseeki64( m_idxFout , 0 , SEEK_END );
		// array is zero based and m_scanNum is 1 based!
		m_index[m_scanNum - 1 ] = _telli64( m_idxFout );
	}
	else {
		m_index[m_scanNum - 1 ] = -1;
	}
}

void XmlOut::m_writeXmlScan()
{
	char	lf = 0xA;

//	try 
//	{
		
		_variant_t varX;
		_variant_t varY;		
		SAFEARRAY *psaX = varX.parray;
		double *dblArrayX;
		SAFEARRAY *psaY = varY.parray;
		double *dblArrayY;
		long nArraySize = m_aRawfile.GetData(!m_centroid, m_scanNum, dblArrayX, dblArrayY, psaX ,psaY, varX,varY);	
		long intensityCutoffType = 0;	// No cutoff
		long intensityCutoffValue = 0;	// No cutoff
		long maxNumberOfPeaks = 0;		// Return all data peaks
		BOOL centroidResult = FALSE;		// No centroiding of the results
		double centroidPeakWidth = 0;	// No centroiding
		void* pDataNetwork;
	
		if( (pDataNetwork =  malloc( ((2 * nArraySize) * sizeof(float)) )) == NULL)
		{
			cerr << "Cannot allocate memory!\n";
			exit(1);
		}
	
		char* m_pEncoded;
		// We add 5 bytes for the the trailing == and the /0
		m_pEncoded = (char *) new char [(  (((2 * nArraySize) * sizeof(float)) / 3) * 4 + 5)];
	
		float* dMass = new float;
		float* dNextMass = new float;
		float* dIntensity = new float;
		long n=0;
		long numPeaks = 0;	
		
		float totIonCurrent = 0;
		m_numPeaks = nArraySize;
		m_addScanToIndex();		
		m_lowMass= dblArrayX[0];
		m_highMass = dblArrayX[nArraySize-1];
		m_RT = m_aRawfile.GetRetentionTime(m_scanNum); 
		
		long peakStart = 0;
		long peakWidth = 0;
		float tempMass = 0;
		float totInt = 0;
		
		if (FALSE && m_centroid && m_msLevel > 1) {
			for ( long j=0; j<nArraySize; j++ ) {
				*dMass = (float) dblArrayX[j];
				*dNextMass = (float) dblArrayX[j+1];
				*dIntensity = (float) dblArrayY[j];			
					
				if (*dNextMass - *dMass < 0.05) {
					peakWidth++;
					if (peakWidth = 2) {
						peakStart = j;	
					}
				}
				else if (peakStart > 0) {
					while (peakStart < j) {
						tempMass += (*dMass) * (*dIntensity);
						totInt += (*dIntensity);
						peakStart++;
					}
					(unsigned int) ((unsigned int *)pDataNetwork)[n] = (unsigned int) htonl( (unsigned int) (tempMass / totInt) );
					n++;
					(unsigned int) ((unsigned int *)pDataNetwork)[n] = (unsigned int) htonl( (unsigned int) totInt );
					n++;
					totIonCurrent += totInt;
					numPeaks++;
					peakStart = 0;
					peakWidth = 0;
					tempMass = 0;
					totInt = 0;
				}
				else {
					peakWidth = 0;
				}
			}
		}
		else {
			for( long j=0; j<nArraySize; j++ )
			{
				// Using the vectors is a little more complicated and slow, 
				// but the conversion does not work directly
				*dMass = (float) dblArrayX[j];
				*dIntensity = (float) dblArrayY[j];

				// Convert from host to network
				(unsigned int) ((unsigned int *)pDataNetwork)[n] = (unsigned int) htonl( *(unsigned int*) dMass );
			//	(unsigned int) ((unsigned int *)pDataNetwork)[n] = (unsigned int) *dMass ;
				n++;
				(unsigned int) ((unsigned int *)pDataNetwork)[n] = (unsigned int) htonl( *(unsigned int*) dIntensity );
			//	(unsigned int) ((unsigned int *)pDataNetwork)[n] = (unsigned int) *dIntensity ;
				n++;
			}
		}

		// Free memory
		delete dMass;
		delete dNextMass;
		delete dIntensity;

		// base64 encode
		int encodedLength = b64_encode( m_pEncoded, (const unsigned char *) pDataNetwork, 
										( nArraySize * 2 * sizeof(float)));
		m_pEncoded[encodedLength] = '\0';
		// Free memory
		free( pDataNetwork );
		if (nArraySize > 0) {
			// scan header info	
			m_fout	<< "<scan num=\"" << m_scanNum << "\"" << lf
					<< "      msLevel=\"" << (int) m_msLevel << "\"" << lf 
					<< "      peaksCount=\"" << nArraySize << "\"" << lf  
					<< "      lowMz=\"" << m_lowMass << "\"" << lf
					<< "      highMz=\"" << m_highMass << "\"" << lf
					<< "      retentionTime=\"PT" << m_RT << "S\""
					<< ">" << lf;
				
//			<< "        peaksCount=\"" << m_numPakets/2 << "\"" << lf
//			<< "        polarity=\"" << m_polarity << "\"" << lf
//			<< "        scanType=\"" << m_scanType << "\"" << lf
//			<< "        retentionTime=\"PT" << m_RT*60 << "S\"" << lf;
		
			if( m_msLevel > 1 )
			{
//		m_fout << "        collisionEnergy=\"" << m_CE << "\""  << lf;
			}		
//	m_fout	<< "        lowMz=\"" << m_lowMass << "\"" << lf
//			<< "        highMz=\"" << m_highMass << "\"" << lf
//			<< "	basePeakMz=\"" << m_basePeakMass << "\"" << lf
//			<< "	basePeakIntensity=\"" << m_basePeakIntensity << "\"" << lf
//			<< "	totIonCurrent=\"" << m_TIC
//		m_fout  << "\">" << lf;

			// precusor information
			if( m_msLevel > 1 )
			{
				m_precursorIntensity = 0;
				m_fout	<< "    <precursorMz precursorIntensity=\"" << m_precursorIntensity << "\"";
			//if (m_precursorCharge > 0)
			//	m_fout << " precursorCharge=\"" << m_precursorCharge << "\"";
				m_fout	<< ">";
				m_fout	<< setprecision(10) << m_precursorMz << setprecision(6) << "</precursorMz>" << lf;
			}
	
			m_fout	<< "    <peaks precision=\"32\"" << lf
					<< "           byteOrder=\"network\"" << lf
					<< "           pairOrder=\"m/z-int\">"
					<< m_pEncoded
					<< "</peaks>" << endl;
			// We don't close the scan element untill we are sure if the next scan will be dependent on this one or not
			// We save the params we are going to need to see if the next scan element will be a children of the current one
			m_msLevelPrevScan = m_msLevel;	

		}

		delete [] m_pEncoded;
		
			// Free memory
			//delete m_pEncodedPrevScan;
		
			//TODO put below last access
			//Unlock SAFEARRAYs
		SafeArrayUnaccessData(psaX);
		SafeArrayUnaccessData(psaY);

//	}
//	catch(_com_error e)
//	{
		//Any errors that occur in the wrapper could be displayed by this code
//		cout<<"Error: "<<e.Description()<<endl;
//	}
//	catch(...) {
//		cout<<"A non-COM exception has been thrown"<<endl;
//	}
	
	//SysFreeString( m_bstrFilter );
}

// Terminate the scan element afte the last scan element
void XmlOut::m_writeScanClosing()
{
	char	lf = 0xA;
	//if (m_numPeaks <= 0)
	//	m_msLevelPrevScan--;

	while( m_msLevelPrevScan >= m_msLevelFirstScan)
	{
		m_fout << "</scan>" << lf;
		m_msLevelPrevScan--;
	}
}


void XmlOut::m_writeXmlIndex()
{
	char	lf = 0xA;
	__int64	indexOffset;
	char    buffer[100];

	m_fout << "</msRun>" << lf;

	// Syncronize the stream
	m_fout.flush();
	_lseeki64( m_idxFout , 0 , SEEK_END );
	// Save the offset for the indexOffset element
	indexOffset = _telli64( m_idxFout );


	m_fout << "<index name=\"scan\">" << lf;
	for( int n = 0 ; n <= (m_lastScan - m_firstScan) ; n++ )
	{
		if (m_index[n] > 0) {
			m_fout << "    <offset id=\"" << n+1 << "\">" << _i64toa(m_index[n], buffer, 10) << "</offset>" << lf;
		}
	}
	m_fout << "</index>" << lf;
	m_fout << "<indexOffset>" << _i64toa(indexOffset, buffer, 10) << "</indexOffset>" << lf;
}

void XmlOut::m_writeXmlSha1()
{
	char	lf = 0xA;

	// Sha1 sum of the mzXML file goes till the end of the opening tag of the sha1 element
	m_fout << "  <sha1>";

	// Make sure everything is printed before starting calculation.
	m_fout.flush();

	// Clean up and calculate
	cout << "Calculating sha1-sum of mzXML" << endl;
	m_pReport[0] = 0;
	m_Csha1.Reset();

	if( !(m_Csha1.HashFile( m_xmlName )) )
    {
      cerr << "Cannot open file " << m_xmlName << " for sha-1 calculation\n";
      exit(-1);		// Cannot open file
    }
	m_Csha1.Final();
	m_Csha1.ReportHash( m_pReport, CSHA1::REPORT_HEX );

	m_fout << m_pReport << "</sha1>" << lf;
	m_fout << "</mzXML>" << lf;

	// Distributions for LTQ-FT tuning
	// Print accurate v. inaccurate masses
	if (M_accurateMasses > 0)
	{
		cout << "Inaccurate Masses: " << M_inaccurateMasses << "\n";
		cout << "Accurate Masses: " << M_accurateMasses << "\n";
	}

	// Print charge state distribution
	if (M_charges[0] != M_accurateMasses + M_inaccurateMasses)
	{
		for (int i = 0; i < _countof(M_charges); i++)
		{
			if (M_charges[i] > 0)
				cout << "Charge " << i << ": " << M_charges[i] << "\n";
		}
	}

	cout << "done\n";
}