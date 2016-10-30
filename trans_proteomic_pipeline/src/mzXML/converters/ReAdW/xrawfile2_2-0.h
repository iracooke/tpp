﻿// Created by Microsoft (R) C/C++ Compiler Version 14.00.50727.762 (57819b21).
//
// (xrawfile2.tlh)
//
// C++ source equivalent of Win32 type library C:\Program Files\Xcalibur\system\programs\XRawfile2.dll
// From Xcalibur version 2.0.5
// compiler-generated file created 01/02/08 at 11:50:55 - DO NOT EDIT!

#pragma once
#pragma pack(push, 8)

#include <comdef.h>

namespace XRAWFILE2Lib {

//
// Forward references and typedefs
//

struct __declspec(uuid("5fe970a2-29c3-11d3-811d-00104b304896"))
/* LIBID */ __XRAWFILE2Lib;
enum __MIDL___MIDL_itf_XRawfile2_0000_0001;
enum __MIDL___MIDL_itf_XRawfile2_0000_0002;
enum __MIDL___MIDL_itf_XRawfile2_0000_0003;
enum __MIDL___MIDL_itf_XRawfile2_0000_0004;
enum MS_PacketTypes;
enum MS_Polarity;
enum MS_ScanData;
enum MS_Dep;
enum MS_Wideband;
enum MS_SourceCID;
enum MS_SourceCIDType;
enum MS_MSOrder;
enum MS_ScanType;
enum MS_TurboScan;
enum MS_IonizationMode;
enum MS_Corona;
enum MS_Detector;
enum MS_PrecursorEnergy;
struct MS_MassRange;
struct MS_ScanEvent;
struct MS_ScanIndex;
struct MS_UVScanIndex;
struct MS_DataPeak;
struct MS_PrecursorInfo;
struct __declspec(uuid("5fe970b1-29c3-11d3-811d-00104b304896"))
/* dual interface */ IXRawfile;
struct __declspec(uuid("5e256644-7300-481f-9d43-33d892bfd912"))
/* dual interface */ IXRawfile2;
struct __declspec(uuid("5e256644-7301-481f-9d43-33d892bfd912"))
/* dual interface */ IXRawfile3;
struct __declspec(uuid("55ea38b7-5419-4be4-9198-3e4d78e6a532"))
/* dual interface */ IXVirMS;
enum MS_DataTypes;
struct __declspec(uuid("1a2bf13f-4e2f-4e7d-9d67-435d5998312b"))
/* dual interface */ IXVirUV;
struct /* coclass */ XRawfile;
struct /* coclass */ XVirMS;
struct /* coclass */ XVirUV;

//
// Smart pointer typedef declarations
//

_COM_SMARTPTR_TYPEDEF(IXRawfile, __uuidof(IXRawfile));
_COM_SMARTPTR_TYPEDEF(IXRawfile2, __uuidof(IXRawfile2));
_COM_SMARTPTR_TYPEDEF(IXRawfile3, __uuidof(IXRawfile3));
_COM_SMARTPTR_TYPEDEF(IXVirMS, __uuidof(IXVirMS));
_COM_SMARTPTR_TYPEDEF(IXVirUV, __uuidof(IXVirUV));

//
// Type library items
//

enum __MIDL___MIDL_itf_XRawfile2_0000_0001
{
    MS_TRAILER_NOT_AVAILABLE = -1
};

enum __MIDL___MIDL_itf_XRawfile2_0000_0002
{
    MS_SCAN_TYPE_NOT_SPECIFIED = -1
};

enum __MIDL___MIDL_itf_XRawfile2_0000_0003
{
    MS_MAX_NUM_MASS_RANGES = 50
};

enum __MIDL___MIDL_itf_XRawfile2_0000_0004
{
    MS_MAX_MS_ORDER = 10
};

enum MS_PacketTypes
{
    MS_PacketTypes_PROF_SP_TYPE = 0,
    MS_PacketTypes_LR_SP_TYPE = 1,
    MS_PacketTypes_HR_SP_TYPE = 2,
    MS_PacketTypes_PROF_INDEX_TYPE = 3,
    MS_PacketTypes_COMP_ACC_SP_TYPE = 4,
    MS_PacketTypes_STD_ACC_SP_TYPE = 5,
    MS_PacketTypes_STD_UNCAL_SP_TYPE = 6,
    MS_PacketTypes_ACC_MASS_PROF_SP_TYPE = 7,
    MS_PacketTypes_DG_xAQR_TYPE = 8,
    MS_PacketTypes_DG_xAQR_INDEX_TYPE = 9,
    MS_PacketTypes_DG_xASR_TYPE = 10,
    MS_PacketTypes_DG_xASR_INDEX_TYPE = 11,
    MS_PacketTypes_CHANNEL_UV_TYPE = 12,
    MS_PacketTypes_MS_ANALOG_TYPE = 13,
    MS_PacketTypes_PROF_SP_TYPE2 = 14,
    MS_PacketTypes_LR_SP_TYPE2 = 15,
    MS_PacketTypes_PROF_SP_TYPE3 = 16,
    MS_PacketTypes_LR_SP_TYPE3 = 17
};

enum MS_Polarity
{
    MS_Negative = 0,
    MS_Positive = 1,
    MS_AnyPolarity = 2
};

enum MS_ScanData
{
    MS_Centroid = 0,
    MS_Profile = 1,
    MS_AcceptAnyScanData = 2
};

enum MS_Dep
{
    MS_NotDependent = 0,
    MS_Dependent = 1,
    MS_AcceptAnyDep = 2
};

enum MS_Wideband
{
    MS_WidebandOff = 0,
    MS_WidebandOn = 1,
    MS_AcceptAnyWideband = 2
};

enum MS_SourceCID
{
    MS_SourceCIDon = 0,
    MS_SourceCIDoff = 1,
    MS_AcceptAnySourceCID = 2
};

enum MS_SourceCIDType
{
    MS_SourceCIDTypeNoValue = 0,
    MS_SourceCIDTypeSingleValue = 1,
    MS_SourceCIDTypeRamp = 2,
    MS_SourceCIDTypeSIM = 3,
    MS_AcceptAnySourceCIDType = 4
};

enum MS_MSOrder
{
    MS_ng = -3,
    MS_nl = -2,
    MS_par = -1,
    MS_AcceptAnyMSorder = 0,
    MS_ms = 1,
    MS_ms2 = 2,
    MS_ms3 = 3,
    MS_ms4 = 4,
    MS_ms5 = 5,
    MS_ms6 = 6,
    MS_ms7 = 7,
    MS_ms8 = 8,
    MS_ms9 = 9,
    MS_ms10 = 10
};

enum MS_ScanType
{
    MS_Fullsc = 0,
    MS_Zoomsc = 1,
    MS_SIMsc = 2,
    MS_SRMsc = 3,
    MS_CRMsc = 4,
    MS_AcceptAnyScanType = 5,
    MS_Q1MSsc = 6,
    MS_Q3MSsc = 7
};

enum MS_TurboScan
{
    MS_TurboScanOn = 0,
    MS_TurboScanOff = 1,
    MS_AcceptAnyTurboScan = 2
};

enum MS_IonizationMode
{
    MS_ElectronImpact = 0,
    MS_ChemicalIonization = 1,
    MS_FastAtomBombardment = 2,
    MS_Electrospray = 3,
    MS_AtmosphericPressureChemicalIonization = 4,
    MS_Nanospray = 5,
    MS_Thermospray = 6,
    MS_FieldDesorption = 7,
    MS_MatrixAssistedLaserDesorptionIonization = 8,
    MS_GlowDischarge = 9,
    MS_AcceptAnyIonizationMode = 10
};

enum MS_Corona
{
    MS_CoronaOn = 0,
    MS_CoronaOff = 1,
    MS_AcceptAnyCorona = 2
};

enum MS_Detector
{
    MS_DetectorValid = 0,
    MS_AcceptAnyDetector = 1
};

enum MS_PrecursorEnergy
{
    MS_PrecursorEnergyValid = 0,
    MS_AcceptAnyPrecursorEnergy = 1
};

#pragma pack(push, 8)

struct MS_MassRange
{
    double dLowMass;
    double dHighMass;
};

#pragma pack(pop)

#pragma pack(push, 8)

struct MS_ScanEvent
{
    long bIsValid;
    enum MS_ScanData eScanData;
    enum MS_Polarity ePolarity;
    enum MS_MSOrder eMSOrder;
    enum MS_Dep eDependent;
    enum MS_Wideband eWideband;
    long bCustom;
    enum MS_SourceCID eSourceCID;
    enum MS_ScanType eScanType;
    enum MS_TurboScan eTurboScan;
    enum MS_IonizationMode eIonizationMode;
    enum MS_Corona eCorona;
    enum MS_Detector eDetector;
    double dDetectorValue;
    enum MS_SourceCIDType eSourceCIDType;
    long nlScanTypeIndex;
    long nNumMassRanges;
    struct MS_MassRange arrMassRanges[50];
    long nNumPrecursorMasses;
    double arrPrecursorMasses[10];
    double arrPrecursorEnergies[10];
    long arrPrecursorEnergiesValid[10];
    long nNumSourceFragmentationEnergies;
    double arrSourceFragmentationEnergies[50];
    long arrSourceFragmentationEnergiesValid[50];
};

#pragma pack(pop)

#pragma pack(push, 8)

struct MS_ScanIndex
{
    unsigned long m_ulDataOffset;
    long m_nlTrailerOffset;
    long m_nlScanTypeIndex;
    int m_nScanNumber;
    int m_nPacketType;
    int m_nNumberPackets;
    double m_dStartTime;
    double m_dTIC;
    double m_dBasePeakIntensity;
    double m_dBasePeakMass;
    double m_dLowMass;
    double m_dHighMass;
};

#pragma pack(pop)

#pragma pack(push, 8)

struct MS_UVScanIndex
{
    int nPacketType;
    int nNumberOfChannels;
    double dStartTime;
    double dTIC;
};

#pragma pack(pop)

#pragma pack(push, 8)

struct MS_DataPeak
{
    double intensity;
    double position;
    float basepeak;
    long scan;
};

#pragma pack(pop)

#pragma pack(push, 8)

struct MS_PrecursorInfo
{
    double dMonoIsoMZ;
    double dIsolationMZ;
    long nChargeState;
    long nScanNumber;
};

#pragma pack(pop)

struct __declspec(uuid("5fe970b1-29c3-11d3-811d-00104b304896"))
IXRawfile : IDispatch
{
    //
    // Wrapper methods for error-handling
    //

    HRESULT Open (
        _bstr_t szFileName );
    HRESULT Close ( );
    HRESULT GetFileName (
        BSTR * pbstrFileName );
    HRESULT GetCreatorID (
        BSTR * pbstrCreatorID );
    HRESULT GetVersionNumber (
        long * pnVersion );
    HRESULT GetCreationDate (
        DATE * pCreationDate );
    HRESULT IsError (
        long * pbIsError );
    HRESULT IsNewFile (
        long * pbIsNewFile );
    HRESULT GetErrorCode (
        long * pnErrorCode );
    HRESULT GetErrorMessage (
        BSTR * pbstrErrorMessage );
    HRESULT GetWarningMessage (
        BSTR * pbstrWarningMessage );
    HRESULT GetSeqRowNumber (
        long * pnSeqRowNumber );
    HRESULT GetSeqRowSampleType (
        long * pnSampleType );
    HRESULT GetSeqRowDataPath (
        BSTR * pbstrDataPath );
    HRESULT GetSeqRowRawFileName (
        BSTR * pbstrRawFileName );
    HRESULT GetSeqRowSampleName (
        BSTR * pbstrSampleName );
    HRESULT GetSeqRowSampleID (
        BSTR * pbstrSampleID );
    HRESULT GetSeqRowComment (
        BSTR * pbstrComment );
    HRESULT GetSeqRowLevelName (
        BSTR * pbstrLevelName );
    HRESULT GetSeqRowUserText (
        long nIndex,
        BSTR * pbstrUserText );
    HRESULT GetSeqRowInstrumentMethod (
        BSTR * pbstrInstrumentMethod );
    HRESULT GetSeqRowProcessingMethod (
        BSTR * pbstrProcessingMethod );
    HRESULT GetSeqRowCalibrationFile (
        BSTR * pbstrCalibrationFile );
    HRESULT GetSeqRowVial (
        BSTR * pbstrVial );
    HRESULT GetSeqRowInjectionVolume (
        double * pdInjVol );
    HRESULT GetSeqRowSampleWeight (
        double * pdSampleWt );
    HRESULT GetSeqRowSampleVolume (
        double * pdSampleVolume );
    HRESULT GetSeqRowISTDAmount (
        double * pdISTDAmount );
    HRESULT GetSeqRowDilutionFactor (
        double * pdDilutionFactor );
    HRESULT GetSeqRowUserLabel (
        long nIndex,
        BSTR * pbstrUserLabel );
    HRESULT InAcquisition (
        long * pbInAcquisition );
    HRESULT GetNumberOfControllers (
        long * pnNumControllers );
    HRESULT GetControllerType (
        long nIndex,
        long * pnControllerType );
    HRESULT SetCurrentController (
        long nControllerType,
        long nControllerNumber );
    HRESULT GetCurrentController (
        long * pnControllerType,
        long * pnControllerNumber );
    HRESULT GetNumSpectra (
        long * pnNumberOfSpectra );
    HRESULT GetNumStatusLog (
        long * pnNumberOfStatusLogEntries );
    HRESULT GetNumErrorLog (
        long * pnNumberOfErrorLogEntries );
    HRESULT GetNumTuneData (
        long * pnNumTuneData );
    HRESULT GetMassResolution (
        double * pdMassResolution );
    HRESULT GetExpectedRunTime (
        double * pdExpectedRunTime );
    HRESULT GetNumTrailerExtra (
        long * pnNumberOfTrailerExtraEntries );
    HRESULT GetLowMass (
        double * pdLowMass );
    HRESULT GetHighMass (
        double * pdHighMass );
    HRESULT GetStartTime (
        double * pdStartTime );
    HRESULT GetEndTime (
        double * pdEndTime );
    HRESULT GetMaxIntegratedIntensity (
        double * pdMaxIntegIntensity );
    HRESULT GetMaxIntensity (
        long * pnMaxIntensity );
    HRESULT GetFirstSpectrumNumber (
        long * pnFirstSpectrum );
    HRESULT GetLastSpectrumNumber (
        long * pnLastSpectrum );
    HRESULT GetInstrumentID (
        long * pnInstrumentID );
    HRESULT GetInletID (
        long * pnInletID );
    HRESULT GetErrorFlag (
        long * pnErrorFlag );
    HRESULT GetSampleVolume (
        double * pdSampleVolume );
    HRESULT GetSampleWeight (
        double * pdSampleWeight );
    HRESULT GetVialNumber (
        long * pnVialNumber );
    HRESULT GetInjectionVolume (
        double * pdInjectionVolume );
    HRESULT GetFlags (
        BSTR * pbstrFlags );
    HRESULT GetAcquisitionFileName (
        BSTR * pbstrFileName );
    HRESULT GetInstrumentDescription (
        BSTR * pbstrInstrumentDescription );
    HRESULT GetAcquisitionDate (
        BSTR * pbstrAcquisitionDate );
    HRESULT GetOperator (
        BSTR * pbstrOperator );
    HRESULT GetComment1 (
        BSTR * pbstrComment1 );
    HRESULT GetComment2 (
        BSTR * pbstrComment2 );
    HRESULT GetSampleAmountUnits (
        BSTR * pbstrSampleAmountUnits );
    HRESULT GetInjectionAmountUnits (
        BSTR * pbstrInjectionAmountUnits );
    HRESULT GetSampleVolumeUnits (
        BSTR * pbstrSampleVolumeUnits );
    HRESULT GetFilters (
        VARIANT * pvarFilterArray,
        long * pnArraySize );
    HRESULT ScanNumFromRT (
        double dRT,
        long * pnScanNumber );
    HRESULT RTFromScanNum (
        long nScanNumber,
        double * pdRT );
    HRESULT GetFilterForScanNum (
        long nScanNumber,
        BSTR * pbstrFilter );
    HRESULT GetFilterForScanRT (
        double dRT,
        BSTR * pbstrFilter );
    HRESULT GetMassListFromScanNum (
        long * pnScanNumber,
        _bstr_t bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        long * pnArraySize );
    HRESULT GetMassListFromRT (
        double * pdRT,
        _bstr_t bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        long * pnArraySize );
    HRESULT GetNextMassListFromScanNum (
        long * pnScanNumber,
        _bstr_t bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        long * pnArraySize );
    HRESULT GetPrevMassListFromScanNum (
        long * pnScanNumber,
        _bstr_t bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        long * pnArraySize );
    HRESULT IsProfileScanForScanNum (
        long nScanNumber,
        long * pbIsProfileScan );
    HRESULT IsCentroidScanForScanNum (
        long nScanNumber,
        long * pbIsCentroidScan );
    HRESULT GetScanHeaderInfoForScanNum (
        long nScanNumber,
        long * pnNumPackets,
        double * pdStartTime,
        double * pdLowMass,
        double * pdHighMass,
        double * pdTIC,
        double * pdBasePeakMass,
        double * pdBasePeakIntensity,
        long * pnNumChannels,
        long * pbUniformTime,
        double * pdFrequency );
    HRESULT GetStatusLogForScanNum (
        long nScanNumber,
        double * pdStatusLogRT,
        VARIANT * pvarLabels,
        VARIANT * pvarValues,
        long * pnArraySize );
    HRESULT GetStatusLogForRT (
        double * pdRT,
        VARIANT * pvarLabels,
        VARIANT * pvarValues,
        long * pnArraySize );
    HRESULT GetStatusLogLabelsForScanNum (
        long nScanNumber,
        double * pdStatusLogRT,
        VARIANT * pvarLabels,
        long * pnArraySize );
    HRESULT GetStatusLogLabelsForRT (
        double * pdRT,
        VARIANT * pvarLabels,
        long * pnArraySize );
    HRESULT GetStatusLogValueForScanNum (
        long nScanNumber,
        _bstr_t bstrLabel,
        double * pdStatusLogRT,
        VARIANT * pvarValue );
    HRESULT GetStatusLogValueForRT (
        double * pdRT,
        _bstr_t bstrLabel,
        VARIANT * pvarValue );
    HRESULT GetTrailerExtraForScanNum (
        long nScanNumber,
        VARIANT * pvarLabels,
        VARIANT * pvarValues,
        long * pnArraySize );
    HRESULT GetTrailerExtraForRT (
        double * pdRT,
        VARIANT * pvarLabels,
        VARIANT * pvarValues,
        long * pnArraySize );
    HRESULT GetTrailerExtraLabelsForScanNum (
        long nScanNumber,
        VARIANT * pvarLabels,
        long * pnArraySize );
    HRESULT GetTrailerExtraLabelsForRT (
        double * pdRT,
        VARIANT * pvarLabels,
        long * pnArraySize );
    HRESULT GetTrailerExtraValueForScanNum (
        long nScanNumber,
        _bstr_t bstrLabel,
        VARIANT * pvarValue );
    HRESULT GetTrailerExtraValueForRT (
        double * pdRT,
        _bstr_t bstrLabel,
        VARIANT * pvarValue );
    HRESULT GetErrorLogItem (
        long nItemNumber,
        double * pdRT,
        BSTR * pbstrErrorMessage );
    HRESULT GetTuneData (
        long nSegmentNumber,
        VARIANT * pvarLabels,
        VARIANT * pvarValues,
        long * pnArraySize );
    HRESULT GetNumInstMethods (
        long * pnNumInstMethods );
    HRESULT GetInstMethod (
        long nInstMethodItem,
        BSTR * pbstrInstMethod );
    HRESULT GetChroData (
        long nChroType1,
        long nChroOperator,
        long nChroType2,
        _bstr_t bstrFilter,
        _bstr_t bstrMassRanges1,
        _bstr_t bstrMassRanges2,
        double dDelay,
        double * pdStartTime,
        double * pdEndTime,
        long nSmoothingType,
        long nSmoothingValue,
        VARIANT * pvarChroData,
        VARIANT * pvarPeakFlags,
        long * pnArraySize );
    HRESULT RefreshViewOfFile ( );
    HRESULT GetTuneDataValue (
        long nSegmentNumber,
        _bstr_t bstrLabel,
        VARIANT * pvarValue );
    HRESULT GetTuneDataLabels (
        long nSegmentNumber,
        VARIANT * pvarLabels,
        long * pnArraySize );
    HRESULT GetInstName (
        BSTR * pbstrInstName );
    HRESULT GetInstModel (
        BSTR * pbstrInstModel );
    HRESULT GetInstSerialNumber (
        BSTR * pbstrInstSerialNumber );
    HRESULT GetInstSoftwareVersion (
        BSTR * pbstrInstSoftwareVersion );
    HRESULT GetInstHardwareVersion (
        BSTR * pbstrInstHardwareVersion );
    HRESULT GetInstFlags (
        BSTR * pbstrInstFlags );
    HRESULT GetInstNumChannelLabels (
        long * pnInstNumChannelLabels );
    HRESULT GetInstChannelLabel (
        long nChannelLabelNumber,
        BSTR * pbstrInstChannelLabel );
    HRESULT GetNumberOfControllersOfType (
        long nControllerType,
        long * pnNumControllersOfType );
    HRESULT GetAverageMassList (
        long * pnFirstAvgScanNumber,
        long * pnLastAvgScanNumber,
        long * pnFirstBkg1ScanNumber,
        long * pnLastBkg1ScanNumber,
        long * pnFirstBkg2ScanNumber,
        long * pnLastBkg2ScanNumber,
        _bstr_t bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        long * pnArraySize );
    HRESULT IsThereMSData (
        long * pbMSData );
    HRESULT HasExpMethod (
        long * pbMethod );
    HRESULT GetFilterMassPrecision (
        long * pnFilterMassPrecision );
    HRESULT GetStatusLogForPos (
        long nPos,
        VARIANT * pvarRT,
        VARIANT * pvarValue,
        long * pnArraySize );
    HRESULT GetStatusLogPlottableIndex (
        VARIANT * pvarIndex,
        VARIANT * pvarValues,
        long * pnArraySize );
    HRESULT GetInstMethodNames (
        long * pnNumInstMethods,
        VARIANT * pvarNames );
    HRESULT SetMassTolerance (
        long bUseUserDefined,
        double dMassTolerance,
        long nUnits );
    HRESULT GetChros (
        long nNumChros,
        double * pdStartTime,
        double * pdEndTime,
        VARIANT * pvarChroParamsArray,
        VARIANT * pvarChroDataSizeArray,
        VARIANT * pvarChroDataArray,
        VARIANT * pvarPeakFlagsArray );

    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall raw_Open (
        BSTR szFileName ) = 0;
      virtual HRESULT __stdcall raw_Close ( ) = 0;
      virtual HRESULT __stdcall raw_GetFileName (
        BSTR * pbstrFileName ) = 0;
      virtual HRESULT __stdcall raw_GetCreatorID (
        BSTR * pbstrCreatorID ) = 0;
      virtual HRESULT __stdcall raw_GetVersionNumber (
        long * pnVersion ) = 0;
      virtual HRESULT __stdcall raw_GetCreationDate (
        DATE * pCreationDate ) = 0;
      virtual HRESULT __stdcall raw_IsError (
        long * pbIsError ) = 0;
      virtual HRESULT __stdcall raw_IsNewFile (
        long * pbIsNewFile ) = 0;
      virtual HRESULT __stdcall raw_GetErrorCode (
        long * pnErrorCode ) = 0;
      virtual HRESULT __stdcall raw_GetErrorMessage (
        BSTR * pbstrErrorMessage ) = 0;
      virtual HRESULT __stdcall raw_GetWarningMessage (
        BSTR * pbstrWarningMessage ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowNumber (
        long * pnSeqRowNumber ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowSampleType (
        long * pnSampleType ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowDataPath (
        BSTR * pbstrDataPath ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowRawFileName (
        BSTR * pbstrRawFileName ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowSampleName (
        BSTR * pbstrSampleName ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowSampleID (
        BSTR * pbstrSampleID ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowComment (
        BSTR * pbstrComment ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowLevelName (
        BSTR * pbstrLevelName ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowUserText (
        long nIndex,
        BSTR * pbstrUserText ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowInstrumentMethod (
        BSTR * pbstrInstrumentMethod ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowProcessingMethod (
        BSTR * pbstrProcessingMethod ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowCalibrationFile (
        BSTR * pbstrCalibrationFile ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowVial (
        BSTR * pbstrVial ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowInjectionVolume (
        double * pdInjVol ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowSampleWeight (
        double * pdSampleWt ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowSampleVolume (
        double * pdSampleVolume ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowISTDAmount (
        double * pdISTDAmount ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowDilutionFactor (
        double * pdDilutionFactor ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowUserLabel (
        long nIndex,
        BSTR * pbstrUserLabel ) = 0;
      virtual HRESULT __stdcall raw_InAcquisition (
        long * pbInAcquisition ) = 0;
      virtual HRESULT __stdcall raw_GetNumberOfControllers (
        long * pnNumControllers ) = 0;
      virtual HRESULT __stdcall raw_GetControllerType (
        long nIndex,
        long * pnControllerType ) = 0;
      virtual HRESULT __stdcall raw_SetCurrentController (
        long nControllerType,
        long nControllerNumber ) = 0;
      virtual HRESULT __stdcall raw_GetCurrentController (
        long * pnControllerType,
        long * pnControllerNumber ) = 0;
      virtual HRESULT __stdcall raw_GetNumSpectra (
        long * pnNumberOfSpectra ) = 0;
      virtual HRESULT __stdcall raw_GetNumStatusLog (
        long * pnNumberOfStatusLogEntries ) = 0;
      virtual HRESULT __stdcall raw_GetNumErrorLog (
        long * pnNumberOfErrorLogEntries ) = 0;
      virtual HRESULT __stdcall raw_GetNumTuneData (
        long * pnNumTuneData ) = 0;
      virtual HRESULT __stdcall raw_GetMassResolution (
        double * pdMassResolution ) = 0;
      virtual HRESULT __stdcall raw_GetExpectedRunTime (
        double * pdExpectedRunTime ) = 0;
      virtual HRESULT __stdcall raw_GetNumTrailerExtra (
        long * pnNumberOfTrailerExtraEntries ) = 0;
      virtual HRESULT __stdcall raw_GetLowMass (
        double * pdLowMass ) = 0;
      virtual HRESULT __stdcall raw_GetHighMass (
        double * pdHighMass ) = 0;
      virtual HRESULT __stdcall raw_GetStartTime (
        double * pdStartTime ) = 0;
      virtual HRESULT __stdcall raw_GetEndTime (
        double * pdEndTime ) = 0;
      virtual HRESULT __stdcall raw_GetMaxIntegratedIntensity (
        double * pdMaxIntegIntensity ) = 0;
      virtual HRESULT __stdcall raw_GetMaxIntensity (
        long * pnMaxIntensity ) = 0;
      virtual HRESULT __stdcall raw_GetFirstSpectrumNumber (
        long * pnFirstSpectrum ) = 0;
      virtual HRESULT __stdcall raw_GetLastSpectrumNumber (
        long * pnLastSpectrum ) = 0;
      virtual HRESULT __stdcall raw_GetInstrumentID (
        long * pnInstrumentID ) = 0;
      virtual HRESULT __stdcall raw_GetInletID (
        long * pnInletID ) = 0;
      virtual HRESULT __stdcall raw_GetErrorFlag (
        long * pnErrorFlag ) = 0;
      virtual HRESULT __stdcall raw_GetSampleVolume (
        double * pdSampleVolume ) = 0;
      virtual HRESULT __stdcall raw_GetSampleWeight (
        double * pdSampleWeight ) = 0;
      virtual HRESULT __stdcall raw_GetVialNumber (
        long * pnVialNumber ) = 0;
      virtual HRESULT __stdcall raw_GetInjectionVolume (
        double * pdInjectionVolume ) = 0;
      virtual HRESULT __stdcall raw_GetFlags (
        BSTR * pbstrFlags ) = 0;
      virtual HRESULT __stdcall raw_GetAcquisitionFileName (
        BSTR * pbstrFileName ) = 0;
      virtual HRESULT __stdcall raw_GetInstrumentDescription (
        BSTR * pbstrInstrumentDescription ) = 0;
      virtual HRESULT __stdcall raw_GetAcquisitionDate (
        BSTR * pbstrAcquisitionDate ) = 0;
      virtual HRESULT __stdcall raw_GetOperator (
        BSTR * pbstrOperator ) = 0;
      virtual HRESULT __stdcall raw_GetComment1 (
        BSTR * pbstrComment1 ) = 0;
      virtual HRESULT __stdcall raw_GetComment2 (
        BSTR * pbstrComment2 ) = 0;
      virtual HRESULT __stdcall raw_GetSampleAmountUnits (
        BSTR * pbstrSampleAmountUnits ) = 0;
      virtual HRESULT __stdcall raw_GetInjectionAmountUnits (
        BSTR * pbstrInjectionAmountUnits ) = 0;
      virtual HRESULT __stdcall raw_GetSampleVolumeUnits (
        BSTR * pbstrSampleVolumeUnits ) = 0;
      virtual HRESULT __stdcall raw_GetFilters (
        VARIANT * pvarFilterArray,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_ScanNumFromRT (
        double dRT,
        long * pnScanNumber ) = 0;
      virtual HRESULT __stdcall raw_RTFromScanNum (
        long nScanNumber,
        double * pdRT ) = 0;
      virtual HRESULT __stdcall raw_GetFilterForScanNum (
        long nScanNumber,
        BSTR * pbstrFilter ) = 0;
      virtual HRESULT __stdcall raw_GetFilterForScanRT (
        double dRT,
        BSTR * pbstrFilter ) = 0;
      virtual HRESULT __stdcall raw_GetMassListFromScanNum (
        long * pnScanNumber,
        BSTR bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_GetMassListFromRT (
        double * pdRT,
        BSTR bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_GetNextMassListFromScanNum (
        long * pnScanNumber,
        BSTR bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_GetPrevMassListFromScanNum (
        long * pnScanNumber,
        BSTR bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_IsProfileScanForScanNum (
        long nScanNumber,
        long * pbIsProfileScan ) = 0;
      virtual HRESULT __stdcall raw_IsCentroidScanForScanNum (
        long nScanNumber,
        long * pbIsCentroidScan ) = 0;
      virtual HRESULT __stdcall raw_GetScanHeaderInfoForScanNum (
        long nScanNumber,
        long * pnNumPackets,
        double * pdStartTime,
        double * pdLowMass,
        double * pdHighMass,
        double * pdTIC,
        double * pdBasePeakMass,
        double * pdBasePeakIntensity,
        long * pnNumChannels,
        long * pbUniformTime,
        double * pdFrequency ) = 0;
      virtual HRESULT __stdcall raw_GetStatusLogForScanNum (
        long nScanNumber,
        double * pdStatusLogRT,
        VARIANT * pvarLabels,
        VARIANT * pvarValues,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_GetStatusLogForRT (
        double * pdRT,
        VARIANT * pvarLabels,
        VARIANT * pvarValues,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_GetStatusLogLabelsForScanNum (
        long nScanNumber,
        double * pdStatusLogRT,
        VARIANT * pvarLabels,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_GetStatusLogLabelsForRT (
        double * pdRT,
        VARIANT * pvarLabels,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_GetStatusLogValueForScanNum (
        long nScanNumber,
        BSTR bstrLabel,
        double * pdStatusLogRT,
        VARIANT * pvarValue ) = 0;
      virtual HRESULT __stdcall raw_GetStatusLogValueForRT (
        double * pdRT,
        BSTR bstrLabel,
        VARIANT * pvarValue ) = 0;
      virtual HRESULT __stdcall raw_GetTrailerExtraForScanNum (
        long nScanNumber,
        VARIANT * pvarLabels,
        VARIANT * pvarValues,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_GetTrailerExtraForRT (
        double * pdRT,
        VARIANT * pvarLabels,
        VARIANT * pvarValues,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_GetTrailerExtraLabelsForScanNum (
        long nScanNumber,
        VARIANT * pvarLabels,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_GetTrailerExtraLabelsForRT (
        double * pdRT,
        VARIANT * pvarLabels,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_GetTrailerExtraValueForScanNum (
        long nScanNumber,
        BSTR bstrLabel,
        VARIANT * pvarValue ) = 0;
      virtual HRESULT __stdcall raw_GetTrailerExtraValueForRT (
        double * pdRT,
        BSTR bstrLabel,
        VARIANT * pvarValue ) = 0;
      virtual HRESULT __stdcall raw_GetErrorLogItem (
        long nItemNumber,
        double * pdRT,
        BSTR * pbstrErrorMessage ) = 0;
      virtual HRESULT __stdcall raw_GetTuneData (
        long nSegmentNumber,
        VARIANT * pvarLabels,
        VARIANT * pvarValues,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_GetNumInstMethods (
        long * pnNumInstMethods ) = 0;
      virtual HRESULT __stdcall raw_GetInstMethod (
        long nInstMethodItem,
        BSTR * pbstrInstMethod ) = 0;
      virtual HRESULT __stdcall raw_GetChroData (
        long nChroType1,
        long nChroOperator,
        long nChroType2,
        BSTR bstrFilter,
        BSTR bstrMassRanges1,
        BSTR bstrMassRanges2,
        double dDelay,
        double * pdStartTime,
        double * pdEndTime,
        long nSmoothingType,
        long nSmoothingValue,
        VARIANT * pvarChroData,
        VARIANT * pvarPeakFlags,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_RefreshViewOfFile ( ) = 0;
      virtual HRESULT __stdcall raw_GetTuneDataValue (
        long nSegmentNumber,
        BSTR bstrLabel,
        VARIANT * pvarValue ) = 0;
      virtual HRESULT __stdcall raw_GetTuneDataLabels (
        long nSegmentNumber,
        VARIANT * pvarLabels,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_GetInstName (
        BSTR * pbstrInstName ) = 0;
      virtual HRESULT __stdcall raw_GetInstModel (
        BSTR * pbstrInstModel ) = 0;
      virtual HRESULT __stdcall raw_GetInstSerialNumber (
        BSTR * pbstrInstSerialNumber ) = 0;
      virtual HRESULT __stdcall raw_GetInstSoftwareVersion (
        BSTR * pbstrInstSoftwareVersion ) = 0;
      virtual HRESULT __stdcall raw_GetInstHardwareVersion (
        BSTR * pbstrInstHardwareVersion ) = 0;
      virtual HRESULT __stdcall raw_GetInstFlags (
        BSTR * pbstrInstFlags ) = 0;
      virtual HRESULT __stdcall raw_GetInstNumChannelLabels (
        long * pnInstNumChannelLabels ) = 0;
      virtual HRESULT __stdcall raw_GetInstChannelLabel (
        long nChannelLabelNumber,
        BSTR * pbstrInstChannelLabel ) = 0;
      virtual HRESULT __stdcall raw_GetNumberOfControllersOfType (
        long nControllerType,
        long * pnNumControllersOfType ) = 0;
      virtual HRESULT __stdcall raw_GetAverageMassList (
        long * pnFirstAvgScanNumber,
        long * pnLastAvgScanNumber,
        long * pnFirstBkg1ScanNumber,
        long * pnLastBkg1ScanNumber,
        long * pnFirstBkg2ScanNumber,
        long * pnLastBkg2ScanNumber,
        BSTR bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_IsThereMSData (
        long * pbMSData ) = 0;
      virtual HRESULT __stdcall raw_HasExpMethod (
        long * pbMethod ) = 0;
      virtual HRESULT __stdcall raw_GetFilterMassPrecision (
        long * pnFilterMassPrecision ) = 0;
      virtual HRESULT __stdcall raw_GetStatusLogForPos (
        long nPos,
        VARIANT * pvarRT,
        VARIANT * pvarValue,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_GetStatusLogPlottableIndex (
        VARIANT * pvarIndex,
        VARIANT * pvarValues,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_GetInstMethodNames (
        long * pnNumInstMethods,
        VARIANT * pvarNames ) = 0;
      virtual HRESULT __stdcall raw_SetMassTolerance (
        long bUseUserDefined,
        double dMassTolerance,
        long nUnits ) = 0;
      virtual HRESULT __stdcall raw_GetChros (
        long nNumChros,
        double * pdStartTime,
        double * pdEndTime,
        VARIANT * pvarChroParamsArray,
        VARIANT * pvarChroDataSizeArray,
        VARIANT * pvarChroDataArray,
        VARIANT * pvarPeakFlagsArray ) = 0;
};

struct __declspec(uuid("5e256644-7300-481f-9d43-33d892bfd912"))
IXRawfile2 : IXRawfile
{
    //
    // Wrapper methods for error-handling
    //

    HRESULT GetLabelData (
        VARIANT * pvarLabels,
        VARIANT * pvarFlags,
        long * pnScanNumber );
    HRESULT GetNoiseData (
        VARIANT * pvarNoisePacket,
        long * pnScanNumber );
    HRESULT GetSegmentedMassListFromRT (
        double * pdRT,
        _bstr_t bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        long * pnArraySize,
        VARIANT * pvarSegments,
        long * pnNumSegments,
        VARIANT * pvarLowHighMassRange );
    HRESULT GetSegmentedMassListFromScanNum (
        long * pnScanNumber,
        _bstr_t bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        long * pnArraySize,
        VARIANT * pvarSegments,
        long * pnNumSegments,
        VARIANT * pvarLowHighMassRange );
    HRESULT GetScanEventForScanNum (
        long nScanNumber,
        BSTR * pbstrScanEvent );
    HRESULT GetSeqRowUserTextEx (
        long nIndex,
        BSTR * pbstrUserText );
    HRESULT GetSeqRowBarcode (
        BSTR * pbstrBarcode );
    HRESULT GetSeqRowBarcodeStatus (
        long * pnBarcodeStatus );

    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall raw_GetLabelData (
        VARIANT * pvarLabels,
        VARIANT * pvarFlags,
        long * pnScanNumber ) = 0;
      virtual HRESULT __stdcall raw_GetNoiseData (
        VARIANT * pvarNoisePacket,
        long * pnScanNumber ) = 0;
      virtual HRESULT __stdcall raw_GetSegmentedMassListFromRT (
        double * pdRT,
        BSTR bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        long * pnArraySize,
        VARIANT * pvarSegments,
        long * pnNumSegments,
        VARIANT * pvarLowHighMassRange ) = 0;
      virtual HRESULT __stdcall raw_GetSegmentedMassListFromScanNum (
        long * pnScanNumber,
        BSTR bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        long * pnArraySize,
        VARIANT * pvarSegments,
        long * pnNumSegments,
        VARIANT * pvarLowHighMassRange ) = 0;
      virtual HRESULT __stdcall raw_GetScanEventForScanNum (
        long nScanNumber,
        BSTR * pbstrScanEvent ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowUserTextEx (
        long nIndex,
        BSTR * pbstrUserText ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowBarcode (
        BSTR * pbstrBarcode ) = 0;
      virtual HRESULT __stdcall raw_GetSeqRowBarcodeStatus (
        long * pnBarcodeStatus ) = 0;
};

struct __declspec(uuid("5e256644-7301-481f-9d43-33d892bfd912"))
IXRawfile3 : IXRawfile2
{
    //
    // Wrapper methods for error-handling
    //

    HRESULT GetMassListRangeFromScanNum (
        long * pnScanNumber,
        _bstr_t bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        LPWSTR szMassRange1,
        long * pnArraySize );
    HRESULT GetMassListRangeFromRT (
        double * pdRT,
        _bstr_t bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        LPWSTR szMassRange1,
        long * pnArraySize );
    HRESULT GetNextMassListRangeFromScanNum (
        long * pnScanNumber,
        _bstr_t bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        LPWSTR szMassRange1,
        long * pnArraySize );
    HRESULT GetPrevMassListRangeFromScanNum (
        long * pnScanNumber,
        _bstr_t bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        LPWSTR szMassRange1,
        long * pnArraySize );
    HRESULT GetPrecursorInfoFromScanNum (
        long nScanNumber,
        VARIANT * pvarPrecursorInfos,
        long * pnArraySize );

    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall raw_GetMassListRangeFromScanNum (
        long * pnScanNumber,
        BSTR bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        LPWSTR szMassRange1,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_GetMassListRangeFromRT (
        double * pdRT,
        BSTR bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        LPWSTR szMassRange1,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_GetNextMassListRangeFromScanNum (
        long * pnScanNumber,
        BSTR bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        LPWSTR szMassRange1,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_GetPrevMassListRangeFromScanNum (
        long * pnScanNumber,
        BSTR bstrFilter,
        long nIntensityCutoffType,
        long nIntensityCutoffValue,
        long nMaxNumberOfPeaks,
        long bCentroidResult,
        double * pdCentroidPeakWidth,
        VARIANT * pvarMassList,
        VARIANT * pvarPeakFlags,
        LPWSTR szMassRange1,
        long * pnArraySize ) = 0;
      virtual HRESULT __stdcall raw_GetPrecursorInfoFromScanNum (
        long nScanNumber,
        VARIANT * pvarPrecursorInfos,
        long * pnArraySize ) = 0;
};

enum MS_DataTypes
{
    MS_DataTypes_NULL = 0,
    MS_DataTypes_CHAR = 1,
    MS_DataTypes_TRUEFALSE = 2,
    MS_DataTypes_YESNO = 3,
    MS_DataTypes_ONOFF = 4,
    MS_DataTypes_UCHAR = 5,
    MS_DataTypes_SHORT = 6,
    MS_DataTypes_USHORT = 7,
    MS_DataTypes_LONG = 8,
    MS_DataTypes_ULONG = 9,
    MS_DataTypes_FLOAT = 10,
    MS_DataTypes_DOUBLE = 11,
    MS_DataTypes_CHAR_STRING = 12,
    MS_DataTypes_WCHAR_STRING = 13
};

struct __declspec(uuid("55ea38b7-5419-4be4-9198-3e4d78e6a532"))
IXVirMS : IDispatch
{
    //
    // Property data
    //

    __declspec(property(get=GetIsError))
    long IsError;
    __declspec(property(get=GetErrorCode))
    long ErrorCode;
    __declspec(property(get=GetIsValid))
    long IsValid;

    //
    // Wrapper methods for error-handling
    //

    HRESULT Create (
        LPWSTR szFileName );
    HRESULT Close ( );
    HRESULT GetFileName (
        BSTR * pbstrFileName );
    long GetIsError ( );
    long GetErrorCode ( );
    long GetIsValid ( );
    HRESULT WriteInstID (
        LPWSTR szName,
        LPWSTR szModel,
        LPWSTR szSerialNumber,
        LPWSTR szSoftwareRev,
        LPWSTR ExpType );
    HRESULT WriteRunHeaderInfo (
        double dExpectedRunTime,
        double dMassResolution,
        LPWSTR szComment1,
        LPWSTR szComment2 );
    HRESULT WriteInstData (
        unsigned char * pcData,
        long nDataSize,
        enum MS_PacketTypes eType );
    HRESULT SetTrailerHeaderNumFields (
        long nFields );
    HRESULT SetTrailerHeaderField (
        long nIdx,
        LPWSTR szLabel,
        enum MS_DataTypes eDataType,
        long nPrecision );
    HRESULT WriteTrailerHeader ( );
    HRESULT SetStatusLogHeaderNumFields (
        long nFields );
    HRESULT SetStatusLogHeaderField (
        long nIdx,
        LPWSTR szLabel,
        enum MS_DataTypes eDataType,
        long nPrecision );
    HRESULT WriteStatusLogHeader ( );
    HRESULT SetTuneDataHeaderNumFields (
        long nFields );
    HRESULT SetTuneDataHeaderField (
        long nIdx,
        LPWSTR szLabel,
        enum MS_DataTypes eDataType,
        long nPrecision );
    HRESULT WriteTuneDataHeader ( );
    HRESULT WriteTuneData (
        unsigned char * pcData );
    HRESULT WriteStatusLog (
        float fTime,
        unsigned char * pcData );
    HRESULT WriteTrailer (
        unsigned char * pcData );
    HRESULT InitializeScanEvent (
        struct MS_ScanEvent * pScanEvent );
    HRESULT InitMethodScanEvents ( );
    HRESULT SetMethodScanEvent (
        long nSegment,
        long nScanEvent,
        struct MS_ScanEvent * pScanEvent );
    HRESULT WriteMethodScanEvents ( );
    HRESULT WriteScanIndex (
        struct MS_ScanIndex * pScanIndex,
        struct MS_ScanEvent * pScanEvent );
    HRESULT WriteInstData2 (
        long nNumPkts,
        struct MS_DataPeak * pPackets );
    HRESULT InitializeScanIndex (
        long nScanIndexPosition,
        enum MS_PacketTypes eType );
    HRESULT WriteScanIndex2 (
        struct MS_ScanIndex * pScanIndex );

    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall raw_Create (
        /*[in]*/ LPWSTR szFileName ) = 0;
      virtual HRESULT __stdcall raw_Close ( ) = 0;
      virtual HRESULT __stdcall raw_GetFileName (
        /*[in]*/ BSTR * pbstrFileName ) = 0;
      virtual HRESULT __stdcall get_IsError (
        /*[out,retval]*/ long * pVal ) = 0;
      virtual HRESULT __stdcall get_ErrorCode (
        /*[out,retval]*/ long * pVal ) = 0;
      virtual HRESULT __stdcall get_IsValid (
        /*[out,retval]*/ long * pVal ) = 0;
      virtual HRESULT __stdcall raw_WriteInstID (
        /*[in]*/ LPWSTR szName,
        /*[in]*/ LPWSTR szModel,
        /*[in]*/ LPWSTR szSerialNumber,
        /*[in]*/ LPWSTR szSoftwareRev,
        /*[in]*/ LPWSTR ExpType ) = 0;
      virtual HRESULT __stdcall raw_WriteRunHeaderInfo (
        /*[in]*/ double dExpectedRunTime,
        /*[in]*/ double dMassResolution,
        /*[in]*/ LPWSTR szComment1,
        /*[in]*/ LPWSTR szComment2 ) = 0;
      virtual HRESULT __stdcall raw_WriteInstData (
        /*[in]*/ unsigned char * pcData,
        /*[in]*/ long nDataSize,
        /*[in]*/ enum MS_PacketTypes eType ) = 0;
      virtual HRESULT __stdcall raw_SetTrailerHeaderNumFields (
        /*[in]*/ long nFields ) = 0;
      virtual HRESULT __stdcall raw_SetTrailerHeaderField (
        /*[in]*/ long nIdx,
        /*[in]*/ LPWSTR szLabel,
        /*[in]*/ enum MS_DataTypes eDataType,
        /*[in]*/ long nPrecision ) = 0;
      virtual HRESULT __stdcall raw_WriteTrailerHeader ( ) = 0;
      virtual HRESULT __stdcall raw_SetStatusLogHeaderNumFields (
        long nFields ) = 0;
      virtual HRESULT __stdcall raw_SetStatusLogHeaderField (
        /*[in]*/ long nIdx,
        /*[in]*/ LPWSTR szLabel,
        /*[in]*/ enum MS_DataTypes eDataType,
        /*[in]*/ long nPrecision ) = 0;
      virtual HRESULT __stdcall raw_WriteStatusLogHeader ( ) = 0;
      virtual HRESULT __stdcall raw_SetTuneDataHeaderNumFields (
        long nFields ) = 0;
      virtual HRESULT __stdcall raw_SetTuneDataHeaderField (
        /*[in]*/ long nIdx,
        /*[in]*/ LPWSTR szLabel,
        /*[in]*/ enum MS_DataTypes eDataType,
        /*[in]*/ long nPrecision ) = 0;
      virtual HRESULT __stdcall raw_WriteTuneDataHeader ( ) = 0;
      virtual HRESULT __stdcall raw_WriteTuneData (
        /*[in]*/ unsigned char * pcData ) = 0;
      virtual HRESULT __stdcall raw_WriteStatusLog (
        /*[in]*/ float fTime,
        /*[in]*/ unsigned char * pcData ) = 0;
      virtual HRESULT __stdcall raw_WriteTrailer (
        /*[in]*/ unsigned char * pcData ) = 0;
      virtual HRESULT __stdcall raw_InitializeScanEvent (
        /*[in]*/ struct MS_ScanEvent * pScanEvent ) = 0;
      virtual HRESULT __stdcall raw_InitMethodScanEvents ( ) = 0;
      virtual HRESULT __stdcall raw_SetMethodScanEvent (
        /*[in]*/ long nSegment,
        /*[in]*/ long nScanEvent,
        /*[in]*/ struct MS_ScanEvent * pScanEvent ) = 0;
      virtual HRESULT __stdcall raw_WriteMethodScanEvents ( ) = 0;
      virtual HRESULT __stdcall raw_WriteScanIndex (
        /*[in]*/ struct MS_ScanIndex * pScanIndex,
        /*[in]*/ struct MS_ScanEvent * pScanEvent ) = 0;
      virtual HRESULT __stdcall raw_WriteInstData2 (
        /*[in]*/ long nNumPkts,
        /*[in]*/ struct MS_DataPeak * pPackets ) = 0;
      virtual HRESULT __stdcall raw_InitializeScanIndex (
        /*[in]*/ long nScanIndexPosition,
        /*[in]*/ enum MS_PacketTypes eType ) = 0;
      virtual HRESULT __stdcall raw_WriteScanIndex2 (
        /*[in]*/ struct MS_ScanIndex * pScanIndex ) = 0;
};

struct __declspec(uuid("1a2bf13f-4e2f-4e7d-9d67-435d5998312b"))
IXVirUV : IDispatch
{
    //
    // Property data
    //

    __declspec(property(get=GetIsError))
    long IsError;
    __declspec(property(get=GetErrorCode))
    long ErrorCode;
    __declspec(property(get=GetIsValid))
    long IsValid;

    //
    // Wrapper methods for error-handling
    //

    HRESULT Create (
        LPWSTR szFileName );
    HRESULT Close ( );
    HRESULT GetFileName (
        BSTR * pbstrFileName );
    long GetIsError ( );
    long GetErrorCode ( );
    long GetIsValid ( );
    HRESULT WriteErrorLog (
        float fTime,
        LPWSTR szError );
    HRESULT WriteInstID (
        LPWSTR szName,
        LPWSTR szModel,
        LPWSTR szSerialNumber,
        LPWSTR szSoftwareRev,
        LPWSTR szLabel1,
        LPWSTR szLabel2,
        LPWSTR szLabel3,
        LPWSTR szLabel4 );
    HRESULT WriteRunHeaderInfo (
        double dExpectedRunTime );
    HRESULT WriteInstData (
        unsigned char * pcData,
        long nDataSize,
        enum MS_PacketTypes eType,
        long nDataLen );
    HRESULT WriteScanIndex (
        struct MS_UVScanIndex * pScanIndex );

    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall raw_Create (
        /*[in]*/ LPWSTR szFileName ) = 0;
      virtual HRESULT __stdcall raw_Close ( ) = 0;
      virtual HRESULT __stdcall raw_GetFileName (
        BSTR * pbstrFileName ) = 0;
      virtual HRESULT __stdcall get_IsError (
        /*[out,retval]*/ long * pVal ) = 0;
      virtual HRESULT __stdcall get_ErrorCode (
        /*[out,retval]*/ long * pVal ) = 0;
      virtual HRESULT __stdcall get_IsValid (
        /*[out,retval]*/ long * pVal ) = 0;
      virtual HRESULT __stdcall raw_WriteErrorLog (
        /*[in]*/ float fTime,
        /*[in]*/ LPWSTR szError ) = 0;
      virtual HRESULT __stdcall raw_WriteInstID (
        /*[in]*/ LPWSTR szName,
        /*[in]*/ LPWSTR szModel,
        /*[in]*/ LPWSTR szSerialNumber,
        /*[in]*/ LPWSTR szSoftwareRev,
        /*[in]*/ LPWSTR szLabel1,
        /*[in]*/ LPWSTR szLabel2,
        /*[in]*/ LPWSTR szLabel3,
        /*[in]*/ LPWSTR szLabel4 ) = 0;
      virtual HRESULT __stdcall raw_WriteRunHeaderInfo (
        double dExpectedRunTime ) = 0;
      virtual HRESULT __stdcall raw_WriteInstData (
        /*[in]*/ unsigned char * pcData,
        /*[in]*/ long nDataSize,
        /*[in]*/ enum MS_PacketTypes eType,
        /*[in]*/ long nDataLen ) = 0;
      virtual HRESULT __stdcall raw_WriteScanIndex (
        /*[in]*/ struct MS_UVScanIndex * pScanIndex ) = 0;
};

struct __declspec(uuid("5fe970b2-29c3-11d3-811d-00104b304896"))
XRawfile;
    // [ default ] interface IXRawfile

struct __declspec(uuid("281748dc-d278-4303-9888-767bad540e80"))
XVirMS;
    // [ default ] interface IXVirMS

struct __declspec(uuid("20af91e1-fbcb-40f5-b4d4-98f1291edef0"))
XVirUV;
    // [ default ] interface IXVirUV

//
// Wrapper method implementations
//

// originally:
// #include "xrawfile2.tli"
#include "xrawfile2_2-0.cpp"

} // namespace XRAWFILE2Lib

#pragma pack(pop)
