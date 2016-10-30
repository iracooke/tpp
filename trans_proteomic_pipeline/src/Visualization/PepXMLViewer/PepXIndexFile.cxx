// -*- mode: c++ -*-



/*
  Program: PepXMLViewer.cgi
  Description: CGI program to display pepxml files in a web browser
  Date: July 31 2006

  Copyright (C) 2006  Natalie Tasman (original author), ISB Seattle

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

  Natalie Tasman
  Institute for Systems Biology
  401 Terry Avenue North
  Seattle, WA  98109  USA
    
  email (remove underscores):
  n_tasman at systems_biology dot org
*/

//for fstat
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//for mkstemp
#include <cstdlib>
#include <cstdio>

#ifndef WINDOWS_NATIVE		//conditional compile for windows port - Nate H insilicos 7.12.07
#include <unistd.h>
#else
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <stdlib.h>

#define _S_IREAD_x 256	//mkstemp emulation courtesy of google
#define _S_IWRITE_x 128
int mkstemp(char *tmpl)
{
  int ret=-1;
  mktemp(tmpl); ret=open(tmpl,O_RDWR|O_BINARY|O_CREAT|O_EXCL|_O_SHORT_LIVED, _S_IREAD_x|_S_IWRITE_x);
  return ret;
}
#endif // WINDOWS_NATIVE

#include <math.h> // ceil
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>

#include <boost/regex.hpp>

#include "common/util.h"

#include "PepXUtility.h"

#include "PepXIndexFile.h"

using std::exception;
using std::runtime_error;
using std::cerr;
using std::endl;
using std::ios;
using std::stringstream;
using std::istringstream;
using boost::regex;
using boost::regex_search;




#define INDEX_FILE_FORMAT_VERSION_STR "1.5"








// IndexDataSorter: helper class

bool
IndexDataSorter::operator()( IndexSortData* const& A,  IndexSortData* const& B) {
  static double valA, valB;
  static string strA, strB;
   
  //  cout << A << endl;

  strA=A->data_;
  strB=B->data_;
  valA = -999;
  valB = -999;



  // text fields
  if (field_.fieldName_ == "peptide" ||
      field_.fieldName_ == "protein" ||
      field_.fieldName_ == "spectrum" ||
      field_.fieldName_ == "ions") {


    // compare text
    if (sortDir_) {
      return (strA>=strB);
    }
    else {
      return (strA<strB);
    }
  } 

  // convert strings to numeric
  if (strA == "") {
    valA = 0;
  }
  else {
    if (!toDouble(strA, valA)) return false;
  }

  if (strB == "") {
    valB = 0;
  }
  else {
    if (!toDouble(strB, valB)) return false;
  }

  // compare numeric
  if (sortDir_) {
    //cout << valA << " > " << valB << "?";
    //cout << (valA > valB) << endl;
    return (valA > valB);
  }
  else {
    //cout << valA << " < " << valB << "?";
    //cout << (valA < valB) << endl;
    return (valA < valB);
  }


}








IndexFile::IndexFile() {

}

void
IndexFile::reset(PipelineAnalysis& pipeline,
		 Options& options) {
  pipeline_ = &pipeline;
  options_ =  &options;

  // bpratt note: this used to replace the extension, but that
  // can lead to ambiguity (think foo.xml and foo.pepxml) - 
  // better to append the extension
  indexFileName_ = pipeline_->xmlFileName_+".index";

  if (tmpFileName_.length()) {
    verified_unlink(tmpFileName_);
    tmpFileName_="";
  }
  numAcceptedSQ_ = 0;
  totalNumSQ_ = 0;

  numSingleHitProteins_ = 0;

  minimumSortFilterFields_.clear();

  vector<string> minimumSortFilterColumns_;
  options_->generateMinimumSortFilterInfo(minimumSortFilterColumns_);

  for (unsigned int i=0; i<minimumSortFilterColumns_.size(); i++) {
    minimumSortFilterFields_.push_back(Field(minimumSortFilterColumns_[i]));
  }


}



