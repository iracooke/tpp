// -*- mode: c++ -*-


/*
    File: AnalystImpl.cpp
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


#include "stdafx.h"
#include "AnalystImpl.h"

#include "Analyst14Interface.h"
#include "Analyst15Interface.h"
#include "AnalystQS11Interface.h"
#include "AnalystQS20Interface.h"

#include "mzXML/common/MSUtilities.h"

using namespace std;

#ifdef _MSC_VER
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif
#include <io.h>	//_access

bool checkPath(const std::string &pathname)
{
	int nPathNameResult=0;
	int nPathResult=0;

	if (-1 == _access(pathname.c_str(), 0)) {
		nPathNameResult = errno;
	}

	string pathOnly;
	{
		string::size_type nPos = pathname.rfind(PATH_SEPARATOR);
		if (string::npos == nPos) {
			pathOnly = pathname;
		} else {
			pathOnly = pathname.substr(0, nPos+1);
		}
		if (-1==_access(pathOnly.c_str(), 6)) {
			nPathResult = errno;
		}
	}

	if (0==nPathNameResult && 0==nPathResult)
		return true;

	if (0!=nPathNameResult)
		cerr << "ERROR: '" << pathname.c_str() << "' could not be access. (" 
			<< nPathNameResult << ")" << endl;

	if (0!=nPathResult)
		cerr << "ERROR: Path '" << pathOnly.c_str() << "' could not be access. (" 
		<< nPathResult << ")" << endl;

	return false;
} /*checkPath*/

void printCOMErrorString (HRESULT hr)
{
	LPVOID lpMsgBuf = NULL;
	::FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM 
			| FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	if ( lpMsgBuf ) {
		USES_CONVERSION;
		const char *szMsg = T2A((LPTSTR)lpMsgBuf);
		std::cerr << "INFO: error message: " << szMsg << std::endl;
		LocalFree(lpMsgBuf);
	}
}

int getLibraryVersion (enum LibraryTagType whichLibrary, std::string &strVersion) {
	strVersion.resize(0);

	std::string strKeyName("SOFTWARE\\PE Sciex\\Products\\");

	if (LIBRARY_Analyst==whichLibrary) {
		strKeyName.append ("Analyst3Q");
	} else if (LIBRARY_AnalystQS==whichLibrary) {
		strKeyName.append ("AnalystQS");
	} else {
		return -2;
	}

	USES_CONVERSION;

	HKEY hKeyProducts;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			A2W(strKeyName.c_str()),
			0, KEY_READ, &hKeyProducts) == ERROR_SUCCESS) {

		TCHAR szVersion[256] = {0};
		DWORD dwBufferSize = 256;
		if (RegQueryValueEx(hKeyProducts,TEXT("Version"), NULL, NULL,
			(LPBYTE)&szVersion, &dwBufferSize) == ERROR_SUCCESS) {
			USES_CONVERSION;
			strVersion = T2A(szVersion);
		}

		RegCloseKey(hKeyProducts);

		return 0;

	} else {
		return -1;
	}
}

const char *getLibraryName (enum LibraryTagType whichLibrary) {
	switch (whichLibrary) {
		case LIBRARY_Analyst:
			return "Analyst";
		case LIBRARY_AnalystQS:
			return "Analyst QS";
		default:
			return "Unknown";
	}
}

CompatibilityType isCompatible (enum LibraryTagType whichLibrary, std::string &strSignature) {
	//TODO: test version - COMPATIBILITY_Expect_Newer_Version
	switch (whichLibrary) {
		case LIBRARY_Analyst:
			if ((strSignature.find(getLibraryName(LIBRARY_Analyst)) != strSignature.npos)
				&& (strSignature.find(getLibraryName(LIBRARY_AnalystQS)) == strSignature.npos)) {
				return COMPATIBILITY_OK;
			} else {
				if (strSignature.find(getLibraryName(LIBRARY_AnalystQS)) != strSignature.npos) {
					return COMPATIBILITY_Expect_AnalystQS;
				} else {
					return COMPATIBILITY_Expect_Unknown;
				}
			}
		case LIBRARY_AnalystQS:
			if (strSignature.find(getLibraryName(LIBRARY_AnalystQS)) != strSignature.npos) {
				return COMPATIBILITY_OK;
			} else {
				if (strSignature.find(getLibraryName(LIBRARY_Analyst)) != strSignature.npos) {
					return COMPATIBILITY_Expect_Analyst;
				} else {
					return COMPATIBILITY_Expect_Unknown;
				}
			}
		default:
			return COMPATIBILITY_No_Library_Signature;
	}
}

