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



#include <exception>
#include <algorithm>

#include <boost/filesystem/operations.hpp> // includes boost/filesystem/path.hpp
#include <boost/filesystem/fstream.hpp>    // ditto

#include "pwiz/utility/misc/random_access_compressed_ifstream.hpp"
#include "PepXUtility.h"
#include "PepXNodeVecBuilder.h"
#include "PepXIndexFile.h"

#include "PepXParser.h"


using namespace std;


PepXParser::PepXParser() {
  indexSortData_ = new vector<IndexSortData*>();;
  reset();
}

PepXParser::~PepXParser() {
  for (std::vector<IndexSortData*>::iterator itr = indexSortData_->begin(); 
       itr !=indexSortData_->end(); 
       itr++) {
    delete *itr;
  }
  delete indexSortData_;
}

void
PepXParser::reset(void) {
  // reset parser objects?
  indexSortData_->clear();
}

void 
PepXParser::parseSQNodes(const string& xmlFileName, 
			 SQHandler& sqAcceptor,
			 int curPage,
			 int rowsPerPage) { 
  sqOffsetOnly_.init(xmlFileName,
		     pipeline_,
		     sqAcceptor);

  pwiz::util::random_access_compressed_ifstream xin(xmlFileName.c_str());

  if (xin.bad() || xin.fail()) {
    throw runtime_error(string("can't open pepxml file ") + 
			xmlFileName.c_str());
  } 




  int page = curPage;
  int perPage = rowsPerPage;

  int startRowNum;
  int endRowNum;

  // display all?
  if (rowsPerPage == 0) {
    startRowNum = 0;
    endRowNum = (int)indexSortData_->size();
  } 
  else {
    // calculate which rows to display
    startRowNum = ((page - 1) * perPage);
    endRowNum = startRowNum + perPage;
    if (endRowNum >   (int)indexSortData_->size()) {
      endRowNum = (int)indexSortData_->size();
    }
  }




  sqOffsetOnly_.resetParser();
  string s = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
  sqOffsetOnly_.bufParse(s.c_str(), (int)s.length(), 0);

  s = "<indexedRoot>\n";
  sqOffsetOnly_.bufParse(s.c_str(), (int)s.length(), 0);


  for (int i=startRowNum; i<endRowNum && startRowNum >= 0; i++) {
    //cerr << "fast sq# " << i << endl;
	
    
    pipeline_.setCurrentRunSummary((*indexSortData_)[i]->MRS_);

    int len = ((*indexSortData_)[i]->endOffset_ -
	       (*indexSortData_)[i]->startOffset_);
    //cerr << "len = " << len << endl;
    xin.seekg((*indexSortData_)[i]->startOffset_);
    char* buf;
    buf = new char[len+1];
    xin.read(buf, len+1);
    buf[len] = 0;
    //cerr << "+++" << buf << "+++" << endl;
    //cout << "***" << buf << "***" << endl << endl << endl;
    sqOffsetOnly_.bufParse(buf, len, false);
    delete [] buf;
  }

  s = "</indexedRoot>\n";
  sqOffsetOnly_.bufParse(s.c_str(), (int)s.length(), true);


  //cout << "done fast sq parsing" << endl;
}









