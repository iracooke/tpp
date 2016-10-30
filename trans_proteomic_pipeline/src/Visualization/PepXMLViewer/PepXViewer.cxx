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

#include <boost/regex.hpp>
#include <boost/filesystem/operations.hpp> // includes boost/filesystem/path.hpp

#include "common/constants.h"

#include "PepXUtility.h"
#include "SQspreadsheetDisplayer.h"
#include "SQhtmlDisplayer.h"
#include "PepXNodeVecSorter.h"

#include "PepXViewer.h"

using namespace std;



PepXViewer::PepXViewer() {
  reset();
}



PepXViewer::~PepXViewer() {
}



void
PepXViewer::reset(void) { 
  mode_ = CGI;
  options_.reset();
  inBlock_ = false;
  evalBlock_ = false;
}



void 
PepXViewer::error(const std::string& msg) {
  if (mode_ == commandLine) {
    cerr << "error: " << msg << endl;
    exit(1);
  }

  else if (mode_ == CGI) {
    cout << "<h1>error:</h1>" << endl;
    cout << "<b>" << msg << "</b>" << endl;
    // no need to return error value to webserver
    exit(0);
  }

}


void
PepXViewer::usage(void) {

  cout << " PepXMLViewer" << endl
       << endl
       << "   extract and display data from a pepxml file." << endl
       << endl
    /**

       usage

       field naming conventions
       if field is a general field (index, etc), prefix with "G"

       if field is a Peptide Prophet field, prefix with "P"
       possible: all

       if field is a search engine field, prefix with "S"
       possible: all


       display:
       -C <column list>	
       comma-separated column list; names follow min description below
   
       -S <sort field> <sort direction>
   


       exclude charges:
       -X <val>	exclude charge <val>, for 1..3 or 'others'
       +2 only: -X 1 -X 3 -X others




       filters:
       -F mSxcorr
       -f m2Sxcorr

   
       min cutoff:
       -m <name> <val>	field <name> must be greater than <val>;



       max cutoff:
       -M <name> <val>	field <name> must be less than <val>;


       value must equal:
       -E <name> <val>	field <name> must equal <val>, discard if false
       ex: -E nmc 1
   

   
     
       boolean switchs:
       -B <name> <val>	set switch <name> to <val>

       recognized switches:
       requireGlyc	require glycosylation motif in peptide (0 or 1)
       includeBinGlycMotif	include B as well as N in the glyc. motif


       peptide requirements:
       -R <name> <val>
   
       recognized names:

       highlightedPeptideText	regex string
       requiredAA		regex string
       requiredPeptideText	regex string
     
     
    */

       << " author: Natalie Tasman, Aebersold Lab, ISB Seattle" << endl
       << endl
       << endl;
}



void
PepXViewer::parseCommandLineOptions(int argc, char** argv) {
  // returns false on error
  if (!options_.parseCommandLineArgs(argc,argv)) {
    usage();
    exit(1);
  }
}



void
PepXViewer::parseCGIOptions(const string& queryString) {
  try {
    // throws exception on error
    options_.parseQueryString(queryString, 
			      true // yes, expect URL encoding
			      );
  }
  catch (const std::exception& e) {
    throw runtime_error(string("error parsing cgi options: ") + e.what());
  }
  catch(...) {
    cerr << "unhandled exception (PepXViewer::parseCGIOptions)" << endl;
    throw;
  }

//   vector<string> cols;
//   options_.generateMinimumSortFilterInfo(cols);
//   for (int i=0; i< cols.size(); i++ ){
//     cout << cols[i] << endl;
//   }

}