void getSoftwareVersion (const std::string &strSignature, std::string &strSoftware, std::string &strVersion) {
	std::string::size_type nPos = strSignature.find_last_of(" ");
	if (nPos == std::string::npos) {
		// no space found, assume it is just software name, no version
		strSoftware = strSignature;
		strVersion.resize(0);
	} else {
		// the RHS of space might be the version
		// assume that if we have digits, period mostly, it is a version
		std::string::size_type lVersionChars = 0;
		std::string::size_type nLen = strSignature.length();
		for (std::string::size_type i=nPos+1; i<nLen; i++) {
			if (strSignature[i] == '.' || (strSignature[i]>= '0' && strSignature[i]<='9')) {
				lVersionChars++;
			}
		}
		nLen -= (nPos+1);
		nLen -= lVersionChars;

		if (lVersionChars > nLen) {
			strSoftware = strSignature.substr(0, nPos);
			strVersion = strSignature.substr(nPos+1);
		} else {
			strSoftware = strSignature;
			strVersion.resize(0);
		}
	}
}

MSAcquisitionSoftwareType softwareToMSAcquisitionSoftwareType(std::string &strSoftware) {
	if (strSoftware=="Analyst QS") {
		return ANALYSTQS;
	} else if (strSoftware=="Analyst") {
		return ANALYST;
	} else {
		return ACQUISITIONSOFTWARE_UNDEF;
	}
}

int getModelById (long lId, std::string &strModel) {
	strModel.resize(0);

	char    buffer[11+1]; //-2147483648 to 2147483647
	_ltoa(lId, buffer, 10);
	std::string strKeyName("SOFTWARE\\PE Sciex\\Analyst\\DAME\\DeviceModelMap\\0\\");
	strKeyName.append (buffer);

	USES_CONVERSION;

	HKEY hKeyAnalyst;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		A2W(strKeyName.c_str() ),
		0, KEY_READ, &hKeyAnalyst) == ERROR_SUCCESS) {

		int nRet = -1;
		TCHAR szVersion[1024] = {0};
		DWORD dwBufferSize = 1024;
		if (RegQueryValueEx(hKeyAnalyst,TEXT("UserFriendlyName"), NULL, NULL,
			(LPBYTE)&szVersion, &dwBufferSize) == ERROR_SUCCESS) {
			strModel = T2A(szVersion);

			// new system: "QStar XL"
			// old system: "MassSpecMethod QStar XL"
			// so, we remove it from older system for consistency
			const std::string prefix("MassSpecMethod ");
			if (0 == strModel.find(prefix)) {
				strModel.erase (0, prefix.length() );
			}

			nRet = 0;

		} else {

			strModel = std::string("Model Id: ").append(buffer);

			cerr << "WARNING: Device Model Map does not contain \"UserFriendlyName\", using model id rather than model description." << endl;
			cerr << "WARNING: Please check that you have installed your Analyst library correctly." << endl;

		}

		RegCloseKey(hKeyAnalyst);

		return nRet;

	} else {

			strModel = std::string("Model Id: ").append(buffer);

		cerr << "WARNING: Device Model Map not found, using model id rather than model description." << endl;
		cerr << "WARNING: Please check that you have installed your Analyst library correctly." << endl;

		return -1;
	}
}

MSInstrumentModelType modelIdToMSInstrumentModel(long lId) {
	switch (lId) {
		case 1:
			return API_100;
		case 2:
			return API_100_LC;
		case 3:
			return API_150_MCA;
		case 4:
			return API_150_EX;
		case 5:
			return API_165;
		case 6:
			return API_300;
		case 7:
			return API_350;
		case 8:
			return API_365;
		case 9:
			return API_2000;
		case 10:
			return API_3000;
		case 11:
			return API_4000;
		case 12:
			return GENERIC_SINGLE_QUAD;
		case 13:
			return QTRAP;
		case 14:
			return _4000_Q_TRAP;
		case 15:
			return API_3200;
		case 16:
			return _3200_Q_TRAP;
		case 17:
			return API_5000;
		case 1001:
			return ELAN_6000;
		case 2001:
			return QSTAR;
		case 2002:
			return API_QSTAR_PULSAR;
		case 2003:
			return API_QSTAR_PULSAR_I;
		case 2004:
			return QSTAR_XL_SYSTEM;
		case 10000:
			return AGILENT_TOF;
		default:
			return INSTRUMENTMODEL_UNDEF;
	}
}

