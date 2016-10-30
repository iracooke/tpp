// -*- mode: c++ -*-

/*
    Program: dta2mzxml
    Description: convert dta scan files to mzXML
    Date: March 27, 2004
    Revision: $Id: Dta2mzXML.cpp 6094 2013-02-20 00:04:47Z slagelwa $

    Copyright (C) 2004 Pedrioli Patrick, ISB, Proteomics


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

/***************************************************************************
    begin				: Sat Mar 27
    copyright			: (C) 2004 by Pedrioli Patrick, ISB, Proteomics
    email				: ppatrick@student.ethz.ch
	additional work	by	: Brian Pratt, Insilicos (brian.pratt@insilicos,com)
 ***************************************************************************/

#include <stdexcept>
#include <iomanip>
#include <map>
#ifndef WIN32
#include <netinet/in.h>	//htonl
#else
#include <winsock2.h> // hotonl
#endif // WIN32
#include "Dta2mzXML.h"

#ifdef WIN32
typedef unsigned int u_int32_t;
#endif // WIN32

using namespace std;

Dta2mzXML::Dta2mzXML() :
  scanNum_(0),
  totalNumScans_(0),
  numPackets_(0),
  precursorMz_(0),
  doCharge_(false),
  byName_(false),
  plusTwo_(false),
  recount_(false),
  verbose_(false),
  scanCountIsNumScans_(true),
  doGzip_(false)
{

}

Dta2mzXML::~Dta2mzXML() {
  if (byName_) { 
    ofstream_map::iterator i = outFiles_.begin();
    while (i != outFiles_.end()) { // Close the filesbc
      i->second->close();
      delete i->second;
      i++;
    }
  }
  else {
    (*fout_).close();
  }
}

// Given a list of dta (scan) files, filter for +2 restriction if plustwo is set
void Dta2mzXML::filterPlusTwo(std::vector<std::string> & fileNameVector) {

  if (!plusTwo_) return; // easy!  nothing to do

  
  // build a map which will only contain all scans with +2 charge
  map<int, int> scanMap;
  for (int i=0; i< (int)fileNameVector.size(); i++) {
    int curScanNum = extractScanNum(fileNameVector[i]);
    int curCharge =  extractScanCharge(fileNameVector[i]);
    if (scanMap.find(curScanNum)==scanMap.end()) {
      scanMap[curScanNum] = curCharge; // 2; the value doesn't really matter
    }
    else if (scanMap[curScanNum] > curCharge) {
      scanMap[curScanNum] = curCharge; // 2; the value doesn't really matter
    }
  }

  // now, go through the list of all filenames;
  // if a non-+2 scan is found, ignore it if there's a +2 scan
  // for the same scan number.  Otherwise, the non-+2 scan is ok.
  vector<string> plusTwoRestricted;

  for (int i=0; i< (int)fileNameVector.size(); i++) {
    bool keepScan = false;
    int curCharge =  extractScanCharge(fileNameVector[i]);
    int curScanNum = extractScanNum(fileNameVector[i]);
    if (scanMap.find(curScanNum) != scanMap.end() && curCharge == scanMap[curScanNum]) {
      // pass!
      keepScan = true;
    } 
    else {
      if (scanMap.find(curScanNum) == scanMap.end()) {
	// there is no +2 scan with the same scan number, so save this one
	keepScan = true;
      }
      // otherwise, there was already a +2 for this scan number, so ignore
    }
    
    if (keepScan) {
      plusTwoRestricted.push_back(fileNameVector[i]);
    }
    
    if (!keepScan && verbose_) {
      cout << "ignoring scan number " << curScanNum << ", charge "
	   << curCharge <<": ";
      cout << "keeping lowest charge only." << endl;
    }
  }

  fileNameVector.clear();
  for (int i=0; i<(int)plusTwoRestricted.size(); i++) {
    fileNameVector.push_back(plusTwoRestricted[i]);
  }
}