void
IndexFile::writeIndexHeader(void) {
  foutTmp_.close();

  fout_.open(indexFileName_.c_str(), ios::out|ios::binary);
  if (fout_.bad() || fout_.fail()) {
    throw runtime_error("can't open index file for output: " + indexFileName_ + string("; check permissions"));
  }

//  int i;

  //debug(options.print()); nate h

  fout_ << INDEX_FILE_FORMAT_VERSION_STR << endl;
  fout_ << pipeline_->xmlFileName_ << endl;

  // get the filelength and modification time
  time_t modTime;
  unsigned long long filelength;
  {
    struct stat filestats;
    int status;
    status = stat(pipeline_->xmlFileName_.c_str(), &filestats);
    modTime = filestats.st_mtime;
    filelength = filestats.st_size;
  }


  fout_ << filelength << endl;
  fout_ << modTime << endl;


  fout_ << totalNumSQ_ << endl;
  fout_ << numAcceptedSQ_ << endl;
  fout_ << pipeline_->numUniquePeptides_ << endl;
  fout_ << pipeline_->numUniqueStrippedPeptides_ << endl;
  fout_ << pipeline_->numUniqueProteins_ << endl;
  fout_ << pipeline_->numSingleHitProteins_ << endl;


  // make a string representing the serialization of options
  string serializedOptions;
  if(0){ //DDS: DISABLE BOOST serialization
    stringstream s;
    boost::archive::text_oarchive oa(s);
    //Options* const tmpOptions = new Options(options);
    //oa << (*tmpOptions);
    const Options tmpConstOptions(*options_);
    oa << tmpConstOptions;
    //oa << options;
    serializedOptions = s.str();
    //fout_ << serializedOptions << endl;
    //fout_ << "writing: " << endl;
    //tmpConstOptions.print();
    //delete tmpOptions;
  }

  //fout_ << options.queryString_ << endl;
  //fout_ << serializedOptions << endl;
  options_->write(fout_);
  fout_ << "***" << endl;


  

  fout_ << pipeline_->mrsOffsets_.size() << endl;
  for (unsigned i=0;i< pipeline_->mrsOffsets_.size(); i++) {
    fout_ << pipeline_->mrsOffsets_[i] << endl;
  }



  // open the temp file and copy it here

  ifstream fin(tmpFileName_.c_str(), ios::in|ios::binary);  
  string inputLine;
  // mark the beginning of the data section
  fout_ << "<<<" << endl;
  while (!fin.eof()) {
    getline(fin, inputLine);
    fout_ << inputLine << endl;
  }
  fin.close();
  fout_.close();

  // remove the temp file
  verified_unlink(tmpFileName_.c_str());
}















void
IndexFile::writeColumnHeader(void) {
  // create a uniquely named tempfile
  if (tmpFileName_.length()) {
    verified_unlink(tmpFileName_); // blow away previous if any
  }
  tmpFileName_ = "/tmp/PepXMLViewerXXXXXX";
  replace_path_with_webserver_tmp(tmpFileName_); // do this in designated tmpdir, if any
  safe_fclose(FILE_mkstemp(tmpFileName_)); // create then close a uniquely named file

  foutTmp_.open(tmpFileName_.c_str(), ios::out|ios::binary);

  if (foutTmp_.bad() || foutTmp_.fail()) {
    throw runtime_error("can't open tmp index file for output: " + tmpFileName_ + string("; check permissions and disk space"));
  }

  foutTmp_ << "startOffset\tendOffset\tMRS";
  for (unsigned int i=0; i<minimumSortFilterFields_.size();i++){
    foutTmp_ << "\t" << minimumSortFilterFields_[i].getFieldParamName();
  }
  foutTmp_ << endl;
}