void getSampleLogEntry(const std::string &strSampleLog, const std::string &strKey, std::string &strValue) {
	strValue.resize(0);

	if (strKey.empty () ) {
		return;
	}

	std::string::size_type nStartPos = strSampleLog.find(strKey);
	if (strSampleLog.npos == nStartPos) {
		return;
	}

	nStartPos += strKey.length() ;

	std::string::size_type nEndPos = strSampleLog.find(", ", nStartPos);
	if (strSampleLog.npos == nEndPos) {
		strValue = strSampleLog.substr (nStartPos);
	} else {
		nEndPos -= nStartPos;
		strValue = strSampleLog.substr (nStartPos, nEndPos);
	}
}

static const int scanToMSLevel[] = {
	1, 
	1, 2, 2, 2, 1, 
	2, 1, 1, 2, 1, 
	2, 1, 3, 1, 1, 
	1, 1};
static const int scanToMSLevelSize = sizeof(scanToMSLevel)/sizeof(int);
static const int scanToMSLevelLastIndex = scanToMSLevelSize-1;

static const char *scanToString[] = {
	"Q1 Scan",
	"Q1 MI", "Q3 Scan", "Q3 MI", "MRM", "Precursor Scan",
	"Product Ion Scan", "Neutral Loss Scan", "TOF MS1", "TOF MS2", "TOF Precursor Ion Scan",
	"EPI", "ER", "MS3", "TDF", "EMS",
	"EMC", "Unknown"};
static const int scanToStringSize = sizeof(scanToString)/sizeof(int);
static const int scanToStringLastIndex = scanToStringSize-1;

static const MSScanType scanToMSScanType[] = {
	Q1Scan,
	Q1MI, Q3Scan, Q3MI, MRM, PrecursorScan,
	ProductIonScan, NeutralLossScan, TOFMS1, TOFMS2, TOFPrecursorIonScan,
	EPI, ER, MS3, TDF, EMS,
	EMC, SCAN_UNDEF
};
static const int scanToMSScanTypeSize = sizeof(scanToMSScanType)/sizeof(int);
static const int scanToMSScanTypeLastIndex = scanToMSScanTypeSize-1;

int getMSLevel(long lScanType)
{
	return ((lScanType<scanToMSLevelSize) ? 
		scanToMSLevel[lScanType] : scanToMSLevel[scanToMSLevelLastIndex]);
}

const char *getPolarityString(long lPolarity)
{
	return (0==lPolarity?"+":"-");
}

const char *getPolarityString(MSPolarityType msPolarity)
{
	if (POSITIVE==msPolarity)
		return "+";
	else if (NEGATIVE==msPolarity)
		return "-";
	else if (POLARITY_UNDEF==msPolarity)
		return "?";
	else if (ANY==msPolarity)
		return "*";
	else
		return "?";
}

const char *getScanTypeString(long lScanType)
{
	return ((lScanType<scanToStringSize) ? 
	scanToString[lScanType] : scanToString[scanToStringLastIndex]);
}

MSScanType getMSScanType(long lScanType)
{
	return ((lScanType<scanToMSScanTypeSize) ? 
		scanToMSScanType[lScanType] : scanToMSScanType[scanToMSScanTypeLastIndex]);
}

unsigned long getLibraryVersion (const std::string &strLibraryVersion)
{
	std::string lowerVersion = toLower(strLibraryVersion);
	int x, y, z;
	x=y=z=0;
	sscanf (lowerVersion.c_str(), "%d.%d.%d", &x, &y, &z);

	unsigned long ulVersion=0;
	ulVersion = x;
	ulVersion = ulVersion << 8;
	ulVersion += y;
	ulVersion = ulVersion << 8;
	ulVersion += z;
	ulVersion = ulVersion << 8;

	if (lowerVersion.find("alpha") != lowerVersion.npos) {
		ulVersion += 1;
	} else if  (lowerVersion.find("beta") != lowerVersion.npos) {
		ulVersion += 2;
	} else {
		ulVersion += 4;
	}

	return ulVersion;
}