bool Dta2mzXML::setOutputStream(std::string& basename) {
  ofstream_map::iterator i = outFiles_.find(basename.c_str());
  ogzstream* tout;
  std::string fname = basename+".mzXML";
  if (doGzip_) {
 	fname += ".gz";
  }
  if (i == outFiles_.end()) {
    std::fstream tmp; 
    tmp.open(fname.c_str(), ios::in);
    if (tmp.is_open()) {
      cerr << "WARNING: File with name " << fname
	   << " already exists.  Refusing to overwrite and skipping!" << endl;
      tmp.close();
      return false;
    }
    tmp.close();
    tout = new ogzstream();
    char* key;
    key = new char[basename.length()+1];
    strcpy(key, basename.c_str());
    outFiles_.insert(std::pair<const char*, ogzstream*>(key, tout));
    //    outFiles_[basename.c_str()] = tout;
    tout->open(fname.c_str());
  }
  else {
    tout = i->second;
  }
  fout_ = tout;
  xmlName_ = fname;
  return true;
}


void Dta2mzXML::writeHeader(const std::vector<std::string> & fileNameVector) {
  int n=0;
  char lf = 0xA;
  string fileName;

  totalNumScans_ = (int)fileNameVector.size();

  if (verbose_) {
    cout << "writing mzXML header information for " << totalNumScans_ << " files" << endl;
  }


  (*fout_)	<< "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>" << lf
		<< "<mzXML" << lf
		<< " xmlns=\"http://sashimi.sourceforge.net/schema/\"" << lf
		<< " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << lf
		<< " xsi:schemaLocation=\"http://sashimi.sourceforge.net/schema/ http://sashimi.sourceforge.net/schema/MsXML.xsd\">" << lf
		<< " <msRun";


  if (scanCountIsNumScans_) {
    (*fout_) << " scanCount=\"" << totalNumScans_ << "\">" << lf;
  }
  else {
    // "scan count" should be the last actual index
    (*fout_) << " scanCount=\"" << scanNumVec_[totalNumScans_ - 1] << "\">" << lf;
  }


  // Generate sha1 hashes for all specified dta files (scans)
  for(n=0; n < totalNumScans_ ; n++ ) {
    fileName = fileNameVector[n];
 
    // Calculate sha1-sum
    if (verbose_) {
      cout << "Calculating sha1-sum of " << fileName << endl;
    }

    shaHash_[0] = 0;
    sha1_.Reset();
    if( !(sha1_.HashFile( (char*) fileName.c_str() )) ) {
      // Cannot open file
      throw runtime_error
	(string("Cannot open file ") + fileName + string(" for sha-1 calculation"));
    }
    sha1_.Final();
    sha1_.ReportHash( shaHash_, SHA1::REPORT_HEX );
    (*fout_) << "  <parentFile fileName=\"" << fileName << "\"" << lf
	  << "              fileType=\"RAWData\"" << lf
	  << "              fileSha1=\"" << shaHash_ << "\"/>" << lf;
  } // end sha1 hash generation


  (*fout_)	<< "  <dataProcessing>" << lf
	<< "    <software type=\"conversion\"" << lf
	<< "              name=\"dta2mzXML\"" << lf
	<< "	       version=\"" << DTA2MZXML_VERSION << "\"/>" << lf ;



  if( MIN_PEAKS_PER_SPECTRA > 0 ) {
    
    // Note the use of the name/value type element!
    (*fout_) << "    <processingOperation name=\"min_peaks_per_spectra\"" << lf
	  << "                         value=\"" << MIN_PEAKS_PER_SPECTRA << "\"/>" << lf;
    // And the comment field to give a little bit more information about the meaning of
    // the last element.
    (*fout_) << "    <comment>Scans with total number of peaks less than min_peaks_per_spectra were not included in this XML file</comment>" << lf;
  }

  (*fout_)	<< "  </dataProcessing>" << lf;

  
  if (verbose_) {
    cout << "completed writing mzXML header." << endl;
  }

}


void Dta2mzXML::writeScans(const std::vector<std::string> & fileNameVector) {
  scanNum_ = 0;
  scanNumVec_.clear();
  for (int i=0; i<(int)fileNameVector.size(); i++) { 
    writeScan(fileNameVector[i]);
  }  
}