void
IndexFile::generateFilteredListFromIndex(vector<IndexSortData*>& filteredList) {
  try{

    proteinList_.clear();
    peptideList_.clear();
    strippedPeptideList_.clear();
    numAcceptedSQ_ = 0;
    numSingleHitProteins_ = 0;
    totalNumSQ_ = 0;
  
    /**
       open the index file,
       read down to the data section,
       make sure the headers are as expected,
       run filter tests,
       if ok, save in filteredList
    */
    ifstream fin(indexFileName_.c_str(), ios::in|ios::binary);
    string inputLine;
    // read to the data section marker
    while (!fin.eof() && (inputLine != "<<<")) {
      getline(fin, inputLine);
    }
    // assert input line is "<<<"

    // the next line is the header line;

    // the first three fields are soffset, eoffset, and mrs


    // each string maps to it's col position, starting with 0
    map<string, int> foundColumns;
    // init this map based on what we'll expect; note that we only care
    // about our current min. reqs, which might be less than what's
    // actually there.
    foundColumns["startOffset"] = -1;
    foundColumns["endOffset"] = -1;
    foundColumns["MRS"] = -1;
    for (unsigned int i=0; i<minimumSortFilterFields_.size();i++){
      foundColumns[minimumSortFilterFields_[i].getFieldParamName()] = -1;
    }

  

    getline(fin, inputLine);
    {
      istringstream ss(inputLine);
      string colName;
      int colPos = 0;
      while (!ss.eof()) {
	ss >> colName;
	//cout << "got " << colName << endl;
	foundColumns[colName] = colPos;
	++colPos;
	// asapratio takes up two cols in actual data lines
	// so does protein, with #alt prot following
	if (colName == "Qasapratio" ||
	    colName == "Gprotein") {
	  ++colPos;
	}
      }
    }
    // check to make sure that everything we expected was there.
    for (unsigned int i=0; i<minimumSortFilterFields_.size();i++){
      if (foundColumns[minimumSortFilterFields_[i].getFieldParamName()] < 0) {
	// error!
      }
    }
  
    // read line-by-line;
    // make a vector of expected fields.

    vector<string> lineData;
    istringstream ss;
    string colData;
    //    map<string,long long>::iterator it;
    while (!fin.eof()) {
      lineData.clear();
      ss.clear();
      getline(fin, inputLine);
    

      if (inputLine != "" ) {

	totalNumSQ_++;

	//cout << inputLine << endl;

	// tokenize on '/t', yes, late at night...
	string::size_type spos=0, epos=0;
	bool pdone=false;
	while (!pdone) {
	  epos = inputLine.find_first_of('\t', spos);
	  if (epos == string::npos) {
	    lineData.push_back(inputLine.substr(spos));
	    //cout << "got *" << inputLine.substr(spos) << "*(last)" << endl;
	    pdone = true;
	  }
	  else {
	    lineData.push_back(inputLine.substr(spos, epos-spos));
	    //cout << "got *" << inputLine.substr(spos, epos-spos) << "*" << endl;
	    spos = epos+1;
	  }
	}


	//       ss.str(inputLine);
	//       while (!ss.eof()) {
	// 	ss >> colData;
	// 	lineData.push_back(colData);
	//       }
 
	//       // print out
	//       for (it=foundColumns.begin(); it != foundColumns.end(); it++) {
	// 	cout << (*it).first << ": " << lineData[(*it).second] << endl;
	//       }
	//       cout << endl << endl;
      






	// filter; if it's good, store in the 'good' list
      
	bool filterPass = false;

	bool done = false;
	while (!done) {
	
	  // check charges
	  if (options_->excludeCharge1 ||
	      options_->excludeCharge2 ||
	      options_->excludeCharge3 ||
	      options_->excludeChargeOthers) {
	    int chargeCol = foundColumns["Gassumed_charge"];
	    int assumedCharge = -1;

	    if (!toInt(lineData[chargeCol], assumedCharge)) {
	      assumedCharge = -1;
	    }

	    if (options_->excludeCharge1) {
	      if ( assumedCharge == 1) {
		done = true;
		break;
	      }
	    }
	
	    if (options_->excludeCharge2) {
	      if (assumedCharge == 2) {
		done = true;
		break;
	      }
	    }
	
	    if (options_->excludeCharge3) {
	      if (assumedCharge == 3) {
		done = true;
		break;
	      }
	    }

	    if (options_->excludeChargeOthers) {
	      if (assumedCharge != 1 &&
		  assumedCharge != 2 &&
		  assumedCharge != 3) {
		done = true;
		break;
	      }
	    }
	  }

	  // check XPRESS minimum area
	  if (options_->requireXPRESSMinArea) {
	    int lightAreaCol = foundColumns["Qlight_area"];
	    int heavyAreaCol = foundColumns["Qheavy_area"];
	    double lightArea, heavyArea;

	    // these safe casts will return 0 for a bad cast (non-numeric value, etc.)
	    toDouble(lineData[lightAreaCol], lightArea);
	    toDouble(lineData[heavyAreaCol], heavyArea);

	    // if both light and heavy are below required value, fail this entry
	    if ( (lightArea < options_->requiredXPRESSMinArea) &&
		 (heavyArea < options_->requiredXPRESSMinArea) ) {
	      done = true;
	      break;
	    }
	  }


	  
	  // check spectrum name
	  if (options_->requireSpectrumText) {
	    regex regSpectrum;
	    regSpectrum = options_->requiredSpectrumText;
	    int specCol = foundColumns["Gspectrum"];
	    string spectrumText = lineData[specCol];
	    if (!regex_search(spectrumText, regSpectrum)) {
	      done = true;
	      break;
	    }
	  }



	  // check peptide
	  int pepCol = foundColumns["Gpeptide"];
	  string peptide = lineData[pepCol];
	  static regex regPeptide;

	  // check for NxS|T glycosolation motif
	  if (options_->requireGlyc) {

	    // N-linked glycosylation motif: X is any AA except pro (P)
	    static const regex NxS_T("(?:N[A-OQ-Z](?:S|T))+");
	    
	    // sometimes B is also used with N
	    static const regex NBxS_T("(?:[NB][A-OQ-Z](?:S|T))+");

	    // (?:pattern) lexically groups pattern, without generating an additional sub-expression.

	    if (options_->includeBinGlycMotif) {
	      // use the [NB] version
	      if (!regex_search(peptide, NBxS_T)) {
		done=true;
		break;
	      };
	    }
	    else {
	      // use the regualar glyc motif
	      if (!regex_search(peptide, NxS_T)) {
		done=true;
		break;
	      };
	    }
	  }

	  // check required peptide AAs (string of single-letter AA codes)
	  if (options_->requireAA) {
	    static string::iterator si;
	    for (si = options_->requiredAA.begin();si != options_->requiredAA.end(); si++) {
	      char buf[2];
	      buf[0] = (*si);
	      buf[1] = 0;
	      string sc;
	      //sc += "[A-Z]*";
	      sc += buf;
	      //sc += "[A-Z]*";
	      regPeptide = sc;
	      //cout << "***testing req AA: " << sc << endl;
	      if (!regex_search(peptide, regPeptide)) {
		done=true;
		break;
	      }
	    }
	  }


	  // required peptide text
	  if (options_->requirePeptideText) {
	    regex regPeptide;
	    int pepCol;
	    if (options_->includeRequiredPepTextMods) {
	      pepCol = foundColumns["GpeptideModText"]; // check
	    }
	    else {
	      pepCol = foundColumns["Gpeptide"];
	    }

	    string peptideText = lineData[pepCol];
	    regPeptide = options_->requiredPeptideText;
	    if (!regex_search(peptideText, regPeptide)) {
	      done=true;
	      break;
	    }
	  }



	  // check required protein text
	  if (options_->requireProteinText) {
	    regex regProtein;
	    int protCol = foundColumns["Gprotein"];
	    string proteinText = lineData[protCol];
	    regProtein = options_->requiredProteinText;
	    if (!regex_search(proteinText, regProtein)) {
	      done=true;
	      break;
	    }
	  }





	  if (options_->requireIonscoreGTidentityscore) {
	    double ionscore, identityscore;
	    int iscoreCol, idscoreCol;
	    iscoreCol=foundColumns["Sionscore"];
	    idscoreCol=foundColumns["Sidentityscore"];
	    toDouble(lineData[idscoreCol], identityscore);
	    toDouble(lineData[iscoreCol], ionscore);
	    //cout << ionscore << " ? " << identityscore << "<br/>" << endl;
	    if (! (ionscore > identityscore)) {
	      done=true;
	      break;
	    }
	  }

	  if (options_->requireIonscoreGThomologyscore) {
	    double ionscore, homologyscore;
	    int iscoreCol, hscoreCol;
	    iscoreCol=foundColumns["Sionscore"];
	    hscoreCol=foundColumns["Shomologyscore"];
	    toDouble(lineData[hscoreCol], homologyscore);
	    toDouble(lineData[iscoreCol], ionscore);
	    //cout << ionscore << " ? " << homologyscore << "<br/>" << endl;
	    if (! (ionscore > homologyscore)) {
	      done=true;
	      break;
	    }
	  }

	  if (options_->requireXPRESS) {
	    int xCol = foundColumns["Qxpress"];
	    if (lineData[xCol] == "") {
	      done=true;
	      break;
	    }
	  }

	  if (options_->requireASAP) {
	    int aCol = foundColumns["Qasapratio"];
	    if (lineData[aCol] == "") {
	      done=true;
	      break;
	    }
	  }


	  if (options_->requireASAPConsistent) {
	    // *only fails if both asap and xpress are actually present*

	    int xCol = foundColumns["Qxpress"];
	    int aCol = foundColumns["Qasapratio"];

	    if (lineData[xCol] != "" &&
		lineData[aCol] != "") {

	      double xpress, asapmean, asaperror;

	      toDouble(lineData[xCol], xpress);
	      toDouble(lineData[aCol], asapmean);

	      if (1
		  //		xpress != -1 
		  //		&&
		  //		asapmean != -1 
		  // 		&&
		  // 		asapmean != 999
		  ) {
		// asap takes up two col in a data line
		toDouble(lineData[aCol+1], asaperror);

		if (! 
		    (xpress <= (asapmean + asaperror)) 
		    ||
		    (xpress >= (asapmean - asaperror))
		    ) {
		  done=true;
		  break;
		}
	      }
	    }
	  }  // require consistent






	  // look in option's filterMap

	  // go through all the filters
	  map< string, FilterVec >::const_iterator it;  
	  for (it=options_->filterMap_.begin();it != options_->filterMap_.end();it++) {

	    //string nodeName = (*it).first;
	    //cout << "filters for " << nodeName;
	    //cout << "(" << (*it).second.filterVec_.size() << " filters)" << endl;
	    for (unsigned int j=0; j<(*it).second.filterVec_.size(); j++) {
	      bool checkCharge = false;
	      if (((*it).second.filterVec_[j])->chargeSpecfication_) {
		checkCharge = true;
		int chargeCol = foundColumns["Gassumed_charge"];
		int assumedCharge = -1;
		toInt(lineData[chargeCol], assumedCharge);
		// skip this filter if it's charge-specific and the wrong charge
		if (((((*it).second.filterVec_[j])->charge_)
		     != assumedCharge)) {
		  continue;
		}
	      }
	    
	      string filterFieldName = 
		(((*it).second.filterVec_[j])->field_).getFieldParamName();
	      int filterCol = foundColumns[filterFieldName];


	      double nodeVal = 0;
	      double testVal = 0;
	      if (!toDouble(lineData[filterCol], nodeVal)) {
		// lex. cast failed-- maybe missing line, such as blank asap ratio
		// so we can't even run the test-- consider it "fail"
		done = true;
	      }
	      if (done == true) break; // couldn't find data to test


	      testVal = (((*it).second.filterVec_[j])->testValue_);
	    
	      switch ((((*it).second.filterVec_[j])->filterType_)) {

	      case Filter::min:
		if (nodeVal < testVal) {
		  done = true;
		  break;
		}
		break;

	      case Filter::max:
		if (nodeVal > testVal) {
		  done = true;
		  break;
		}
		break;

	      case Filter::equals: 
		if (nodeVal != testVal) {
		  done = true;
		  break;
		}
		break;

	      default:
		// error
		cout << "testing error" << endl;
		throw runtime_error("filtering error");
		break;
	      }
	    }
	    if (done == true) break;
	  }
	  if (done == true) break;
	  // end filterVec tests


	  // if we made it here, we passed *all* the tests
	
	  // yes, ran the gauntlet!
	  filterPass = true;
	  done = true;
	}


	if (filterPass) {



	  /**
	     peptide and protein tallying
	 
	     keep protein and peptide info for bookkeeping.
	  */
	  int protCol = foundColumns["Gprotein"];
	  proteinList_.push_back(lineData[protCol]);
	  int numAltProt;
	  toInt(lineData[protCol + 1], numAltProt);
	  if (numAltProt == 0) {
	    ++numSingleHitProteins_;
	  }


	  int pepCol = foundColumns["Gpeptide"];  
	  string peptideSequence = lineData[pepCol];
	  // save as stripped 
	  strippedPeptideList_.push_back(peptideSequence);

	  int modPepCol = foundColumns["GpeptideModText"];
	  string modPeptideString = lineData[modPepCol];
	  peptideList_.push_back(modPeptideString);
	  /** 
	      end pep, prot bookkeeping stuff
	  */

      
	  // store in the filter list

	  int startIndx = foundColumns["startOffset"];
	  int endIndx = foundColumns["endOffset"];
	  int mrsIndx = foundColumns["MRS"];
	  int sortDataIndex = foundColumns[options_->sortField.getFieldParamName()];

	  unsigned long long startIndex, endIndex;
	  int mrsIndex;
	  toULLong(lineData[startIndx], startIndex);
	  toULLong(lineData[endIndx], endIndex);
	  toInt(lineData[mrsIndx], mrsIndex);
	  //cout << lineData[sortDataIndex] << endl;
	  filteredList.push_back
	    (new IndexSortData(startIndex,
			       endIndex,
			       mrsIndex,
			       lineData[sortDataIndex]));
	  //cout <<filteredList[filteredList.size()-1].data_<< endl;
	}

      }


    }


    IndexDataSorter*  indxSorter = new IndexDataSorter(options_->sortField, options_->sortDir);

    //cout << filteredList.size() << " passed filter." << endl;
    //cout << "sorting, by " << options_->sortField.getFieldParamName() << endl;
    stable_sort(filteredList.begin(),filteredList.end(),
	 //	      filteredList.end(),
		*indxSorter);

    delete indxSorter;

    //cout << "sorted!" << endl;
    // print list
    //   for (int i=0; i<filteredList.size(); i++) {
    //     cout << filteredList[i].startOffset_ << "\t"
    // 	 << filteredList[i].endOffset_ << "\t"
    // 	 << filteredList[i].MRS_ << "\t"
    // 	 << filteredList[i].data_ << endl;
    //   }



    /*********/

    // set vals for the pipeline
    pipeline_->totalNumSQ_ =
      totalNumSQ_;

    pipeline_->acceptedSQ_ =
      (int)filteredList.size();

  
    /* get peptide, protein bookkeeping */
    vector<string>::iterator si;


    sort(peptideList_.begin(),
	 peptideList_.end());
    si = unique(peptideList_.begin(),
		peptideList_.end());
    peptideList_.erase(si, peptideList_.end());
    pipeline_->numUniquePeptides_ = 
      (int)peptideList_.size();


	
    sort(strippedPeptideList_.begin(),
	 strippedPeptideList_.end());
    si = unique(strippedPeptideList_.begin(),
		strippedPeptideList_.end());
    strippedPeptideList_.erase(si, strippedPeptideList_.end());
    pipeline_->numUniqueStrippedPeptides_ = 
      (int)strippedPeptideList_.size();


    sort(proteinList_.begin(),
	 proteinList_.end());
    //cerr << "proteinlist size:" << proteinList_.size();

	
    // go through and count up the number of protein groups, and
    // tally the number of groups with only one member
    int numProteinGroups = 0;
    int numSingleMemberProteinGroups = 0;
    string oldProtein = "-";
    int proteinsPerGroup=0;
    for (unsigned int i = 0; i<proteinList_.size(); i++) {
      if (oldProtein != proteinList_[i]) {
	// start of a new group
	++numProteinGroups;
	if (proteinsPerGroup == 1) {
	  ++numSingleMemberProteinGroups;
	}
	// reset counter
	proteinsPerGroup = 1;
      }
      else {
	// still in the same group
	++proteinsPerGroup;
      }

      oldProtein = proteinList_[i];
    }

    // get the last one
    if (proteinsPerGroup == 1) {
      ++numSingleMemberProteinGroups;
    };


    pipeline_->numSingleHitProteins_ = 
      numSingleMemberProteinGroups;

    pipeline_->numUniqueProteins_ = 
      numProteinGroups;

    /* 
       jmt: alternatively (and incorrectly), this would count the number
       of spectra with *only one matched protein hit.*
    */
    // 	si = unique(nvb.proteinList_.begin(),
    // 			 nvb.proteinList_.end());
    // 	nvb.proteinList_.erase(si, nvb.proteinList_.end());
    // 	pipeline_.numUniqueProteins_ =
    // 	  nvb.proteinList_.size();

    // 	//cerr << ", unique: " << nvb.proteinList_.size() << endl;
    // 	//for (int z=0; z<nvb.proteinList_.size(); z++) 
    // 	//  cerr << "*"<< nvb.proteinList_[z] << "*" << endl;


    /*********/

  }
  catch (const exception &e) {
    cerr << "exception: " << e.what() << endl;
    throw runtime_error(string("unable to read pepxml file: ")  + e.what());
  }
  catch(...) {
    cerr << "unhandled exception (IndexFile::generateFilteredListFromIndex)" << endl;
    throw;
  }
}







