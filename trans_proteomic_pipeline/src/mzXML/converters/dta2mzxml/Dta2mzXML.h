// -*- mode: c++ -*-

/*
    Program: dta2mzxml
    Description: convert dta scan files to mzXML
    Date: March 27, 2004

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



#ifndef _INCLUDED_DTA2MZXML_H_
#define _INCLUDED_DTA2MZXML_H_


/** @file Dta2mzXML.h
    @brief converts dta input file(s) to a single mzXML output file
*/




#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "common/tpp_hashmap.h"
#include "mzXML/common/SHA1.h"
#include "mzXML/common/Base64.h"




// currently mostly unused!! 
// JMT todo: in writeScan, take MIN_PEAKS_PER_SPECTRA into
// account before writing <scan> node

// scans (dta files) with total number of peaks less than
// min_peaks_per_spectra will be excluded from the mzXML output file
#define MIN_PEAKS_PER_SPECTRA 0





// program version; stored in mzXML output file
#define	DTA2MZXML_VERSION 0.3





/**

    jmt changes:

    general code cleanup

    general commenting, formatting


    removed namespace pollution (using namespace std; in header)
    
    removed seemingly unnecessary inline directives

    removed unnecessary public wrappers of private functions


    todo: add done flag;if destroyed before done, remove output file
    should "-charge" affect precursorMz modification, or not?

    


    -- original documentation --


    massWolf
    
    This program converts MassLynx native data into mzXML format. The program 
    requires the DAC library from Waters to run.

    -------------------
    begin				: Sat Mar 27
    copyright			: (C) 2004 by Pedrioli Patrick, ISB, Proteomics
    email				: ppatrick@student.ethz.ch
    additional work	by	: Brian Pratt, Insilicos (brian.pratt@insilicos,com)
    Natalie Tasman, ISB (Aebersold Lab)
*/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/





#include "gzstream.h" // for ogzfopen decl



struct eqstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;
  }
};


/**
   simple m/z peak representation
 */



struct PeakData {
  float mz_;
  float intensity_;

  PeakData() :
    mz_(0),
    intensity_(0) {
  }
};



/**
   dta (scan file) to mzXML converter
 */

class Dta2mzXML {
 protected:

  // methods
  void addScanToIndex();
  void writeXmlScan();
  void writeXmlIndex();
  void writeXmlSha1();

  // get scan number from filename
  int extractScanNum(const std::string& dtaFileName);
  
  // get the charge from the filename
  int extractScanCharge(const std::string& dtaFileName);



  /* variables */

  // general

  // sha1 hash generating object
  SHA1	sha1_;
  // sha1 hash result
  char shaHash_[1024];


  // base64 encoder/decoder object
  Base64 base64_;



  // output filestream
  ogzstream* fout_;
  bool doGzip_; // if true, write out as gzip file

  // keep track of the byte position of each scan record in the mzXML
  // file; this is saved at the end of the file for fast reparsing
  std::vector<gzstream_fileoffset_t> indexVec_;

  // keep track of the scan number of each scan
  std::vector<int> scanNumVec_;


  std::string xmlName_;

  // program behavoir:


  //bool centroid_; // currently unused

  bool verbose_;

  bool byName_;   // Separate dta files into mzXML files by the first 
                  // part of the name

  bool doCharge_; // if true, the dta-specificed precursor charge is
		  // stored in the mzXML "scan" element; otherwise, 
		  // no charge info is included in the output

  bool recount_;  // if true, renumber scans from 0 in mzXML output;
		  // otherwise, extract the scan number from the dta
		  // filename

  bool plusTwo_;  // if true, if a +2 dta file exists, ignore other
		  // files for the same scan with different charges.

  
  bool scanCountIsNumScans_; // if true, the mzXML header's
			     // 'scanCount' will be the standard
			     // number of scans in the file; if false,
			     // it will be the number of the last
			     // scan.  This hack-ish change deals with
			     // cases where scan numbers are not
			     // contiguous and not renumbered.
  

  // Header information
  char pAcquisitionFileName_[250];
  int totalNumScans_; // number of dta files to process




  /* Scan information */
  
  // current input dta (scan) file
  std::ifstream inDta_;
  
  // current input dta filename
  std::string dtaName_;

  // current scan number; used in mzXML (output) "scan" node
  int scanNum_;

  // number of peaks in current scan
  long numPackets_;

  // peak (m/z, intensity) list
  std::vector<PeakData> peakDataVec_;
  float	precursorMz_;

  // mzXML file streams hash
  typedef TPP_CONSTCHARP_HASHMAP(ogzstream *) ofstream_map;
  ofstream_map outFiles_;



 public:

  Dta2mzXML();
  ~Dta2mzXML();
  
  bool setOutputStream(std::string& basename);
 

  /* command-line switch behavior modification */
  void setDoCharge() {
    doCharge_ = true;
  }

  
  void setPlusTwo() {
    plusTwo_ = true;
  }

  void setRecount() {
    recount_ = true;
    scanNum_ = 0;
  }

  
  void setVerbose() {
    verbose_ = true;
  }

  void setDoGzip() {
    doGzip_ = true;
  }

  void setByName() {
    byName_ = true;
  }

  void init();
  void setScanCountIsNumScans() {
    scanCountIsNumScans_ = true;
  }
  
  void setScanCountIsMaxScanNum() {
    scanCountIsNumScans_ = false;
  }

  // Given a list of dta (scan) files, filter for +2 only if plustwo is set
  void filterPlusTwo(std::vector<std::string> & fileNameVector);
  

  // Given a list of dta (scan) files, produce the mzXML header information
  void writeHeader(const std::vector<std::string> & fileNameVector);


  // Given a dta (scan) file, produce the mzXML "scan" output
  void writeScan(const std::string & dtaName);

  // Given a list of dta (scan) files, produce the mzXML "scan" output
  // for each file.
  void writeScans(const std::vector<std::string> & fileNameVector);
  

  void finalizeXml() {
    if (verbose_) {
      std::cout << "Finalizing:\n";
    }

    char lf = 0xA;
    (*fout_) << " </msRun>" << lf;

    if (verbose_) {
      std::cout << "  writing the index\n";
    }
    writeXmlIndex();


    if (verbose_) {
      std::cout << "  writing the SHA1\n";
    }
    writeXmlSha1();

    (*fout_) << "</mzXML>" << lf;


  }

};



#endif // header guards
