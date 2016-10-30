/*
Program       : AgilentRAW.cpp for Agilent2mzXML
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
// Machine generated IDispatch wrapper class(es) created with ClassWizard
#include "AgilentRAW.h"

/////////////////////////////////////////////////////////////////////////////
// AgilentRAW properties

/////////////////////////////////////////////////////////////////////////////
// AgilentRAW operations

void AgilentRAW::Init() {
	ADAL::IMSAnalysisPtr pMSAnalysis(__uuidof(ADAL::MSAnalysis));
	_pMSAnalysis = pMSAnalysis;
}
bool AgilentRAW::Open(_bstr_t filePath)
{	
	try{
		_pMSAnalysis->Open(filePath);
	}
	catch(_com_error e)
	{
		cout << "ERROR: " << e.Description() << endl;
		return false;
	}
	
	//Pointers to MSSpectrum objects
	_pSpectra = _pMSAnalysis->MSSpectra;
	return true;
}

double AgilentRAW::GetPrecursorMz(long idx) {
	_pSpectrum = _pSpectra->Item[idx];
	_variant_t varPrecursors;
			
	long lPrecCount = 0;
	lPrecCount = _pSpectrum->GetPrecursors(&varPrecursors);
	SAFEARRAY *psaPrecursors = varPrecursors.parray;
	double *dblArrayPrecursors;
	SafeArrayAccessData(psaPrecursors, reinterpret_cast<void**>(&dblArrayPrecursors));
	double ret = dblArrayPrecursors[0];
	SafeArrayUnaccessData(psaPrecursors);
	return ret;

}
 
double AgilentRAW::GetRetentionTime(long idx) {
	_pSpectrum = _pSpectra->Item[idx];
	return _pSpectrum->RetentionTime;

}

long AgilentRAW::GetData(BOOL profile, long idx, double* &dblArrayX, double* &dblArrayY, SAFEARRAY* &psaX ,SAFEARRAY* &psaY, _variant_t &varX, _variant_t &varY) {
	//Declare SAFEARRAYs		
	//_variant_t varX;	
	//_variant_t varY;

	long lNumberOfValuePairs=0;
	lNumberOfValuePairs = _pSpectra->Item[idx]->GetXYValues(profile, &varX, &varY);
	
	if (profile && lNumberOfValuePairs == 0) {
		cerr << "WARNING: The data is not profile!" << endl;
	}
		

	//Convert SAFEARRAYs to useable form (C++ array)
	//SAFEARRAY is locked for our exclusive use while the C++ array exists
	psaX = varX.parray;
	//double *dblArrayX;
	SafeArrayAccessData(psaX, reinterpret_cast<void**>(&dblArrayX));

	psaY = varY.parray;
	//double *dblArrayY;
	SafeArrayAccessData(psaY, reinterpret_cast<void**>(&dblArrayY));

	return lNumberOfValuePairs;
}


ADAL::IMSSpectrumPtr AgilentRAW::GetSpectrumAtIndex(long idx) {
	return _pSpectra->Item[idx];
}

void AgilentRAW::GetInstModel(BSTR* pbstrInstModel) 
{
	

}
void AgilentRAW::GetFirstSpectrumNumber(long* pnFirstSpectrum) {
	*pnFirstSpectrum = 1;
}

void AgilentRAW::GetLastSpectrumNumber(long* pnLastSpectrum) {
	*pnLastSpectrum = _pSpectra->Count;
}

int AgilentRAW::GetMSLevelForSpectrumIdx(long idx) {
	return _pSpectra->Item[idx]->MSMSLevel + 1;	
}


void AgilentRAW::Close() {

}