/**
   When we don't have an index file, we use a PepXSAXHandler parser to
   parse the entire PepXML file.  During this parsing, this function
   is called when we encounter *any* SQ node.  In this mode of
   action, we want to extract the minimum required data for sorting
   and filtering from this node, and write it directly to the index
   file.
*/
void 
IndexFile::acceptSQ(XMLNodePtr sqNode) {
  
  /**
     format for line of data in index file (tsv)

     startoffset	endoffset	mrs	<min. req. data>
  */

  ++numAcceptedSQ_;
  ++totalNumSQ_;
  
  // based on current min. req. fields, extract data
  // from the XMLNode and store it in the PepXNode.

  foutTmp_ << sqNode->startOffset_;
  foutTmp_ << "\t" << sqNode->endOffset_;
  foutTmp_ << "\t" << pipeline_->getCurrentRunSummaryNum();
  
  //sqNode->print();
  for (unsigned int i=0; i<minimumSortFilterFields_.size(); i++) {
    //foutTmp_ << "field: " << minimumSortFilterFields_[i].fieldName_ << " ";

    string curFieldName = minimumSortFilterFields_[i].fieldName_;

    /*
    // PepXDebug.helpers
      string o=minimumSortFilterFields_[i].getText(Field::value,
      sqNode,
      *pipeline_,
      "",
      "",
      "",
      false, // don't care about highlighting
      options_->libraAbsoluteValues);
      const char* oo = o.c_str();
    */
      
    foutTmp_ << "\t" << minimumSortFilterFields_[i].getText(Field::value,
							    sqNode,
							    *pipeline_,
							    "",
							    "",
							    "",
							    false, // don't care about highlighting
							    options_->libraAbsoluteValues);
    if (curFieldName == "asapratio") {
      // special case: asapratio
      // we need both ratio and mean; Field::plainText returns both, tsv
      foutTmp_ << "\t" << minimumSortFilterFields_[i].getText(Field::plainText,
							      sqNode,
							      *pipeline_,
							      "",
							      "",
							      "",
							      false, // don't care about highlighting
							      options_->libraAbsoluteValues);
    }
    else if (curFieldName == "protein") {
      // special case: protein
      // we need both protein name and num. alt proteins
      foutTmp_ << "\t" << 
	(sqNode->
	 findChild("search_result")->
	 findChild("search_hit", "hit_rank", "1")->
	 findChildren("alternative_protein")->size());
    }

    
  }
  foutTmp_ << endl;



}

















