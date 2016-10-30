/*

Program       : Pep3D
Author        : Xiao-jun Li <xli@systemsbiology.org>                                                      
Date          : 10.08.02 

Functions needed in Pep3D program.

Copyright (C) 2002 Xiao-jun Li

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

Xiao-jun Li
Institute for Systems Biology
401 Terry Avenue North 
Seattle, WA  98109  USA
xli@systemsbiology.org

*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "common/sysdepend.h"
#include "Pep3D_functions.h"
#include "cramp.hpp"
#include "common/spectStrct.h"


///////////////////////////////////////////////////////////////////
/*
  This function searches a string for a specific tage and returns the
  point at the beginning of the tag. 
*/
int searchTag(char * string, char * tag)
{
  int strLen = (int)strlen(string); // string length
  int tagLen = (int)strlen(tag); // tag length
  int match; // match index

  int i, k;

  for (i = 0; i <= strLen - tagLen; ++i) {
    k = 0;
    do {
      if (string[i+k] == tag[k]) match = 1;
      else match = 0;
      ++k;
    }
    while (match == 1 && k < tagLen);
    if (match == 1 && k == tagLen) 
      return i;
  }

  return -1;
}




/*
**************************************************************************************************************
void getMsSpect(spectStrct *msSpect, char *xmlFile, long scanNum[2])
**************************************************************************************************************
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
  This function gets the MS spectrum from .mzXML file.
*/
void getMsSpect(spectStrct *msSpect, char *xmlFile, int scanNum[2])
{
  void getCmbSpect(spectStrct *cmbSpect, 
		   int spectNum, spectStrct *spects, double *wghs);

  RAMPFILE *pFI;
  ramp_fileoffset_t *pScanIndex;
  int iLastScan;
  struct ScanHeaderStruct scanHeader;
  struct RunHeaderStruct runHeader;
  ramp_fileoffset_t indexOffset;

  spectStrct *spects;
  double *wghs;
  int spectNum;
  long i;
  RAMPREAL *pPeaks;
  int n;


  msSpect->size = -1;

  if ((pFI = rampOpenFile(xmlFile)) == NULL)
    {
      printf ("Could not open input file %s\n", xmlFile);
      fflush(stdout);
      return;
    }

  // Read the offset of the index
  indexOffset = getIndexOffset (pFI);
  
  // Read the scan index into a vector, get LastScan
  pScanIndex = readIndex(pFI, indexOffset, &iLastScan);

  fflush(stdout);
  readRunHeader(pFI, pScanIndex, &runHeader, iLastScan);
  
  // initialize
  scanNum[0] = scanNum[0] > 1 ? scanNum[0] : 1;
  scanNum[1] = scanNum[1] < iLastScan ? scanNum[1] : iLastScan;
  spectNum = scanNum[1] - scanNum[0] + 1;
  if(spectNum < 1){
    printf("Invalid scan number: %d -- %d (Full scan range: 1 -- %d)!<br/>\n", 
	   scanNum[0], scanNum[1], iLastScan);
    fflush(stdout);
    rampCloseFile(pFI);
    free (pScanIndex);
    return;    
  }

  spects = new spectStrct[spectNum];
  spectNum = 0;
  for (i = scanNum[0]; i <= scanNum[1]; ++i) {
      fflush(stdout);
      readHeader (pFI, pScanIndex[i], &scanHeader);

      if (scanHeader.msLevel == 1 // msScan
	  && scanHeader.peaksCount > 0) {                         
	
	spects[spectNum].size = scanHeader.peaksCount;
	spects[spectNum].xval = (double *) calloc(spects[spectNum].size, sizeof(double));
	spects[spectNum].yval = (double *) calloc(spects[spectNum].size, sizeof(double));

	fflush(stdout);
	pPeaks = readPeaks (pFI, pScanIndex[i]);

	spects[spectNum].size = 0;
	n = 0;
	while (pPeaks[n] != -1)
	  {
	    spects[spectNum].xval[spects[spectNum].size] = pPeaks[n];
	    n++;
	    spects[spectNum].yval[spects[spectNum].size] = pPeaks[n];
	    n++;
	    ++(spects[spectNum].size);
	  }
	free (pPeaks);
	
	if(spects[spectNum].size > 0)
	  ++spectNum;
	else
	  spects[spectNum].clear();
      } // if (scanHeader.msLevel == 1 // msScan

  } // for (i = scanNum[0]; i <= scanNum[1]; ++i) {
  rampCloseFile(pFI);
  free (pScanIndex);

  if(spectNum > 0) {
    wghs = (double *) calloc(spectNum, sizeof(double));
    for (i = 0; i < spectNum; ++i)
      wghs[i] = 1.;
    getCmbSpect(msSpect, spectNum, spects, wghs);
    free(wghs);
  }
  else {
    printf("Cannot get the MS spectrum.<br/>\n");
    fflush(stdout);
  }
  
  delete []spects;
  
  return;
}