AnalystBaseInterface *getAnalystInterface(bool fVerbose)
{
	AnalystBaseInterface *pReturn = NULL;
	std::string msg;
	string strLibraryVersion;
	if (0==getLibraryVersion(LIBRARY_Analyst, strLibraryVersion)
		&& !strLibraryVersion.empty()) {

		unsigned long ulVersion=getLibraryVersion(strLibraryVersion);
		bool fToWarn=false;
		ostringstream msgStream;
		msgStream << "INFO: Analyst " << strLibraryVersion << " ("
			<< std::hex << ulVersion << std::dec 
			<< ") detected.";
		if (ulVersion < 0x01020000) {
			// v0.0.0 to v1.1.x
			pReturn = new Analyst14Interface();
		} else if (ulVersion < 0x01030000) {
			// v1.2.0 to v1.2.x
			pReturn = new Analyst14Interface();
		} else if (ulVersion < 0x01040000) {
			// v1.3.0 to v1.3.x
			pReturn = new Analyst14Interface();
		} else if (ulVersion < 0x01050000) {
			// v1.4.0 to v1.4.x
			pReturn = new Analyst14Interface();
		} else if (ulVersion < 0x01050100) {
			// v1.5.0
			pReturn = new Analyst15Interface();
		} else {
			// v1.5.1 onward
			fToWarn = true;
			pReturn = new Analyst15Interface();
		}
		msgStream << "  Using v" << pReturn->getInterfaceVersion().c_str()
			<< " interface.";
		if (fToWarn) {
			msgStream << std::endl << std::endl;
			msgStream << "IMPORTANT: You are using a newer Analyst version that mzWiff has not encountered before.";
			msgStream << "  There could be some incompatibility issues.";
			msgStream << "  You could contact the author(s) to check that your version of Analyst library is backward compatible.";
			msgStream << std::endl;
		}
		msg.append (msgStream.str());
	} else {
		// no installation detected
		msg.append("WARNING: No Analyst installation detected.  Using v1.5 interface.");
		pReturn = new Analyst15Interface();
	}

	if (fVerbose && !msg.empty()) {
		std::cout << msg.c_str() << std::endl;
	}
	return pReturn;
}

AnalystBaseInterface *getAnalystQSInterface(bool fVerbose)
{
	AnalystBaseInterface *pReturn = NULL;
	std::string msg;
	string strLibraryVersion;
	if (0==getLibraryVersion(LIBRARY_AnalystQS, strLibraryVersion) 
		&& !strLibraryVersion.empty()) {

		unsigned long ulVersion=getLibraryVersion(strLibraryVersion);
		bool fToWarn=false;
		ostringstream msgStream;
		msgStream << "INFO: AnalystQS " << strLibraryVersion << " ("
			<< std::hex << ulVersion << std::dec 
			<< ") detected.";
		if (ulVersion < 0x01010000) {
			// v0.0.0 to v1.0.x
			pReturn = new AnalystQS11Interface();
		} else if (ulVersion < 0x02000000) {
			// v1.1.0 to v1.x.y
			pReturn = new AnalystQS11Interface();
		} else if (ulVersion < 0x02010000) {
			// v2.0.0 to v2.0.x
			pReturn = new AnalystQS20Interface();
		} else {
			// v2.1.x onward
			fToWarn = true;
			pReturn = new AnalystQS20Interface();
		}
		msgStream << "  Using v" << pReturn->getInterfaceVersion().c_str()
			<< " interface.";
		if (fToWarn) {
			msgStream << std::endl << std::endl;
			msgStream << "IMPORTANT: You are using a newer AnalystQS version that mzWiff has not encountered before.";
			msgStream << "  There could be some incompatibility issues.";
			msgStream << "  You could contact the author(s) to check that your version of AnalystQS library is backward compatible.";
			msgStream << std::endl;
		}
		msg.append (msgStream.str());
	} else {
		// no installation detected
		msg.append("WARNING: No AnalystQS installation detected.  Using v2.0 interface.");
		pReturn = new AnalystQS20Interface();
	}

	if (fVerbose && !msg.empty()) {
		std::cout << msg.c_str() << std::endl;
	}
	return pReturn;
}

//======================================================================

InstrumentInfoParser::InstrumentInfoParser(InstrumentInfo &instrumentInfo, int iVerbose)
:	m_instrumentInfo(instrumentInfo),
	m_iVerbose(iVerbose)
{
}

InstrumentInfoParser::~InstrumentInfoParser(void)
{
}