/**
   convert a single dta file into base-64 encoding and create the
   corresponding node in the mzXML output file we're creating
 */

void Dta2mzXML::writeScan(const std::string & dtaName) {
  char	lf = 0xA;

  dtaName_ = dtaName;

  if (verbose_) {
      cout << "writing scan info for input file " << dtaName_ << endl;
  }

  // get the scan number: either from the filename, or incrementing
  // from zero if renumbering (recounting) scans
  if( !recount_ ) {
    // get MSMS scan number from filename
    // JMT todo: add exception and checking!
    scanNum_ = extractScanNum(dtaName);
  }
  else {
    // renumber scans
    scanNum_++;
  }

  scanNumVec_.push_back(scanNum_);

  // save the current file pointer; this will mark the beginning of
  // the mzXML "scan" node and we'll record this at the end of the
  // mzXML file for fast parsing.
  addScanToIndex();

  inDta_.open( dtaName_.c_str() );
  if( !inDta_.is_open() ) {
    throw runtime_error
      (string("Error opening file ") + dtaName_);
  }

  /**
     Now, read the m/z intensity pairs and convert them to binary
  */

  char	buf[256];

  // The first line in the dta file is the precursor ion; read it and adjust

  // we're done if we're at the end of the file
  if( inDta_.eof() ) {
    // JMT todo: throw exception here!
    return;
  }


  // split the line on whitespace
  // the line is 
  // precursorMH[ \t]precursorCharge
  inDta_.getline( buf , 256 );
  string inputline;

  inputline = buf;

  string::size_type separatorPos = inputline.find_first_of(" \t");
  if ( separatorPos == string::npos) {
    throw runtime_error
      (string("PrecursorMH and charge in dta file are not separated by space or tab: ")
       + inputline );
  }
  double precursorMH = atof ((inputline.substr(0, separatorPos)).c_str());
  int precursorCharge = atol ((inputline.substr(separatorPos)).c_str());

  // converting MH to M/Z
  precursorMz_ = (float) ((precursorMH + (precursorCharge - 1)*1.00794) / precursorCharge);

  peakDataVec_.clear();

  /**
     jmt: the peak data is stored as a list of lines, with [ \t]-separated pairs
     of m/z and intensity in ASCII.

     lines are read until the end of the file.

     a 'packet' refers to one m/z - intensity pair
  */

  // read the m/z intensity pairs until end of file reached;
  // build vector of PeakData objects

  while( !inDta_.eof() ) {
    PeakData peakData;
    
    inDta_.getline( buf , 256 );
    if( inDta_.eof() ) {
      break;
    }
    inputline = buf;
    separatorPos = string::npos;
    separatorPos = inputline.find_first_of(" \t");
    if ( separatorPos == string::npos) {
      throw runtime_error
	(string("expected [space or tab] seperator in ") 
	 + inputline);
    }
    peakData.mz_ = (float)atof ((inputline.substr(0, separatorPos)).c_str());
    peakData.intensity_ = (float)atof ((inputline.substr(separatorPos)).c_str());

    peakDataVec_.push_back(peakData);
  }

  numPackets_ = (int)peakDataVec_.size();

  // output mzXML scan header info;
  // the current index points to the start of this xml node
  (*fout_)	<< "  <scan num=\"" << scanNum_ << "\"" << lf
	<< "        msLevel=\"2\"" << lf
	<< "        peaksCount=\"" << numPackets_ << "\">" << lf;
  
  // output mzXML precusor information for this scan

  // JMT ???
  // We have to use an arbitray number
  (*fout_)	<< "    <precursorMz precursorIntensity=\"1\"";	

  if( doCharge_ ) {
    (*fout_) << lf 
	  << "                 precursorCharge=\"" << precursorCharge << "\">";
  }
  else {
    (*fout_) << ">";
  }

  (*fout_) << setprecision(8) << precursorMz_ << "</precursorMz>" << lf;

  /* convert to network byte order */
  

  /*    
    JMT:
    This code is dependent on machine-size for data types.
    We need some asserts here.

    Bascially, the dta file contains float data; We'll need to encode
    this in network byte order, and then base-64 encode *that*.

    To encode in network byte order: treat a float (assuming it's 4
    bytes long) as an unsigned int32 (4 bytes), and use htonl()
    

    on a pentium:

    size float: 4
    size double: 8
    size int: 4
    size unsigned int: 4
    size u_int32_t: 4
    size long: 4
  */



  /*
    Now that we've nicely stored the peak data in objects, we'll splat
    it out in a long linear list of bytes; alternating m/z (4 bytes,
    network encoded) with intensity (4 bytes, network encoded).
  */  
  void* peakDataNetworkBuf;

  // the actual number of floats read: 2 * (# of peaks), as each peak
  // is a pair of floats
  long numFloats = 2 * numPackets_;


  int bytesPerFloat = sizeof(float);
  int numNetworkDataBytes = bytesPerFloat * numFloats;

  
  // allocate the long linear list of bytes; we'll need the same
  // storage space as we started with.
  if ( 
      (peakDataNetworkBuf =  malloc(numNetworkDataBytes)) 
      == 
      NULL
       ) {
    throw runtime_error
      (string("Cannot allocate memory for byte conversion!"));
  }

  /*  now, do the conversion from host to network byte order */

  int n;
  int j=0;
  for (n=0; n<(int)peakDataVec_.size(); n++) {
    /*
      from man pages:
      uint32_t htonl(uint32_t hostlong);
      The htonl() function converts the unsigned integer hostlong from host byte order to network byte order.
      
      On the i80x86 the host byte order is Least Significant Byte
      first, whereas the network byte order, as used on the
      Internet, is Most Significant Byte first.
    */

    
    // get the 4 bytes that represent a float:
    // get the addr of the float, treat it as a uint32 ptr, and dereference
    u_int32_t mzBytes = (*(u_int32_t *)(&(peakDataVec_[n].mz_)));

    // pretend that peakDataNework stores 4-byte unsigned ints, rather
    // than 4-byte floats, for the sake of the conversion
    ( (u_int32_t *) peakDataNetworkBuf)[j] = htonl(mzBytes);
    j++;

    // JMT: some error checking, at least cout, like old code below?
    //cout << peakDataVec_[j] << " " << ((float*)peakDataNetworkBuf)[j] << " " << (float) ntohl( ((unsigned int *)peakDataNetworkBuf)[j]) << endl;
    

    u_int32_t intensityBytes = (*(u_int32_t *)(&(peakDataVec_[n].intensity_)));
    ( (u_int32_t *) peakDataNetworkBuf)[j] = htonl(intensityBytes);
    j++;
  }

  // JMT: we could assert that n*2 == j here

	

  /**
     base64 encode


     Refer to Base64.h|cpp comments
     

     remembering that floats are stored as 4 bytes:

     sizeof(float) = 4,
     (number of floats * 4) = number of  network-encoded data bytes
  */


  int encodedLength = 
    base64_.b64_encode(	       
		       // bytes to convert (network-ordered data)
		       (const unsigned char *) peakDataNetworkBuf, 

		       // number of input bytes to convert
		       numNetworkDataBytes); 
  // JMT todo: assert encodedLength > 0

  //
  // free the network-ordered array
  //
  free(peakDataNetworkBuf);


  
  // finally, output the base-64 encoded list of network-ordered (mz,
  // intensity) pairs
  (*fout_)	<< "    <peaks precision=\"32\">"
	<< base64_.getOutputBuffer()
	<< "</peaks>" << lf;

  //
  // release the base64-encoded character buffer
  //
  base64_.freeOutputBuffer();


  /* end base64-encoding! */


  /*
    If we're renumbering the scan numbers, add the original scan
    number (filename) in a nameValue field so we keep some record of it. 
  */
  if( recount_ ) {
    (*fout_) << "    <nameValue name=\"original dta name\" value=\"" << dtaName_ << "\" type=\"string\"/>" << lf;
  }
  (*fout_)	<< "  </scan>" << lf;


  // Close the dta input-file we just read
  inDta_.close();
  // and reset input stream state for next access
  inDta_.clear();
}