bool
IndexFile::readAndTestHeader(void) {
  try {
    pipeline_->haveIndexFile_ = false;

    ifstream fin(indexFileName_.c_str(), ios::in|ios::binary);
    if (fin.bad() || fin.fail()) {
      //throw runtime_error("can't open index file " + indexFileName);
      return false;
    }

    pipeline_->reset();

    string inputline;

    safeReadLine(fin, inputline);
    if (inputline != INDEX_FILE_FORMAT_VERSION_STR) {
      return false;
    }

    safeReadLine(fin, pipeline_->xmlFileName_);


    /* check file stats against xmlFileName_ */
    // get the current pepxml file's stats:

    // get the filelength and modification time
    time_t modTime;
    long filelength;
    {
      struct stat filestats;
      int status;
      status = stat(pipeline_->xmlFileName_.c_str(), &filestats);
      modTime = filestats.st_mtime;
      filelength = filestats.st_size;
    }

    long oldFileLength;
    long oldModTime;

    safeReadLine(fin, inputline);
    toLong(inputline, oldFileLength);
    if (filelength != oldFileLength) { // different file
      return false;
    }

    safeReadLine(fin, inputline);
    toLong(inputline, oldModTime);
    if (modTime != oldModTime) { // different file
      return false;
    }




    safeReadLine(fin, inputline);
    toInt(inputline, pipeline_->totalNumSQ_);

    safeReadLine(fin, inputline);
    toInt(inputline, pipeline_->acceptedSQ_);

    safeReadLine(fin, inputline);
    toInt(inputline, pipeline_->numUniquePeptides_);

    safeReadLine(fin, inputline);
    toInt(inputline, pipeline_->numUniqueStrippedPeptides_);

    safeReadLine(fin, inputline);
    toInt(inputline, pipeline_->numUniqueProteins_);

    safeReadLine(fin, inputline);
    toInt(inputline, pipeline_->numSingleHitProteins_);



    // make an Options object representing the reconstituted options
    // from the file
    Options oldOptions;

    safeReadLine(fin, inputline);
    while (inputline == "") {
      safeReadLine(fin, inputline);
    }
    stringstream archiveStream(stringstream::in | stringstream::out);
    archiveStream << inputline.c_str();
    // read up to the "***" marker, denoting the end of the boost::archive text
    safeReadLine(fin, inputline); 
    while ( inputline != "***") {
      //archiveStream.write(inputline.c_str(), inputline.length());
      archiveStream << inputline.c_str();
      safeReadLine(fin, inputline); 
    }
    archiveStream.flush();

    //boost::archive::text_iarchive ia(archiveStream);

    //ia >> oldOptions;

    oldOptions.read(archiveStream);

    //oldOptions.print();
    //options.print();
 

    //cout << "same? " << (options.filterEquals(oldOptions)) << endl;
    //exit(0);

    if (! (options_->filterEquals(oldOptions))) {
      /*
	cout << "from file: ";
	oldOptions.print();
	cout << "from query: ";
	options.print();
      */

      options_->page = 1;
      return false;
    }



    // skip whitespace and "***" marker
    while (
	   (inputline == "") ||
	   (inputline == "***") ) {
      safeReadLine(fin, inputline); 
    }



    // read the MRS section

    try {
      // safeReadLine(fin, inputline); // NO-- not after change
      //cout << "line: " << inputline << endl;
      int numMRS;
      toInt(inputline, numMRS);
      debug(debugout << "looking for " << numMRS << " MRS offsets" << endl);
      
      for (int i=0; i< numMRS; i++) {
	safeReadLine(fin, inputline);
	int mrsOffset;
	toInt(inputline, mrsOffset);
	pipeline_->mrsOffsets_.push_back(mrsOffset);
	debug(debugout << "read line: " << inputline << endl);
	int curVal;
	toInt(inputline, curVal);
	debug(debugout << "int val: " << curVal << endl);
      }
      debug(debugout << "# mrs to parse:" << pipeline_->mrsOffsets_.size() << endl << endl);
    }
    catch (const exception& e) {
      cerr << "error reading MRS offsets from index" << endl;
      throw(e);
    }
    catch(...) {
      cerr << "unhandled exception (IndexFile::readAndTestHeader)" << endl;
      throw;
    }




    // go a little farther, and test the existing headers.
    string inputLine;
    // read to the data section marker
    while (!fin.eof() && (inputLine != "<<<")) {
      getline(fin, inputLine);
    }
    // assert input line is "<<<"

    // the next line is the header line;

    // the first three fields are soffset, eoffset, and mrs


    // each string maps to it's col position, starting with 0
    map<string, int> foundColumns;
    // init this map based on what we'll expect; note that we only care
    // about our current min. reqs, which might be less than what's
    // actually there.
    foundColumns["startOffset"] = -1;
    foundColumns["endOffset"] = -1;
    foundColumns["MRS"] = -1;
    for (unsigned int i=0; i<minimumSortFilterFields_.size();i++){
      foundColumns[minimumSortFilterFields_[i].getFieldParamName()] = -1;
    }

    getline(fin, inputLine);
    {
      istringstream ss(inputLine);
      string colName;
      int colPos = 0;
      while (!ss.eof()) {
	ss >> colName;
	//cout << "got " << colName << endl;
	foundColumns[colName] = colPos;
	++colPos;
      }
    }
    // check to make sure that everything we expected was there.
    for (unsigned int i=0; i<minimumSortFilterFields_.size();i++){
      if (foundColumns[minimumSortFilterFields_[i].getFieldParamName()] < 0) {
	return false;
      }
    }


    fin.close();


    pipeline_->haveIndexFile_ = true;
    return true;
  }
  catch (const exception& e) {
    if(0)cerr << "error reading index file: " << e.what() << endl;
    //throw runtime_error(string("error reading index file: ") + e.what());
    pipeline_->haveIndexFile_ = false;
  }
  catch (...) {
    cerr << "error reading index file (unhandled exception)" << endl;
    pipeline_->haveIndexFile_ = false;
  }

  // if we somehow got here (probably now impossible with catch(...) above), we didn't finish the index file parsing sucessfully.
  return false;
}