bool InstrumentInfoParser::parse(const std::string &strLog, long lModelId)
{
	// TODO:
	// 1. actual implementation
	// 2. take value supply by user?

	std::string strManufacturer;
	getMSManufacturer(strLog, strManufacturer);
	if (m_iVerbose>=VERBOSE_INFO) {
		if (!strManufacturer.empty()) {
			std::cout << "INFO: Found Manufacturer tag \"" << strManufacturer.c_str() << "\" in file." << std::endl;
		}
	}

	m_instrumentInfo.instrumentModel_ = modelIdToMSInstrumentModel(lModelId);
	if (m_iVerbose>=VERBOSE_INFO) {
		std::string strModel;
		getModelById (lModelId, strModel);
		if (!strModel.empty()) {
			std::cout << "INFO: Found Model tag \"" << strModel.c_str() << "\" in file." << std::endl;
		}
	}

	getMSSerialNumber(strLog, m_instrumentInfo.instrumentSerialNumber_);
	if (m_iVerbose>=VERBOSE_INFO) {
		if (!m_instrumentInfo.instrumentSerialNumber_.empty()) {
			std::cout << "INFO: Found Serial Number tag \"" << m_instrumentInfo.instrumentSerialNumber_.c_str() << "\" in file." << std::endl;
		}
	}

	getMSHardwareVersion(strLog, m_instrumentInfo.instrumentHardwareVersion_);
	if (m_iVerbose>=VERBOSE_INFO) {
		if (!m_instrumentInfo.instrumentHardwareVersion_.empty()) {
			std::cout << "INFO: Found Hardware Version tag \"" << m_instrumentInfo.instrumentHardwareVersion_.c_str() << "\" in file." << std::endl;
		}
	}

	if (IONIZATION_UNDEF == m_instrumentInfo.ionSource_) {
		std::string strIonSource;
		getMSDetector(strLog, strIonSource);
		if (strIonSource.empty()) {
			strIonSource = "ESI";
		} else {
			// TODO: translate internal text to MSIonizationType
			if (m_iVerbose>=VERBOSE_INFO) {
				std::cout << "INFO: Found Ionization tag \"" << strIonSource.c_str() << "\" in file." << std::endl;
			}
		}
		m_instrumentInfo.ionSource_ = MSIonizationTypeFromString(strIonSource);
	}

	if (m_instrumentInfo.analyzerList_.size()<1 
		|| ANALYZER_UNDEF == m_instrumentInfo.analyzerList_[0]) {
		std::string strAnalyzer;
		getMSMassAnalyzer(strLog, strAnalyzer);
		if (strAnalyzer.empty()) {
			strAnalyzer = "TOFMS";
		} else {
			// TODO: translate internal text to MSIonizationType
			if (m_iVerbose>=VERBOSE_INFO) {
				std::cout << "INFO: Found Mass Analyzer tag \"" << strAnalyzer.c_str() << "\" in file." << std::endl;
			}
		}
		if (m_instrumentInfo.analyzerList_.size()<1) {
			m_instrumentInfo.analyzerList_.push_back(MSAnalyzerTypeFromString(strAnalyzer));
		} else {
			m_instrumentInfo.analyzerList_[0] = MSAnalyzerTypeFromString(strAnalyzer);
		}
	}

	/*
	std::string strDetector;
	getMSDetector(strDetector);
	*/
	m_instrumentInfo.detector_ = DETECTOR_UNDEF;

	return true;
}

// Return the mass spectrometer manufacturer (return 0 as success)
inline void InstrumentInfoParser::getMSManufacturer(const std::string &strLog, std::string &strManufacturer)
{
	getSampleLogEntry (strLog, "Manufacturer: ", strManufacturer);
	std::string::size_type nStartPos = strManufacturer.find_first_of(",\r\n");
	if (strManufacturer.npos != nStartPos) {
		strManufacturer.erase (nStartPos);
	}

	if (strManufacturer.empty()) {
		// hardcoded to "ABI" as the information seems unavailable from file
		strManufacturer = "ABI";
	}
}

inline void InstrumentInfoParser::getMSSerialNumber(const std::string &strLog, std::string &strSerialNumber)
{
	getSampleLogEntry (strLog, "Serial Number: ", strSerialNumber);
	if (strSerialNumber.empty ()) {
		getSampleLogEntry (strLog, "S/N: ", strSerialNumber);
	}
	std::string::size_type nStartPos = strSerialNumber.find_first_of(",\r\n");
	if (strSerialNumber.npos != nStartPos) {
		strSerialNumber.erase (nStartPos);
	}
}

inline void InstrumentInfoParser::getMSHardwareVersion(const std::string &strLog, std::string &strHardwareVersion)
{
	getSampleLogEntry (strLog, "Firmware Version: ", strHardwareVersion);
	if (strHardwareVersion.empty ()) {
		getSampleLogEntry (strLog, "Firmware Ver: ", strHardwareVersion);
	}
	std::string::size_type nStartPos = strHardwareVersion.find_first_of(",\r\n");
	if (strHardwareVersion.npos != nStartPos) {
		strHardwareVersion.erase (nStartPos);
	}
}

// Return the mass spectrometer ionisation (return 0 as success)
inline void InstrumentInfoParser::getMSIonisation(const std::string &strLog, std::string &strIonisation)
{
	// TODO:
	// does not seem to find this recorded much
	strIonisation.resize(0);
}

