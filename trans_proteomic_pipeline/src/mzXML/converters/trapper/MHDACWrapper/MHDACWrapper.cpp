// class: MHDACWrapper 
// purpose: apache-licensed wrapper class, in order to insulate LGPL code from directly calling into propretrary Agilent code.  The author's intention is to remove any possiblity of implication of reciprocal open-source code obligations from Agilent.  More specifically, we want to use the MHDAC system without any implied requirement that the MHDAC be open-source or redistributable in any way.  This arraignment was jointly determined and agreed upon between the author of this code and Agilent.
// 
// Copyright 2008 Natalie Tasman (ntasman (a t) systemsbiology (d o t) org)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#include "MHDACWrapper.h"
#include "Comdef.h"

using namespace std;



void MHDACWrapper::InitCOM(void) {
	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0)) {
		cerr << "Fatal Error: MFC initialization failed" << endl;
		throw runtime_error(string("Fatal Error: MFC initialization failed"));
	}
	// Initializes the COM library on the current thread 
	// and identifies the concurrency model as single-thread 
	// apartment (STA)

	CoInitialize(NULL);
}



MH::IMsdrDataReader* MHDACWrapper::CreateMsdrDataReader(void) {
	MH::IMsdrDataReader* pMSDataReader = NULL;
	COMCHECK(CoCreateInstance(
		MH::CLSID_MassSpecDataReader,
		NULL, 
		CLSCTX_INPROC_SERVER ,	
		MH::IID_IMsdrDataReader, 
		(void**)&pMSDataReader),
		"ERROR - CoCreateInstance failed.");
	return pMSDataReader;
}


VARIANT_BOOL MHDACWrapper::OpenDataFile(MH::IMsdrDataReader* pMSDataReader, const string& fileName) {
	VARIANT_BOOL success;
	CComBSTR fileNameBSTR(fileName.c_str());
	COMCHECK(pMSDataReader->OpenDataFile(
		fileNameBSTR,
		&success),
		"OpenDataFile");
	return success;
}


MH::BDA::IBDAFileInformation* MHDACWrapper::GetFileInformation(MH::IMsdrDataReader* pMSDataReader) {
	MH::BDA::IBDAFileInformation* pFileInfo = NULL;
	COMCHECK(pMSDataReader->get_FileInformation(&pFileInfo),
		"error getting file information");
	return pFileInfo;
}


DATE MHDACWrapper::GetAcquisitionTime(MH::BDA::IBDAFileInformation* pFileInfo) {
	DATE date;
	COMCHECK(pFileInfo->get_AcquisitionTime(&date),
		"error getting acquisition time");
	return date;
}


BSTR MHDACWrapper::GetVersion(MH::IMsdrDataReader* pMSDataReader) {
	BSTR versionBSTR;
	COMCHECK(pMSDataReader->get_Version(&versionBSTR),
		"error getting version");
	return versionBSTR;
}


MH::BDA::IBDAMSScanFileInformation* MHDACWrapper::GetMSScanFileInformation(MH::IMsdrDataReader* pMSDataReader) {
	MH::BDA::IBDAMSScanFileInformation* pScanInfo = NULL;
	COMCHECK(pMSDataReader->get_MSScanFileInformation(&pScanInfo),
		"error getting MSScanFileInformation");
	return pScanInfo;
}


__int64 MHDACWrapper::GetTotalScansPresent(MH::BDA::IBDAMSScanFileInformation* pScanInfo) {
	__int64 totalNumScans = 0;
	COMCHECK(pScanInfo->get_TotalScansPresent(&totalNumScans),
		"error getting total scans present");
	return totalNumScans;
}


VARIANT_BOOL MHDACWrapper::CheckDualMode(MH::BDA::IBDAMSScanFileInformation* pScanInfo) {
	VARIANT_BOOL bDualMode;
	COMCHECK(pScanInfo->IsMultipleSpectraPerScanPresent(&bDualMode),
		"error in checking if file is dual-mode");
	return bDualMode;
}