/**
   based on the dta filename, extract the scan number.

   assume the format is
   xxx.xxxx.dddd.xxx.xxxx
   and take the integer value of dddd
   
 */
int Dta2mzXML::extractScanNum(const string& dtaFileName)
{

  std::string::size_type pos = dtaFileName.rfind("/");

  if (pos == string::npos)  {
    pos = dtaFileName.rfind("\\");
  }

  if (pos == string::npos) { 
    pos = 0;
  }

  for( int n=0 ; n < 2 ; n++) {
      pos = dtaFileName.find("." , pos);
      pos++;
  }	
  
  std::string::size_type pos2 = dtaFileName.find("." , pos);
  string scanNum( dtaFileName , pos , pos2-pos ); /* not always 4 digits for scan # */
  return atoi( scanNum.c_str() );
}


/**
   based on the dta filename, extract the charge

   assume the format is 
   xxxx.xxxx.xxxx.SCANCHARGE.dta
*/
int Dta2mzXML::extractScanCharge(const string& dtaFileName) {

  std::string::size_type pos = dtaFileName.rfind("/");

  if (pos == string::npos)  {
    pos = dtaFileName.rfind("\\");
  }

  if (pos == string::npos)  {
    pos = 0;
  }

  for( int n=0 ; n < 3 ; n++) {
      pos = dtaFileName.find("." , pos);
      pos++;
    }	
  string scanCharge( dtaFileName , pos , 1 );
  //cerr << scanCharge << endl;
  return atoi( scanCharge.c_str() );
}