// Return the mass spectrometer mass analyzer (return 0 as success)
inline void InstrumentInfoParser::getMSMassAnalyzer(const std::string &strLog, std::string &strMassAnalyzer)
{
	// TODO:
	// does not seem to find this recorded much
	// maybe can guess from the model?
	strMassAnalyzer.resize(0);

	/*
	// TODO:
	// Seems like valid options for ABI instruments are:
	// "Q-TOF", "TOF", "TOF-TOF", "Q1", "Q3"
	// Question: Can we guess?

	const std::string prefix("Component ID: ");
	getSampleLogEntry (m_strSampleLog, prefix, strMassAnalyzer);
	*/
}

// Return the mass spectrometer detector (return 0 as success)
inline void InstrumentInfoParser::getMSDetector(const std::string &strLog, std::string &strDetector)
{
	// TODO:
	// does not seem to find this recorded much
	strDetector.resize(0);
}

//======================================================================

SoftwareInfoParser::SoftwareInfoParser(InstrumentInfo &instrumentInfo, int iVerbose)
:	m_instrumentInfo(instrumentInfo),
	m_iVerbose(iVerbose)
{
}

SoftwareInfoParser::~SoftwareInfoParser(void)
{
}

bool SoftwareInfoParser::parse(const std::string &strCreator)
{
	std::string strSoftware;
	getSoftwareVersion (strCreator, strSoftware, 
		m_instrumentInfo.acquisitionSoftwareVersion_);

	m_instrumentInfo.acquisitionSoftware_ = softwareToMSAcquisitionSoftwareType(strSoftware);
	return true;
}

const int WiffStructureLoader::MAX_TEXT = 0x200;
WiffStructureLoader::WiffStructureLoader(const std::string &strFilename, int iVerbose)
:	m_strFilename(strFilename),
	m_iVerbose(iVerbose),
	//m_strCreator(""),
	m_fHasWiffScanFile(false),
	m_iSampleTag(0),
	m_iSampleScanTag(0),
	m_loaded(false)
{
}

WiffStructureLoader::~WiffStructureLoader(void)
{
}


bool WiffStructureLoader::load(bool fReportStructure)
{
	m_strCreator.clear();
	m_fHasWiffScanFile=false;
	m_iSampleTag = 0;
	m_iSampleScanTag = 0;
	m_loaded = false;

	USES_CONVERSION;
	HRESULT hr;
	hr = StgIsStorageFile(A2W(m_strFilename.c_str()));

	if (FAILED(hr)) {
		if (STG_E_FILENOTFOUND==hr) {
			std::cerr << "Error: '" << m_strFilename.c_str() << "' does not exist!" << std::endl;
		} else if (S_FALSE==hr) {
			std::cerr << "Error: '" << m_strFilename.c_str() << "' does not seem like a valid wiff format!" << std::endl;
		} else {
			std::cerr << "Error: Could not access '" << m_strFilename.c_str() << "' instance, hr("
				<< std::hex << hr << std::dec <<")" << std::endl;
		}
		return false;
	}

	CComPtr<IStorage> pIStorage;
	hr = StgOpenStorage(A2W(m_strFilename.c_str()), NULL,
		STGM_DIRECT | STGM_READ | STGM_SHARE_DENY_WRITE,
		NULL, 0, &pIStorage);
	if (FAILED(hr)) {
		ExplainError(hr);
		return false;
	}

	if (fReportStructure)
		std::cout << "INFO: " << m_strFilename.c_str() << std::endl;
	hr = ReadStorage(pIStorage, 1, fReportStructure);
	if (fReportStructure)
		std::cout << "INFO: --- END wiff structure ---" << std::endl;
	if (FAILED(hr)){
		ExplainError(hr);
		return false;
	}

	m_fHasWiffScanFile=(m_iSampleTag != m_iSampleScanTag);
	m_loaded = true;
	return true;
}

bool WiffStructureLoader::isWiffScanFileExist()
{
	if (!m_loaded) load(false);
	if (!m_loaded) return false;
	if (!m_fHasWiffScanFile) return false;
	// check for the present of .wiff.scan
	std::string scanFile(m_strFilename);
	scanFile.append (".scan");
	return checkPath(scanFile);
}