MH::MSStorageMode MHDACWrapper::GetSpectraFormat(MH::BDA::IBDAMSScanFileInformation* pScanInfo) {
	MH::MSStorageMode storageMode;
	COMCHECK(pScanInfo->get_SpectraFormat(&storageMode),
		"error getting SpectraFormat");
	return storageMode;
}


MH::DeviceType MHDACWrapper::GetDeviceType(MH::BDA::IBDAMSScanFileInformation* pScanInfo)  {
	MH::DeviceType devType;
	COMCHECK(pScanInfo->get_DeviceType(&devType),
		"error getting device type");
	return devType;
}


BSTR MHDACWrapper::GetDeviceName(MH::BDA::IBDAFileInformation* pFileInfo, MH::DeviceType devType) {
	BSTR devName = NULL;
	COMCHECK(pFileInfo->GetDeviceName(devType, &devName),
		"error getting device name");
	return devName;
}


MH::IonizationMode MHDACWrapper::GetIonModes(MH::BDA::IBDAMSScanFileInformation* pScanInfo) {
	MH::IonizationMode ionMode;
	COMCHECK(pScanInfo->get_IonModes(&ionMode),
		"error getting ion modes");
	return ionMode;
}


MH::BDA::IBDAChromFilter* MHDACWrapper::CreateChromFilter(void) {
	MH::BDA::IBDAChromFilter* pChromFilter = NULL;
	COMCHECK(
		CoCreateInstance(MH::BDA::CLSID_BDAChromFilter, 
		NULL, 
		CLSCTX_INPROC_SERVER ,	
		MH::BDA::IID_IBDAChromFilter, 
		(void**)&pChromFilter),
		"couldn't create chromfilter");
	return pChromFilter;
}


void MHDACWrapper::SetChromatogramType(MH::BDA::IBDAChromFilter* pChromFilter, MH::ChromType chromType){
	COMCHECK(pChromFilter->put_ChromatogramType(MH::ChromType_TotalIon),
		"error setting chromatogram type");
}

void MHDACWrapper::SetDoCycleSum(MH::BDA::IBDAChromFilter* pChromFilter, VARIANT_BOOL doCycleSum){
	COMCHECK(pChromFilter->put_DoCycleSum(doCycleSum),
		"error setting chromatogram type");
}


void MHDACWrapper::SetDesiredChromMSStorageType(MH::BDA::IBDAChromFilter* pChromFilter, MH::DesiredMSStorageType storageType){
	COMCHECK(pChromFilter->put_DesiredMSStorageType(storageType),
		"error setting chromatogram storage type");
}


SAFEARRAY * MHDACWrapper::GetChromatogram(MH::IMsdrDataReader* pMSDataReader, MH::BDA::IBDAChromFilter* pChromFilter) {
	SAFEARRAY *pSafeArrayChromData;
	COMCHECK(pMSDataReader->GetChromatogram(pChromFilter, &pSafeArrayChromData),
		"Error getting chromatogram");
	return pSafeArrayChromData;
}


long MHDACWrapper::GetTotalChromDataPoints(MH::BDA::IBDAChromData* pChromData) {
	long dataPoints=0;
	COMCHECK(pChromData->get_TotalDataPoints(&dataPoints),
		"Error getting total data points from TIC.");
	return dataPoints;
}


SAFEARRAY* MHDACWrapper::GetChromDataXArray(MH::BDA::IBDAChromData* pChromData) {
	SAFEARRAY* pSafeArrayRetentionTimes;
	COMCHECK(pChromData->get_xArray(&pSafeArrayRetentionTimes),
		"Error getting XArray of TIC.");
	return pSafeArrayRetentionTimes;
}


