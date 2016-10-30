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




/** @file main.cpp
    @brief converts dta input file(s) to a single mzXML output file
*/





/** JMT: (cleaned up) original notes:

    !!ATTENTION:!!

    This converter is only meant for the conversion of dta files
    created with mascot2dta.  The converter will only work if there is
    maximally one charge state (i.e. one output file) for each scan.
    In the case of mascot2dta, where +1 +2 and +3 output scan files
    are created, just call the converter as:

      dta2mzxml '*.1.dta'
    
    to limit processing to charge +1 input files (if the files follow
    the expected naming convention.)
*/


#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

#ifndef WIN32
#include <glob.h>
//#include "wordexp.h" // for globbing
//#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp
//namespace fs=boost::filesystem;

#else
#include "common/wglob.h"
#include <stdio.h> // for fopen
#endif


#include "Dta2mzXML.h"



using namespace std;





void usage()
{
  cout << endl
       << " dta2mzxml: converts dta files into mzXML format" << endl
       << endl
       << "  Usage: dta2mzxml (-recount -charge -verbose) <dta files to convert>" << endl
       << endl
       << "         To avoid overrun of command-line arguments," << endl
       << "         wildcards may be specified as a single argument, e.g.:" << endl
       << "         dta2mzxml <flags> '*.dta'" << endl
       << endl
       << "         Flags:" << endl
       << "         -recount: will override old dta numbering" << endl
       << "         -charge:  will include the charge value from the" << endl
       << "                   dta files into the mzXML file" << endl
       << "         -lowcharge: (identical to -plustwo)" << endl
       << "                   only process the *lowest* charge file" << endl
       << "         -byname:  use the basename of the dta files to create mzXML files " << endl 
       << "                   with the same basenames" << endl
       << "         -scanCountIsMaxScanNum: in the case that scan numbers are not" << endl
       << "                   contiguious, and -recount is not selected, write the" << endl
       << "                   last scan's number in the mzXML header's scanCount" << endl    
       << "                   field, instead of the default total number of scans." << endl
       << "                   NOTE: This is non-standard and not a recommended" << endl
       << "                   mode of action." << endl    
       << "         -g:       write output in gzip format (.mzxml.gz)" << endl
       << "         -verbose: verbose mode" << endl
       << "         -help: more detailed usage information" << endl
       << endl
       << "  Original author: Patrick Pedrioli" << endl
       << "  Additional work by Brian Pratt"  << endl
       << "  Additional work Natalie Tasman" << endl;
}


void help() {
  cout << endl
       << " dta2mzXML" << endl
       << " ---------" << endl
       << endl
       << "  Converts dta (MS-MS scan) files into mzXML format;" << endl
       << "  If more than one input file is present, all scans" << endl
       << "  are included in the output file, \"change.mzXML\"" << endl
       << endl
       << endl
       << " command-line flags:"
       << endl
       << "  -recount" << endl
       << "         if this flag is present, scan numbers in the mzXML will be" << endl
       << "         renumbered, starting from zero.  Otherwise, the scan number" << endl
       << "         is extracted from the dta filename." << endl
       << endl
       << "  -charge" << endl
       << "         if this flag is present, each scan element in the mzXML" << endl
       << "         output file will use the charge specificed as the " << endl
       << "         precursor charge from the corresponding dta input file." << endl
       << "         Otherwise, no charge information is included in the " << endl
       << "         mzXML output file. " << endl
       << endl
       << "  -lowcharge" << endl
       << "  -plustwo" << endl
       << "         if this flag is present, if dta scan files exist for " << endl
       << "         multiple charges, only process the *lowest* charge file;" << endl
       << "         for a specific scan number." << endl
       << endl
       << "  -g" << endl
       << "         gzip the output file(s)" << endl
       << endl
       << "  -scanCountIsMaxScanNum" << endl
       << "          in the case that scan numbers are not contiguious," << endl
       << "          and -recount is not selected, write the last scan's number in the" << endl
       << "          mzXML header's scanCount field, " << endl    
       << "          instead of the default total number of scans." << endl    
       << endl
       << "          NOTE: This is non-standard and not a recommended mode of action." << endl    
       << endl
       << "  -verbose" << endl
       << "         Output status messages to the console" << endl
       << endl
       << "  -help" << endl
       << "         Display this message." << endl
       << endl;

  cout << "  ATTENTION:" << endl
       << endl 
       << "    This converter is only meant for the conversion of dta files" << endl
       << "    created with mascot2dta.  The converter will only work if there is" << endl
       << "    maximally one charge state (i.e. one output file) for each scan." << endl
       << "    In the case of mascot2dta, where +1 +2 and +3 output scan files" << endl
       << "    are created, just call the converter as:" << endl
       << endl
       << "      dta2mzxml '*.1.dta'" << endl
       << endl
       << "    to limit processing to charge +1 input files " << endl
       << "    (assuming the dta filenames follow the expected naming convention.)" << endl;
  cout << "    --or--" << endl
       << "    simply use the -lowcharge option, described above, if you still want" << endl
       << "    include other charges for a certain scan number, if the +2 charge" << endl
       << "    dta scan file isn't found for that scan." << endl;

  cout  << endl
	<< endl
	<< endl
	<< "  Original author: Patrick Pedrioli" << endl
	<< "  Additional work by Brian Pratt"  << endl
	<< "  Additional work by Natalie Tasman" << endl
	<< endl;

}