void
PepXViewer::run(void) {
  try {
    // we have options, either defaults or modified
    // we know if we're command-line or CGI

    string templateLocation;

    templateLocation = getLocalHTML(); // from constants.h

    if (options_.xmlFileName == "") {

      templateLocation += "PepXMLViewer_noFile.html";
      processHTMLTemplate(templateLocation);
      
      return;
    }
    
    debug(debugout << "parsing file, either index or entire pepxml");
    pepxmlParser_.parsePepXMLEssentials(options_.xmlFileName,
					options_);
    debug(debugout << "initial parse complete");
    //options_.print();

    debug(options_.print(debugout));    
    debug(pepxmlParser_.pipeline_.print(debugout));
    // cout << endl << endl;
    // we now have a good index file, along with good index in memory

    // sort the PepXNode vector according to the current settings in 
    // options_
    
    // find the column number that represents the current sortField from options



    debug(debugout << "preparing parameters");
    pepxmlParser_.pipeline_.prepareParameters(options_); 
    debug(pepxmlParser_.pipeline_.print(debugout));
  


    if (options_.exportSpreadsheet) {
      string spreadsheetFileName = options_.xmlFileName.substr
	(0, options_.xmlFileName.rfind('.')) 
	+
	".xls";      
      debug(debugout << "exporting spreadsheet to " << spreadsheetFileName);
      writeSpreadSheet(spreadsheetFileName);
    }

    string modelsFileName = options_.xmlFileName.substr
      (0, options_.xmlFileName.rfind('.'))
      +
      "-MODELS.html";

    if ( !boost::filesystem::exists( modelsFileName) ) {
      debug(debugout << "models file " << modelsFileName << "not found. generating...");

      std::string cmd(DEFAULT_LOCAL_BIN);
      cmd += "tpp_models.pl ";
      cmd += options_.xmlFileName;

      FILE* r = popen(cmd.c_str(),"r");
      if (!r) {
	debug(debugout << "call to tpp_models failed; carrying on...");
      }
      pclose(r);
    }


    //pepxmlParser_.pipeline_.print();
      
  

    // this will call printHTMLColumnHeaders and printHTMLDataRows
    debug(debugout << "processing html template:");


    templateLocation += "PepXMLViewer.html";
    processHTMLTemplate(templateLocation);

    debug(debugout << "done presenting html template");
  }
  catch (const std::exception & e) {
    string es = e.what();
    throw runtime_error(es); 
  }
  catch(...) {
    cerr << "unhandled exception (PepXViewer::run)" << endl;
    throw;
  }
}





void 
PepXViewer::printHTMLColumnHeaders(void) {

  cout << "<tr>" << endl;
  for (unsigned int i=0; i<options_.columns.size();i++) {
    cout << "<td valign=\"top\" style=\"font-family: monospace; font-variant: small-caps;\">";
    string header;
    if (options_.minimizeTableHeaders) {
      header = options_.columns[i].getAbbreviation();
      
      /*
      // the following code prints vertical text
      for (int c=0; c<header.size();c++) {
	cout << header[c] << "<br />" << endl;
      }
      */
    }
    else {
      header = options_.columns[i].fieldName_;
      // distinguish pepProphet prob from interProphet prob
      if (options_.columns[i].fieldCode_ == Field::interProphet &&
	  header == "iprobability") {
	header = "iP prob";
      }
      if (options_.columns[i].fieldCode_ == Field::peptideProphet &&
	  header == "probability") {
	header = "pepP prob";
      }

      if (header.substr(0,5) == "libra") {
	int channel;
	toInt(header.substr(5), channel);
	// channels count from 1
	channel--;
	string s;
	toString(pepxmlParser_.pipeline_.libraChannelsMZ_[channel], s);
	header = "libra " + s;
      }

      if (header == "xpress") {
	header = "xpress (l:h)";
      }

      if (header == "asapratio") {
	header = "asapratio (l:h)";
      }

      if (header == "asapratio_HL") {
	header = "asapratio (h:l)";
      }


    }

    cout << header;
    cout << "</td>" << endl;
  }
  cout << "</tr>" << endl;
}





void 
PepXViewer::printHTMLDataRows(void) {
  try {
    SQhtmlDisplayer h(pepxmlParser_.pipeline_, options_);

    // fast parse, with html displayer as SQ acceptor
    
    pepxmlParser_.parseSQNodes(options_.xmlFileName,
			       h, // sq acceptor
			       options_.page,
			       options_.perPage);

    

  }
  catch (const std::exception &e) {
    throw runtime_error(string("error with html display: ")  + e.what());
  }
  catch (...) {
    error("unhandled exception (PepXViewer::printHTMLDataRows)");
  }
}




void 
PepXViewer::writeSpreadSheet(const string & spreadsheetFileName) {

  // write header, with pipeline info
  try {

    ofstream fout(spreadsheetFileName.c_str(), ios::out|ios::binary);

    if (fout.bad() || fout.fail()) {
      throw runtime_error(string("can't open spreadsheet output file ") + 
			  spreadsheetFileName.c_str());
    } 

    SQspreadsheetDisplayer s(pepxmlParser_.pipeline_, options_, fout);
    s.writeHeaders();
    // fast parse, with spreadsheet displayer as SQ acceptor
    
    pepxmlParser_.parseSQNodes(options_.xmlFileName,
			       s, // sq acceptor
			       0,
			       0);

    

  }
  
  catch (const std::exception & e) {
    throw runtime_error(string("error with spreadsheet printing: ")  + e.what());
  }
  catch(...) {
    cerr << "unhandled exception (PepXViewer::writeSpreadSheet)" << endl;
    throw;
  }
}