SAFEARRAY* MHDACWrapper::GetChromDataYArray(MH::BDA::IBDAChromData* pChromData) {
	SAFEARRAY* pSafeArrayRetentionTimes;
	COMCHECK(pChromData->get_yArray(&pSafeArrayRetentionTimes),
		"Error getting YArray of TIC.");
	return pSafeArrayRetentionTimes;
}


MH::BDA::IBDAChromData* MHDACWrapper::GetTIC(MH::IMsdrDataReader* pMSDataReader){
	MH::BDA::IBDAChromData* pChromData = NULL;
	COMCHECK(pMSDataReader->GetTIC(&pChromData),
		"Error getting base peak chromatogram.");
	return pChromData;
}


MH::BDA::IBDASpecData* MHDACWrapper::GetSpectrum(MH::IMsdrDataReader* pMSDataReader,
												 double retentionTime,
												 MH::MSScanType scanType,
												 MH::IonPolarity ionPolarity,
												 MH::IonizationMode ionizationMode,
												 MH::IMsdrPeakFilter* filter,
												 VARIANT_BOOL* filterOnCentroid)
{
	MH::BDA::IBDASpecData* specData = NULL;
	COMCHECK(pMSDataReader->GetSpectrum_7(
		retentionTime,
		scanType,
		ionPolarity,
		ionizationMode,
		filter,
		*filterOnCentroid,
		&specData),
		"error getting spectrum");

	return specData;
}

SAFEARRAY* MHDACWrapper::GetProfileSpectrumfromMixedData(MH::IMsdrDataReader* pMSDataReader,
												 MH::BDA::IBDASpecFilter *specFilter)
{
	
	SAFEARRAY *pSpecDataArray;

	COMCHECK(pMSDataReader->GetSpectrum_5(specFilter, &pSpecDataArray), "error getting spectrum");
	return pSpecDataArray;
	
}

void MHDACWrapper::ConvertDataToMassUnits(MH::BDA::IBDASpecData* pSpecData) {
	
	COMCHECK(pSpecData->ConvertDataToMassUnits(),
		"error converting time to mass units");
}


MH::MSStorageMode  MHDACWrapper::GetMSStorageMode(MH::BDA::IBDASpecData* pSpecData) {
	MH::MSStorageMode spectrumMode;
	COMCHECK(pSpecData->get_MSStorageMode(&spectrumMode),
		"error getting spec. mode");
	return spectrumMode;
}


MH::IMsdrChargeStateAssignmentFilter* MHDACWrapper::CreateChargeStateAssignmentFilter(void) {
	MH::IMsdrChargeStateAssignmentFilter* pCsaFilter = NULL;
	COMCHECK(CoCreateInstance( MH::CLSID_MsdrChargeStateAssignmentFilter, NULL, 
		CLSCTX_INPROC_SERVER ,	
		MH::IID_IMsdrChargeStateAssignmentFilter, 
		(void**)&pCsaFilter),
		"CoCreateInstance for charge state assignment filter failed.");
	return pCsaFilter;
}


void MHDACWrapper:: SetCSAFMaxChargeStateLimit(MH::IMsdrChargeStateAssignmentFilter* pCsaFilter, VARIANT_BOOL pLimit){
	COMCHECK(pCsaFilter->put_LimitMaxChargeState(pLimit), "error");
}


void MHDACWrapper:: SetCSAFMaximumChargeState(MH::IMsdrChargeStateAssignmentFilter* pCsaFilter, int maxCharge){
	COMCHECK(pCsaFilter->put_MaximumChargeState(maxCharge),"error");

}


void MHDACWrapper:: SetCSAFRequirePeptideLikeAbundanceProfile(MH::IMsdrChargeStateAssignmentFilter* pCsaFilter, VARIANT_BOOL pRequire){
	COMCHECK(pCsaFilter->put_RequirePeptideLikeAbundanceProfile(pRequire), "error");
}


void MHDACWrapper:: SetCSAFAbsoluteTolerance(MH::IMsdrChargeStateAssignmentFilter* pCsaFilter, double absTol){ 
	COMCHECK(pCsaFilter->put_AbsoluteTolerance(absTol), "error");
}


