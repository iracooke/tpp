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


#pragma once
#include <string>
#include "stdafx.h"

using std::string;

namespace MH {
#import "BaseCommon.tlb" raw_interfaces_only, no_namespace, named_guids
#import "BaseDataAccess.tlb" raw_interfaces_only, rename_namespace("BDA"), named_guids
#import "MassSpecDataReader.tlb" raw_interfaces_only, no_namespace, named_guids
}; // namespace MH



#define COMCHECK(comcall, errMsg) \
{ \
	HRESULT hr = comcall; \
	if (hr != S_OK) { \
	cerr << "ERROR at " << __FILE__ << ", " << __LINE__ << ":" << endl; \
	cerr << "HR = " << hr << endl; \
	cerr << errMsg << endl; \
	throw runtime_error(string(errMsg)); \
	} \
}



// what kind of mode is the file?
typedef enum {
	MIXED,
	PROFILE_ONLY,
	PEAKPICKED_ONLY, // centroided
} SpectraModeType;



class MHDACWrapper {
public:

	MHDACWrapper(){}
	~MHDACWrapper() {}

	void InitCOM(void);  // should be called before anything else

	MH::IMsdrDataReader* CreateMsdrDataReader(void);
	MH::BDA::IBDAFileInformation* GetFileInformation(MH::IMsdrDataReader* MsdrDataReader);
	MH::BDA::IBDAMSScanFileInformation* GetMSScanFileInformation(MH::IMsdrDataReader* pMSDataReader);


	// other data access

	// - general file info
	VARIANT_BOOL OpenDataFile(MH::IMsdrDataReader* MsdrDataReader, const string& fileName);
	DATE GetAcquisitionTime(MH::BDA::IBDAFileInformation* pFileInfo);
	BSTR GetVersion(MH::IMsdrDataReader* MsdrDataReader);
	__int64 GetTotalScansPresent(MH::BDA::IBDAMSScanFileInformation* pScanInfo);
	VARIANT_BOOL CheckDualMode(MH::BDA::IBDAMSScanFileInformation* pScanInfo);
	MH::MSStorageMode GetSpectraFormat(MH::BDA::IBDAMSScanFileInformation* pScanInfo);
	MH::DeviceType GetDeviceType(MH::BDA::IBDAMSScanFileInformation* pScanInfo);
	BSTR GetDeviceName(MH::BDA::IBDAFileInformation* pFileInfo, MH::DeviceType devType);
	MH::IonizationMode GetIonModes(MH::BDA::IBDAMSScanFileInformation* pScanInfo);

	// - chromatogram
	MH::BDA::IBDAChromFilter* CreateChromFilter(void);
	void SetChromatogramType(MH::BDA::IBDAChromFilter* pChromFilter, MH::ChromType chromType);
	void SetDoCycleSum(MH::BDA::IBDAChromFilter* pChromFilter, VARIANT_BOOL doCycleSum);
	void SetDesiredChromMSStorageType(MH::BDA::IBDAChromFilter* pChromFilter, MH::DesiredMSStorageType storageType);
	SAFEARRAY * GetChromatogram(MH::IMsdrDataReader* pMSDataReader, MH::BDA::IBDAChromFilter* pChromFilter);
	long GetTotalChromDataPoints(MH::BDA::IBDAChromData* pChromData);
	SAFEARRAY* GetChromDataXArray(MH::BDA::IBDAChromData* pChromData);
	SAFEARRAY* GetChromDataYArray(MH::BDA::IBDAChromData* pChromData);
	MH::BDA::IBDAChromData* GetTIC(MH::IMsdrDataReader* MsdrDataReader);


	// spectra 
	MH::BDA::IBDASpecData* GetSpectrum(MH::IMsdrDataReader* pMSDataReader,
		double retentionTime,
		MH::MSScanType scanType,
		MH::IonPolarity ionPolarity,
		MH::IonizationMode ionizationMode,
		MH::IMsdrPeakFilter* filter,
		VARIANT_BOOL* filterOnCentroid);
	
	SAFEARRAY* MHDACWrapper::GetProfileSpectrumfromMixedData(MH::IMsdrDataReader* pMSDataReader,
												 MH::BDA::IBDASpecFilter *specFilter);

	void ConvertDataToMassUnits(MH::BDA::IBDASpecData* pSpecData);

	long GetTotalSpectrumDataPoints(MH::BDA::IBDASpecData* pSpecData);
	SAFEARRAY * GetSpectrumXArray(MH::BDA::IBDASpecData* pSpecData);
	SAFEARRAY * GetSpectrumYArray(MH::BDA::IBDASpecData* pSpecData);

	long GetScanID(MH::BDA::IBDASpecData* pSpecData);
	MH::MSStorageMode GetMSStorageMode(MH::BDA::IBDASpecData* pSpecData);
	MH::IonPolarity GetIonPolarity(MH::BDA::IBDASpecData* pSpecData);
	MH::MSScanType GetMSScanType(MH::BDA::IBDASpecData* pSpecData);

	// precursor info
	long GetParentScanID(MH::BDA::IBDASpecData* pSpecData);
	double GetCollisionEnergy(MH::BDA::IBDASpecData* pSpecData);
	SAFEARRAY* GetPrecursorIon(MH::BDA::IBDASpecData* pSpecData, long* precursorCount);
	long GetPrecursorCharge(MH::BDA::IBDASpecData* pSpecData, VARIANT_BOOL* sucess);
	double GetPrecursorIntensity(MH::BDA::IBDASpecData* pSpecData, VARIANT_BOOL* sucess);

	// deisotoping spectra
	MH::IMsdrChargeStateAssignmentFilter* CreateChargeStateAssignmentFilter(void);
	void SetCSAFMaxChargeStateLimit(MH::IMsdrChargeStateAssignmentFilter* pCsaFilter, VARIANT_BOOL pLimit);
	void SetCSAFMaximumChargeState(MH::IMsdrChargeStateAssignmentFilter* pCsaFilter, int maxCharge);
	void SetCSAFRequirePeptideLikeAbundanceProfile(MH::IMsdrChargeStateAssignmentFilter* pCsaFilter, VARIANT_BOOL pRequire);
	void SetCSAFAbsoluteTolerance(MH::IMsdrChargeStateAssignmentFilter* pCsaFilter, double absTol);
	void SetCSAFRelativeTolerance(MH::IMsdrChargeStateAssignmentFilter* pCsaFilter, double relTOl);
	void Deisotope(MH::IMsdrDataReader* pMSDataReader, MH::BDA::IBDASpecData* pSpecData, MH::IMsdrChargeStateAssignmentFilter* pCsaf);

private:
};