// save the current file pointer position as the start of the current scan
void Dta2mzXML::addScanToIndex() {
  (*fout_).flush();
  indexVec_.push_back((*fout_).bytes_written());
}


/** 
    output a list of file-offset pointers into the mzXML file itself;
    this point to the scan nodes for fast access.
*/
void Dta2mzXML::writeXmlIndex()
{
  char	lf = 0xA;
  gzstream_fileoffset_t	indexOffset;

  // Save the offset for the indexOffset element
  (*fout_).gzflush();
  indexOffset = (*fout_).bytes_written(); // count of bytes into the gzip (if we're gzipping)

  (*fout_) << "  <index name=\"scan\">" << lf;
  for( int n = 0 ; n < totalNumScans_ ; n++ ) {
    (*fout_) << "    <offset id=\"" << scanNumVec_[n] << "\">" << indexVec_[n] << "</offset>" << lf;
  }
  (*fout_) << "  </index>" << lf;
  (*fout_) << "  <indexOffset>" << indexOffset << "</indexOffset>" << lf;

}


/**
   we include the sha1-hash 
   *of the mzXML file itself* 
   *in the mzXML file itself*.
   
   the hash is calculated on the entire file length, up to the start of the <sha1> tag.

*/
void Dta2mzXML::writeXmlSha1()
{
  char	lf = 0xA;

  // Sha1 sum of the mzXML file goes till the end of the opening tag
  // of the sha1 element
  (*fout_) << "  <sha1>";

  // Make sure everything is printed before starting calculation.
  (*fout_).gzflush();

  // Clean up and calculate
  if (verbose_) {
    cout << "Calculating sha1-sum of " << xmlName_ << endl;
  }
  shaHash_[0] = 0;
  sha1_.Reset();

  if( !(sha1_.HashGZFile( (char *)xmlName_.c_str() )) ) {
    throw runtime_error
      (string("Cannot open mzXML output file ") + xmlName_ + string(" for mzXML self-SHA1 calculation"));
  }
  sha1_.Final();
  sha1_.ReportHash( shaHash_, SHA1::REPORT_HEX );

  (*fout_) << shaHash_ << "</sha1>" << lf;
  if (verbose_) {
    cout << "done with mzXML sha1 hash\n";
  }
}