void MHDACWrapper:: SetCSAFRelativeTolerance(MH::IMsdrChargeStateAssignmentFilter* pCsaFilter, double relTol){ 
	COMCHECK(pCsaFilter->put_RelativeTolerance(relTol), "error");
}


void MHDACWrapper:: Deisotope(MH::IMsdrDataReader* pMSDataReader, MH::BDA::IBDASpecData* pSpecData, MH::IMsdrChargeStateAssignmentFilter* pCsaf){
	COMCHECK(pMSDataReader->Deisotope(pSpecData, pCsaf), "Error in deisotoping");
}


long MHDACWrapper:: GetScanID(MH::BDA::IBDASpecData* pSpecData) {
	long scanID;
	COMCHECK(pSpecData->get_ScanId(&scanID),
		"get ScanId error");
	return scanID;
}


MH::IonPolarity MHDACWrapper::GetIonPolarity(MH::BDA::IBDASpecData* pSpecData) {
	MH::IonPolarity polarity;
	COMCHECK(pSpecData->get_IonPolarity(&polarity),
		"ionpolarity error");
	return polarity;
}


MH::MSScanType MHDACWrapper::GetMSScanType(MH::BDA::IBDASpecData* pSpecData) {
	MH::MSScanType scanType;
	COMCHECK(pSpecData->get_MSScanType(&scanType),"error getting scan type");
	return scanType;
}


SAFEARRAY* MHDACWrapper::GetPrecursorIon(MH::BDA::IBDASpecData* pSpecData, long* precursorCount) {
	SAFEARRAY* psaPrecursors = NULL;
	COMCHECK(pSpecData->GetPrecursorIon(precursorCount, &psaPrecursors), "Error getting Precursor Ion for spectrum");
	return psaPrecursors;
}


long MHDACWrapper::GetPrecursorCharge(MH::BDA::IBDASpecData* pSpecData, VARIANT_BOOL* success){
	long charge;
	COMCHECK(pSpecData->GetPrecursorCharge(&charge, success),
		"Error getting Precursor Charge for spectrum");
	return charge;
}


double MHDACWrapper::GetPrecursorIntensity(MH::BDA::IBDASpecData* pSpecData, VARIANT_BOOL* success){
	double intensity; 
	COMCHECK(pSpecData->GetPrecursorIntensity(&intensity, success),
		"error getting Precursor intensity for spectrum");
	return intensity;
}


long MHDACWrapper::GetParentScanID(MH::BDA::IBDASpecData* pSpecData) {
	long parentScanID;
	COMCHECK(pSpecData->get_ParentScanId(&parentScanID),
		"get ParentScanId error");
	return parentScanID;
}


double MHDACWrapper::GetCollisionEnergy(MH::BDA::IBDASpecData* pSpecData){
	double collisionEnergy;
										   COMCHECK(pSpecData->get_CollisionEnergy(&collisionEnergy), "error");
										   return collisionEnergy;
}


long MHDACWrapper::GetTotalSpectrumDataPoints(MH::BDA::IBDASpecData* pSpecData) {
	long numPeaks;
	COMCHECK(pSpecData->get_TotalDataPoints(&numPeaks), "Error getting number of peaks in spectrum");
	return numPeaks;
}


SAFEARRAY * MHDACWrapper::GetSpectrumXArray(MH::BDA::IBDASpecData* pSpecData) {
	SAFEARRAY *psaX = NULL;
	COMCHECK(pSpecData ->get_xArray(&psaX), "Error getting XArray data of spectrum");
	return psaX;
}


SAFEARRAY * MHDACWrapper:: GetSpectrumYArray(MH::BDA::IBDASpecData* pSpecData) {
	SAFEARRAY *psaY = NULL;
	COMCHECK(pSpecData ->get_yArray(&psaY), "Error getting YArray data of spectrum");
	return psaY;
}