/*
**************************************************************************************************************
void getCidSpect(double *mz, double *et, spectStrct *cidSpect, char *xmlFile, long scanNum[2])
**************************************************************************************************************
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
  This function gets the cid spectrum from .mzXML file.
*/
void getCidSpect(double *mz, double *et, spectStrct *cidSpect, char *xmlFile, int scanNum[2])
{
  void getCmbSpect(spectStrct *cmbSpect, 
		   int spectNum, spectStrct *spects, double *wghs);

  RAMPFILE *pFI;
  ramp_fileoffset_t *pScanIndex;
  int iLastScan;
  struct ScanHeaderStruct scanHeader;
  struct RunHeaderStruct runHeader;
  ramp_fileoffset_t indexOffset;

  spectStrct *spects;
  double *wghs;
  int spectNum;
  long i;
  RAMPREAL *pPeaks;
  int n;


  cidSpect->size = -1;

  if ((pFI = rampOpenFile(xmlFile)) == NULL)
    {
      printf ("Could not open input file %s\n", xmlFile);
      fflush(stdout);
      return;
    }
  
  //  printf("DDS: %s <br/>\n", xmlFile); fflush(stdout);
  // Read the offset of the index
  indexOffset = getIndexOffset (pFI);
    
  fflush(stdout);
  // Read the scan index into a vector, get LastScan
  pScanIndex = readIndex (pFI, indexOffset, &iLastScan);
  
  //  printf("DDS: %s <br/>\n", xmlFile); fflush(stdout);
  
  fflush(stdout);
  readRunHeader(pFI, pScanIndex, &runHeader, iLastScan);
  //DDS: BREAKS before this line
  //  printf("DDS: %s <br/>\n", xmlFile); fflush(stdout);

  // initialize
  scanNum[0] = scanNum[0] > 1 ? scanNum[0] : 1;
  scanNum[1] = scanNum[1] < iLastScan ? scanNum[1] : iLastScan;
  spectNum = scanNum[1] - scanNum[0] + 1;
  if(spectNum < 1){
    printf("Invalid scan number: %d -- %d (Full scan range: 1 -- %d)!<br/>\n", 
	   scanNum[0], scanNum[1], iLastScan);
    fflush(stdout);
    rampCloseFile(pFI);
    free (pScanIndex);
    return;    
  }
  spects = new spectStrct[spectNum];
  *mz = 0.;
  *et = 0.;
  spectNum = 0;
  for (i = scanNum[0]; i <= scanNum[1]; ++i) {
    
      readHeader (pFI, pScanIndex[i], &scanHeader);

      if (scanHeader.msLevel == 2 // cidScan
	  && scanHeader.peaksCount > 0) {                         
	
	*mz += scanHeader.precursorMZ;
	*et += scanHeader.retentionTime/60;

	spects[spectNum].size = scanHeader.peaksCount;
	spects[spectNum].xval = (double *) calloc(spects[spectNum].size, sizeof(double));
	spects[spectNum].yval = (double *) calloc(spects[spectNum].size, sizeof(double));

	pPeaks = readPeaks (pFI, pScanIndex[i]);

	spects[spectNum].size = 0;
	n = 0;
	while (pPeaks[n] != -1)
	  {
	    spects[spectNum].xval[spects[spectNum].size] = pPeaks[n];
	    n++;
	    spects[spectNum].yval[spects[spectNum].size] = pPeaks[n];
	    n++;
	    ++(spects[spectNum].size);
	  }
	free (pPeaks);
	
	if(spects[spectNum].size > 0)
	  ++spectNum;
	else
	  spects[spectNum].clear();
      } // if (scanHeader.msLevel == 2 // cidScan

  } // for (i = scanNum[0]; i <= scanNum[1]; ++i) {
  rampCloseFile(pFI);
  free (pScanIndex);
  if(spectNum > 0) {
    *mz /= spectNum;
    *et /= spectNum;
    wghs = (double *) calloc(spectNum, sizeof(double));
    for (i = 0; i < spectNum; ++i)
      wghs[i] = 1.;
    getCmbSpect(cidSpect, spectNum, spects, wghs);
    free(wghs);
  }
  else {
    printf("Cannot get the CID spectrum.<br/>\n");
    fflush(stdout);
  }
  
  delete []spects;
  
  return;
}