typedef TPP_CONSTCHARP_HASHMAP(std::vector<std::string> *) stringvec_map;


int main(int argc, char* argv[]) {

  int n;
  bool byName = false;

  if( argc < 2 ) {
    usage();
    exit(1);
  }

  try {
    

    
    // process command-line switches
    Dta2mzXML mzXmlWriter;
  
    //cout << argc << endl;
    for( n = 1 ; n < argc ; n++ ) {
      //cout << "checking " << argv[n] << endl;
      if (argv[n][0] != '-') {
	//cout << "skip " << argv[n][0] <<endl;
	break;
      }
      else if( !strcmp( argv[n] , "-charge" ) ) {
	mzXmlWriter.setDoCharge();
      }
      else if( !strcmp( argv[n] , "-recount" ) ) {
	mzXmlWriter.setRecount();
      }
      else if( !strcmp( argv[n] , "-lowcharge" ) ) {
	mzXmlWriter.setPlusTwo();
      }
      else if( !strcmp( argv[n] , "-plustwo" ) ) {
	mzXmlWriter.setPlusTwo();
      }
      else if( !strcmp( argv[n] , "-scanCountIsMaxScanNum" ) ) {
	mzXmlWriter.setScanCountIsMaxScanNum();
      }
      else if( !strcmp( argv[n] , "-verbose" ) ) {
	mzXmlWriter.setVerbose();
      }
      else if( !strcmp( argv[n] , "-g" ) ) {
	mzXmlWriter.setDoGzip();
      }
      else if( !strcmp( argv[n] , "-byname" ) ) {
	mzXmlWriter.setByName();
	byName = true;
      }
      else if( !strcmp( argv[n] , "-help" ) ) {
	help();
	return 0;
      }
      else {
	usage();
	throw runtime_error
	  (string("unknown option ") + argv[n]);
      }
    }



    // argv[n] is now the first dta filename parameter
    
    stringvec_map* dtaFileHash = new stringvec_map;

    stringvec_map::iterator file_itr;

      // create a vector of filenames
    vector<string>* dtaFileNameVec;
    string basename;

    for (;n<argc; n++) {
      //cout << "examining filename " << argv[n] << endl;

      /**
	 glob expansion, from man sh

          After word splitting, unless the -f option has been set,
          bash scans each word for the characters *, ?, and [.  If one
          of these characters appears, then the word is regarded as a
          pattern, and replaced with an alphabetically sorted list of
          file names matching the pattern.  If no matching file names
          are found, and the shell option nullglob is disabled, the
          word is left unchanged.

	Here, if the input filename contains ~, {, }, (, ), [, ], or
	$, *, or ?, we'll use wordexp(3) to expand it and disregard it
	if it fails.
      */

      string filename = argv[n];
      char* key;
      if (byName) {
		  string::size_type spos = filename.find_last_of("/");
	basename = filename;
	if (spos != string::npos) {
	  basename = basename.substr(spos+1);
	}
	string::size_type dpos = basename.find_first_of(".");
	basename = basename.substr(0, dpos);
      }
      else {
	basename = "change";
      }
    
      


      if (filename.find_first_of("~{}()[]$*?") == string::npos) {
	// treat the filename as a regular name

	key = new char[basename.length()+1];
	strcpy(key, basename.c_str());
	file_itr = dtaFileHash->find(basename.c_str());
      

	if (file_itr == dtaFileHash->end()) {
	  dtaFileNameVec = new vector<string>;
	  dtaFileHash->insert(pair<const char*, vector<string>*>(key, dtaFileNameVec));
							       //(*dtaFileHash)[basename.c_str()] = dtaFileNameVec;
	}
	else {
	  dtaFileNameVec = file_itr->second;
	}	
      
      
	FILE* fp;
	fp = fopen(filename.c_str(), "r");
	if (fp != NULL) {
		fclose(fp);

		cout << "adding filename " << filename << endl;
	  dtaFileNameVec->push_back(filename);
	}
	else {
	  cout << "  [nonexistant file: \"" << filename << "\" ]" << endl;
	}
	// on to next command-line argument
	continue;
      }
      else {
	// filename contains patterns; try to expand it

		  // on windows: use wglob instead
	glob_t g;

	int expansionStatus = glob(argv[n],0,NULL,&g); // returns 1 on fail
	if (expansionStatus != 0) {
		// bad pattern specification
		cout << "  [bad pattern: \""<< filename << "\" ]" << endl;
	}
	else {
	  // store matches
	  for (int i=0; i<g.gl_pathc; i++) {
	    string filename = g.gl_pathv[i];
	    char* key;
	    if (byName) {
			string::size_type spos = filename.find_last_of("/");
	      basename = filename;
	      if (spos != string::npos) {
		basename = basename.substr(spos+1);
	      }
		  string::size_type dpos = basename.find_first_of(".");
	      basename = basename.substr(0, dpos);
	    }
	    else {
	      basename = "change";
	    }

	    key = new char[basename.length()+1];
	    strcpy(key, basename.c_str());
	    file_itr = dtaFileHash->find(basename.c_str());
	    

	    if (file_itr == dtaFileHash->end()) {
	      dtaFileNameVec = new vector<string>;
	      dtaFileHash->insert(pair<const char*, vector<string>*>(key, dtaFileNameVec));
							       //(*dtaFileHash)[basename.c_str()] = dtaFileNameVec;
	    }
	    else {
	      dtaFileNameVec = file_itr->second;
	    }	
	    
	    
	    FILE* fp;
	    fp = fopen(filename.c_str(), "r");
	    if (fp != NULL) {
	      fclose(fp);
	      
	      cout << "adding filename " << g.gl_pathv[i] << endl;
	      dtaFileNameVec->push_back(filename);
	    }
	    else {
	      cout << "  [nonexistant file: \"" << filename << "\" ]" << endl;
	    }
	  }

	}
	globfree(&g);
      }
    }
      
    
    file_itr = dtaFileHash->begin();
    while (file_itr != dtaFileHash->end()) {
	dtaFileNameVec = file_itr->second;
	basename = file_itr->first;
        file_itr++;
	if (dtaFileNameVec->size() < 1) {
	  cout << "!no dta files to process-- exiting!" << endl;
	  return 0;
	}
	
	
	
	if (!mzXmlWriter.setOutputStream(basename)) {
	  continue;
	}
	
	// process the filename list to remove unnecessary charges, if
	// requested with plustwo flag
	mzXmlWriter.filterPlusTwo(*dtaFileNameVec);
	
	
	
	
	/*
	  for (int i=0; i<dtaFileNameVec.size(); i++)
	  cout <<"fvec: " << dtaFileNameVec[i] << endl;
	  return 0;
	*/
	
	// write the mzXMLheader, including data from each dta file specified
	mzXmlWriter.writeHeader(*dtaFileNameVec);
	
	// convert scans to mzXML format
	cout << "Starting conversion of the scans...\n";
	mzXmlWriter.writeScans(*dtaFileNameVec);
	
	mzXmlWriter.finalizeXml();
	
	cout << "...done\n";
    }
  }


  catch(const exception &e) {
    cerr << "fatal error: " << e.what() << endl
	 << "[exiting with error]" << endl;
    return -1;
  }

  return 0;
}