HRESULT WiffStructureLoader::ReadStorage(IStorage* pIStorage, int nLevel, bool fReportStructure)
{
	HRESULT hr = S_OK;

	CComPtr<IEnumSTATSTG> pIEnumStatStg;
	hr = pIStorage->EnumElements(0,NULL,0,&pIEnumStatStg);
	if (FAILED(hr)) return hr;

	STATSTG statstg;
	hr = pIEnumStatStg->Next(1,&statstg,NULL);
	if (FAILED(hr)) {
		if ( statstg.pwcsName ) CoTaskMemFree(statstg.pwcsName);
		return hr;
	}

	USES_CONVERSION;
	while (SUCCEEDED(hr) && (hr != S_FALSE)) {
		if (STGTY_STORAGE == statstg.type) {
			const char *szName = W2A(statstg.pwcsName);
			if (fReportStructure)  LogStructurePrefix (nLevel, "[C]", szName);

			const char *szSampleTag = "Sample";
			if (2==nLevel && szName==strstr(szName, szSampleTag)) {
				if (strlen(szName)>strlen(szSampleTag)) {
					bool fNumericOnly = true;
					for (size_t i=strlen(szSampleTag); i<strlen(szName); i++) {
						if (szName[i]<'0' || '9'<szName[i]) {
							fNumericOnly = false;
							break;
						}
					}
					if (fNumericOnly) m_iSampleTag++;
				}
			}

			CComPtr<IStorage> pIChildStorage;
			hr = pIStorage->OpenStorage(statstg.pwcsName, 0,
				STGM_READ | STGM_SHARE_EXCLUSIVE,
				0, 0, &pIChildStorage);
			if ( FAILED(hr) ) {
				if ( statstg.pwcsName ) CoTaskMemFree(statstg.pwcsName);
				break;
			}

			hr = ReadStorage(pIChildStorage, nLevel+1, fReportStructure);
			if ( FAILED(hr) ) {
				if ( statstg.pwcsName ) CoTaskMemFree(statstg.pwcsName);
				break;
			}

		} else if (STGTY_STREAM == statstg.type) {
			const char *szName = W2A(statstg.pwcsName);
			if (fReportStructure) LogStructurePrefix (nLevel, "[S]", szName);

			if (3==nLevel && 0==strcmp(szName, "Scan")) {
				m_iSampleScanTag++;
			}

			CComPtr<IStream> pIStream;
			hr = pIStorage->OpenStream(statstg.pwcsName, 0,
				STGM_READ | STGM_SHARE_EXCLUSIVE,
				0, &pIStream);
			if ( FAILED(hr) ) {
				if ( statstg.pwcsName ) CoTaskMemFree(statstg.pwcsName);
				break;
			}

			TCHAR strData[MAX_TEXT+1] = {0};
			DWORD dwRead = 0;
			hr = pIStream->Read(strData,MAX_TEXT,&dwRead);
			if ( FAILED(hr) ) {
				if ( statstg.pwcsName ) CoTaskMemFree(statstg.pwcsName);
				break;
			}

			TextLines textLines;
			ExtractStreamText (szName, strData, dwRead, textLines);
			if (fReportStructure) {
				if (!textLines.empty()) {
					for(TextLines::const_iterator iter=textLines.begin();
						iter != textLines.end(); ++iter) {
						LogStructurePrefix (nLevel, "   ", iter->c_str());
					}
				}
			}

			// this is overloaded; for speed performance
			// we can write 2 separate routines for 
			// both creator version and external .wiff.scan file
			if (1==nLevel) {
				if (0==strcmp(szName, "FileRec_Str")) {
					// let's extract the file creator and version
					for(TextLines::const_iterator iter=textLines.begin();
						iter != textLines.end(); ++iter) {
						if (0==iter->find("Analyst")) {
							//Analyst QS 2.0&File Version:  1.00
							std::string creator(*iter);
							std::string::size_type nPos = creator.find("File");
							if (nPos != std::string::npos) {
								creator.erase (nPos);
								while (nPos >= 0) {
									if ('.'==creator[nPos] || ' '==creator[nPos]
										|| ('0'<=creator[nPos] && creator[nPos]<='9')
										|| ('A'<=creator[nPos] && creator[nPos]<='Z')
										|| ('a'<=creator[nPos] && creator[nPos]<='z')) {
										creator.erase (nPos+1);
										break;
									}
									nPos--;
								}
							}
							m_strCreator = creator;
						}
					}
				}
			}

		} // else if

		CoTaskMemFree(statstg.pwcsName);
		hr = pIEnumStatStg->Next(1, &statstg, NULL);
	} // while

	return hr;
}