//////////////////////////////////////////////////////////////////////////////
/*
  This function combines spectra.
*/
void getCmbSpect(spectStrct *cmbSpect, 
		 int spectNum, spectStrct *spects, double *wghs)
{
  spectStrct tmpSpect[2];
  int indx, indx1, indx2;
  double tmpWghs[2] = {1., 1.};
  int i;

  if (spectNum < 1)
    return;

  // single spectrum
  if(spectNum == 1) {
    *cmbSpect = spects[0];
    if(wghs[0] != 1.) {
      for(i = 0; i < cmbSpect->size; ++i)
	cmbSpect->yval[i] *= wghs[0];
    }
    return;
  } // if(spectNum == 1) {


  // 2 spectra
  if(spectNum == 2) {
    tmpSpect[0].setsize(spects[0].size + spects[1].size);

    indx1 = 0;
    indx2 = 0;
    indx = 0;
    while(indx1 < spects[0].size || indx2 < spects[1].size) {
      
      if(indx1 >= spects[0].size){
	tmpSpect[0].xval[indx] = spects[1].xval[indx2];
	tmpSpect[0].yval[indx] = spects[1].yval[indx2]*wghs[1];      
	++indx2;
	++indx;
      }
      else if (indx2 >= spects[1].size) {
	tmpSpect[0].xval[indx] = spects[0].xval[indx1];
	tmpSpect[0].yval[indx] = spects[0].yval[indx1]*wghs[0];      
	++indx1;
	++indx;
      }
      else if(spects[0].xval[indx1] == spects[1].xval[indx2]) {
	tmpSpect[0].xval[indx] = spects[0].xval[indx1];
	tmpSpect[0].yval[indx] = spects[0].yval[indx1]*wghs[0] 
	  + spects[1].yval[indx2]*wghs[1];      
	++indx1;
	++indx2;
	++indx;
      }
      else if(spects[0].xval[indx1] < spects[1].xval[indx2]) {
	tmpSpect[0].xval[indx] = spects[0].xval[indx1];
	tmpSpect[0].yval[indx] = spects[0].yval[indx1]*wghs[0];      
	++indx1;
	++indx;
      }
      else {
	tmpSpect[0].xval[indx] = spects[1].xval[indx2];
	tmpSpect[0].yval[indx] = spects[1].yval[indx2]*wghs[1];      
	++indx2;
	++indx;
      }
    } // while(index1 < spects[0].size && index2 < spects[1].size) {
    tmpSpect[0].size = indx;
    
    *cmbSpect = tmpSpect[0];

    return;
  }

  // at least three spectra
  indx1 = spectNum/2;
  indx2 = spectNum - spectNum/2;
  getCmbSpect(&(tmpSpect[0]), indx1, spects, wghs);
  getCmbSpect(&(tmpSpect[1]), indx2, spects+indx1, wghs+indx1);
  getCmbSpect(cmbSpect, 2, tmpSpect, tmpWghs);
  
  return;
}