void 
PepXParser::parsePepXMLEssentials(const std::string & xmlFileName,
				  Options& options) {
  try {

    // test for exisitence of pepxml file
    if ( !boost::filesystem::exists( xmlFileName) ) {
      throw runtime_error(xmlFileName + " doesn't exist");
    }

    pipeline_.reset();
    pipeline_.xmlFileName_ = xmlFileName;



    //cout << "reading index..." << endl;
    
    // try to read index
    readIndex(options);

    int entireParseAttempts = 0;
    bool not_parsed = true;

    while (not_parsed) {
      if (!pipeline_.haveIndexFile_) {
	// index file wasn't good;
	debug(debugout << "index file not good");

	pipeline_.reset();
	pipeline_.xmlFileName_ = xmlFileName;
      
	try {
	  //PepXNodeVecBuilder nvb(pipeline_, options);
	  indexFile_.reset(pipeline_, options);

	  // extract header info into pipeline_;
	  // when encountering an MRS section:
	  //  extract MRS info into pipeline_,
	  //  then build SQ nodes for that MRS;
	  //  for accepted SQ nodes, give them to the nodeVec builder,
	  //  which we've asked to store them in the pipeline object
	  debug(debugout << "will parse entire pepxml file");
	  headerMRSSQ_.init(xmlFileName,
			    options, 
			    pipeline_,
			    //nvb // sq acceptor: builds nodelist
			    indexFile_ // sq acceptor: writes index file line-by-line
			    );
	  debug(debugout << "writing index column header");
	  indexFile_.writeColumnHeader();
	  debug(debugout << "parsing entire pepxml file");
	  headerMRSSQ_.parse();
	  debug(debugout << "parsing complete");
	  debug(debugout << "writing index header");
	  indexFile_.writeIndexHeader();
	  pipeline_.rewroteIndex_ = true;
	  debug(debugout << "generating filtered list from index");
	  indexFile_.generateFilteredListFromIndex(*indexSortData_);
	  pipeline_.haveIndexFile_ = true;

	  debug(debugout << pipeline_.totalNumSQ_ << "total SQ encountered, ");
	  debug(debugout << pipeline_.acceptedSQ_ << "accepted SQs,");
	  debug(debugout << "nodevec list size: " << pipeline_.pepXNodeVec_.size());
	  not_parsed = false;
	}
	catch (const std::exception& e) {
	  ++entireParseAttempts;
	  if (entireParseAttempts >= 2) {
	    throw runtime_error(string("error parsing pepxml file: ") + e.what());
	  }
	  pipeline_.haveIndexFile_ = false;
	  not_parsed = true;
	  continue;
	}
	catch (...) {
	  cerr << "unhandled exception (PepXParser::parsePepXMLEssentials)" << endl;
	  throw;
	}
      }
      else {
	debug(debugout << "index file good!" << endl);
	try {

	  headerOnly_.init(xmlFileName, 
			   pipeline_);
	  headerOnly_.parse();

	  mrsOffsetOnly_.init(xmlFileName, 
			      pipeline_);
	  mrsOffsetOnly_.offsetParse();
	  not_parsed = false;
	}
	catch (const std::exception& e) {
	  ++entireParseAttempts;
	  if (entireParseAttempts >= 2) {
	    throw runtime_error(string("trouble reparsing from index file: ") + e.what());
	  }
	  pipeline_.haveIndexFile_ = false;
	  not_parsed = true;
	  continue;
	}
	catch (...) {
	  cerr << "unhandled exception (PepXParser::parsePepXMLEssentials)" << endl;
	  throw;
	}
      }
    }

    // at this point, we have a good filtered list in the indexFile_ object,
    // and a good index file on the disk
  }
  catch (const std::exception& e) {
    throw runtime_error(string("unable to read pepxml file: ")  + e.what());
  }
  catch (...) {
    cerr << "unhandled exception (PepXParser::parsePepXMLEssentials)" << endl;
    throw;
  }
}














// user: check haveIndexFile_ after calling
// read the header; if things match, read the rest of the data.
bool
PepXParser::readIndex(Options& options) {
  try {
    indexFile_.reset(pipeline_, options);
    
    // try to read the header
    if (!indexFile_.readAndTestHeader()) {
      // either no index file, or requires rewrite.
      // try to delete, if possible
      unlink(indexFile_.indexFileName_.c_str());
      return false;
    }

    // if we're here, the pipeline object has been set;
    // we're ready to read the bulk of the data.

    // if everything matches, get the index
    indexFile_.generateFilteredListFromIndex(*indexSortData_);
    pipeline_.haveIndexFile_ = true;
    return true;
  }
  catch (const std::exception& e) {
    if(0)cerr << "error reading index file: " << e.what() << endl; // reference e for quiet compile
    //throw runtime_error(string("error reading index file: ") + e.what());
    unlink(indexFile_.indexFileName_.c_str());
    return false;
  }
  catch (...) {
    cerr << "unhandled exception (PepXParser::readIndex)" << endl;
    throw;
  }
}