void WiffStructureLoader::ExplainError (HRESULT hr)
{
	std::cerr << std::endl << "Error: Exception while opening wiff structure, COM error " 
		<< std::hex << hr << std::dec <<std::endl;
	printCOMErrorString (hr);
	if (STG_E_SHAREVIOLATION==hr || STG_E_LOCKVIOLATION==hr) {
		std::cerr << "INFO: Another application has opened and locked the file." << std::endl;
		std::cerr << "INFO: 1. Please restart AnalystService and retry if no application is using the service." <<std::endl;
		std::cerr << "INFO: 2. A reboot might help." << std::endl;
		std::cerr << "INFO: 3. If problem persist, please contact your administrator." << std::endl;
		std::cerr << std::endl;
	} else if (STG_E_ACCESSDENIED==hr) {
		std::cerr << "INFO: You do have have enough permissions to access the file or another application has opened and locked it." << std::endl;
		std::cerr << "INFO: 1. Please restart AnalystService and retry if no application is using the service." <<std::endl;
		std::cerr << "INFO: 2. A reboot might help." << std::endl;
		std::cerr << "INFO: 3. If problem persist, please contact your administrator." << std::endl;
		std::cerr << std::endl;
	} else if (STG_E_TOOMANYOPENFILES==hr) {
		std::cerr << "INFO: There is inadequate resource to open your data file." << std::endl;
		std::cerr << "INFO: 1. Please close some applications and retry." <<std::endl;
		std::cerr << "INFO: 2. A reboot might help." << std::endl;
		std::cerr << "INFO: 3. If problem persist, please contact your administrator." << std::endl;
		std::cerr << std::endl;
	} else if (STG_E_INSUFFICIENTMEMORY==hr) {
		std::cerr << "INFO: There is inadequate memory to process your data file." << std::endl;
		std::cerr << "INFO: 1. Please close some applications and retry." <<std::endl;
		std::cerr << "INFO: 2. A reboot might help." << std::endl;
		std::cerr << "INFO: 3. If problem persist, please contact your administrator." << std::endl;
		std::cerr << std::endl;
	} else if (STG_E_OLDDLL==hr) {
		std::cerr << "INFO: Your OLE DLLs on your processing workstation is older than the acquisition workstation." << std::endl;
		std::cerr << "INFO: Please make sure they are the same version." <<std::endl;
		std::cerr << std::endl;
	}
}

void WiffStructureLoader::LogStructurePrefix (int nLevel, const char* szCategory, const char *szName)
{
	std::cout << "INFO: ";
	for (int i=0; i<nLevel; i++) std::cout << "    ";
	std::cout << szCategory << " " << szName << std::endl;
}

void WiffStructureLoader::ExtractStreamText (const char *szName, const TCHAR *tchData, DWORD dwRead, TextLines &textLines)
{
	bool fExtractText=false;
	if ('A' == szName[0]) {
		fExtractText = (0==strcmp(szName, "AcqMethodFileInfoStm"));
	} else if ('C' == szName[0]) {
		fExtractText = (0==strcmp(szName, "CFRFileHeader"))
			|| (0==strcmp(szName, "CFR_INFO"));
	} else if ('D' == szName[0]) {
		fExtractText = (0==strcmp(szName, "DATA"))
			|| (0==strcmp(szName, "DabsInfo"))
			|| (0==strcmp(szName, "DeviceTable"));
	} else if ('F' == szName[0]) {
		fExtractText = (0==strcmp(szName, "FileRec_Str"));
	} else if ('G' == szName[0]) {
		fExtractText = (0==strcmp(szName, "GLPTables"));
	} else if ('L' == szName[0]) {
		fExtractText = (0==strcmp(szName, "Log"));
	} else if ('O' == szName[0]) {
		fExtractText = (0==strcmp(szName, "Opt2Rec"));
	} else if ('P' == szName[0]) {
		fExtractText = (0==strcmp(szName, "PeakFinderInfo"));
	} else if ('R' == szName[0]) {
		fExtractText = (0==strcmp(szName, "Rsc"));
	} else if ('\05' == szName[0]) {
		fExtractText = (0==strcmp(szName, "\05SummaryInformation"))
			|| (0==strcmp(szName, "\05DocumentSummaryInformation"));
	}

	if (!fExtractText) return;

	WCHAR *pWChar = (WCHAR*)tchData;
	DWORD pos = 0;
	std::string textline;
	while (pos < dwRead) {
		if ((' '<=*pWChar && *pWChar<='~') 
			|| '\t'==*pWChar || '\r'==*pWChar  || '\n'==*pWChar){
			textline.append(1, (char)(*pWChar));
		} else {
			if (!textline.empty()) {
				textLines.push_back (textline);
				textline.clear();
			}
		}
		pos++;
		pWChar++;
	}
	if (!textline.empty()) {
		textLines.push_back (textline);
	}
}

//======================================================================