bool 
PepXViewer::processHTMLTemplate(const std::string& templateFileName)  {

  //pepxmlParser_.pipeline_.print();
  
  //readLine(

  string command; 
  string parameter;

//  bool result;
  string conditional;
  string param;

  
  try {


    //ifstream htmlFile("../html/PepXMLViewer_Demo.html", ios::in);
    ifstream htmlFile(templateFileName.c_str(), ios::in|ios::binary);
    if (htmlFile.bad() || htmlFile.fail()) {
      throw runtime_error(string("can't open html template") + templateFileName);
    } 

    while(!htmlFile.eof() ) {

      string inputLine;
      safeReadLine(htmlFile, inputLine);

      //cout << "got line: " << inputLine << endl;

      //const std::string replacement("$2");

      boost::regex reParameter("(.*[^_]*)__(\\w+)__([^_]*.*)");
      boost::regex reBlock(".*___(.+)___.*");

      const std::string fBlock("$1");
      const std::string fParam("$2");

      //const std::string f("---$1---$2---$3---");
    
      if (inBlock_) {
	// inside block

	// first, see if we're at the end
	if (boost::regex_match(inputLine, reBlock) &&
	    boost::regex_replace(inputLine, reBlock, fBlock) == "END") {
	  // in block, reached block end
	  //cout << "### block end" << endl;
	  //cout << "###" << inputLine << endl;      
	  inBlock_ = false;
	  continue;
	}

	// then, see if we're evaling this block or not
	else {
	  // inside block
	  if (!evalBlock_) {
	    // otherwise, we're not evaluating this block, so skip this line
	    //cout << "### skiping block line " << inputLine << endl;
	    continue;
	  }
	  // otherwise, continue on to normal line processing	
	}
      } // end inBlock

	// not inside, but maybe at beginning of block?
      else if (boost::regex_match(inputLine, reBlock)) {
	// beginning of block
	inBlock_ = true;


	conditional = boost::regex_replace(inputLine, reBlock, fBlock);
	//cout << "### block start: " << conditional << endl;
	//cout << "###" << inputLine << endl;      
      
	// eval block condition
      
	//cout << "looking up condition test " << conditional << ": ";
	map<string, bool>::iterator i = pepxmlParser_.pipeline_.conditions_.find(conditional);
	if (i==pepxmlParser_.pipeline_.conditions_.end()) {
	  // not listed: no
	  evalBlock_ = false;
	  //cout << "### conditional test " << conditional << " not found." << endl;
	  continue;
	} else {
	  // and set evalBlock according to the lookup
	  evalBlock_ = (*i).second;
	  //cout << "### conditional test " << conditional << ": " << evalBlock_ << endl;
	  continue;
	}
      


	//evalBlock_ = false;
	//return true;


      } // end block begin
    
	// if we get here, it's normal line evaluation.

    
	// nothing to do with blocks-- normal line evaluation
      if (boost::regex_match(inputLine, reParameter)) {
	// assume only one match per line
	//command = boost::regex_replace(inputLine, reCmd, replacement);
	command = boost::regex_replace(inputLine, reParameter, fParam);
	//cout << "*** found command " << command << " ***" << endl;

	if (command == "tableColumnHeaders") {
	  if (mode_ == PepXViewer::CGI) {
	    printHTMLColumnHeaders();
	  }
	}
	else if (command == "tableDataRows") {
	  if (mode_ == PepXViewer::CGI) {
	    printHTMLDataRows();
	  }
	}
	else {

	  //map<string,string>::iterator i = pepxmlParser_.pipeline_.generalInfo_.find(command);


	  std::string::const_iterator start, end; 
	  start = inputLine.begin(); 
	  end = inputLine.end(); 
	  boost::match_results<std::string::const_iterator> what; 
	  boost::match_flag_type flags = boost::match_default; 
	  regex_search(start, end, what, reParameter, flags);
	  //for (int c=0; c<what.size(); c++) {
	  //  cout << what[c] << endl;
	  //}
	  //cout << "!" << what[1] << "###" << (*i).second << "###" << what[3] << "!" << endl;

	  //if (i != pepxmlParser_.pipeline_.generalInfo_.end()) {
	  string result = pepxmlParser_.pipeline_[command];
	  if (result != "") {
	    //cout << what[1] << (*i).second << what[3] << endl;
	    if (mode_ == PepXViewer::CGI) {
	      cout << what[1] << result << what[3] << endl;
	    }
	  }
	  else {
	    if (mode_ == PepXViewer::CGI) {
	      cout << what[1] << what[3] << endl;
	    }
	    //cout << inputLine << endl;
	  }
	}
      } // end 'if command found'
      else {
	// this was just a regular line
	if (mode_ == PepXViewer::CGI) {
	  cout << inputLine << endl;
	}
      }
    }
    return true;
  }
  catch (const std::exception& e) {
    error(e.what());
    return false;
  }
  catch(...) {
    cerr << "unhandled exception (PepXViewer::processHTMLTemplate)" << endl;
    return false;
  }

  return false;   // probably impossble to get here, but quiet
		  // compiler warnings about return values
}



