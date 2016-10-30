/*

Program       : Respect                                                       
Author        : David Shteynberg <dshteynb  AT systemsbiology.org>                                                       
Date          : 03.18.2011

Primary data object holding all mixture distributions for each precursor ion charge

Copyright (C) 2011 David Shteynberg

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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

David Shteynberg
Institute for Systems Biology
401 Terry Avenue North 
Seattle, WA  98109  USA

*/

#include "RespectParser.h"
#include "common/util.h"
#include "common/TPPVersion.h"
#include "Parsers/Parser/TagListComparator.h" // for REGRESSION_TEST_CMDLINE_ARG defn

#ifdef WINDOWS_NATIVE
#include "common/wglob.h"		//glob for windows
#else
#include <glob.h>		//glob for real
#endif

int main(int argc, char** argv) {
  hooks_tpp handler(argc,argv); // set up install paths etc
  
  //TODO: Add input error handling
  if(argc < 2) {
    //    cerr <<  argv[0] << " (" << szTPPVersionInfo << ")" << endl;
    cerr << "USAGE: " << argv[0] << " <OPTIONS> <input_file"<<get_pepxml_dot_ext()<<">"<< endl 
	 << "\nOPTIONS:\n"
      //  << "\tMZML:\t Generate mzML files using msconvert (default only mzXML).\n"
         << "\tTHREADS=<number>:\t Use specified number of threads (default=8).\n"
	 << "\tMINPROB=<number>:\t Minimum probability of spectra to respect (default=0.5).\n"
         << "\tMZTOL=<number>:\t\t Use specified +/- mz tolerance on ions (default=0.1 dalton).\n"
	 << "\tISOTOL=<number>:\t Use specified +/- mz tolerance on isotopic offset ions (default=0.1 dalton).\n"
      //	 << "\tTMPDIR=<string>:\t Set intermediate file output directory (default=./reSpect).\n"
         << "\tKEEPCHG:\t\t Keep the charge of the precursor in the mzML file (default will remove charge).\n"
         << "\tMZML:\t\t\t Output mzML (default is to output same format as input).\n"
         << "\nVERSION: " << szTPPVersionInfo 
	 << endl ;
    exit(1);
  }

  // regression test stuff- bpratt Insilicos LLC
  int testarg = 0;
  eTagListFilePurpose testType=NO_TEST;
  char *testArgArg=NULL;
  char *testFileName=NULL;
  string* arg = NULL;
  string optString = "";
  double minProb =0.5;
  double rtMinProb = 0.9;
  double tol = 0.1;
  double isotol = 0.1;
  int threads = 8;
  bool em = false;
  bool update = true;
  bool mpx = true;

  bool mzML = false;
  bool keepChg = false;

  int minChg = 0;
  int maxChg = 0;

  if (!strncmp(argv[1],REGRESSION_TEST_CMDLINE_ARG,strlen(REGRESSION_TEST_CMDLINE_ARG))) {
     checkRegressionTestArgs(testArgArg = strdup(argv[testarg=1]),testType);
     if (testType!=NO_TEST) {  
        testFileName = constructTagListFilename(argv[1+testarg], // input data file
           testArgArg, // args to the program
           "RespectParser",  // program name
           testType); // user info output
    }
  }

  for (int argidx = testarg+1; argidx < argc-1; argidx++) {
    string arg = argv[argidx];
    
    if (!strncmp(argv[argidx], "MZTOL=", 6)) {
      arg = string(argv[argidx] + 6);
      tol = atof(arg.c_str());
    }

    if (!strncmp(argv[argidx], "MINPROB=", 8)) {
      arg = string(argv[argidx] + 8);
      minProb = atof(arg.c_str());
    }
   
    if (!strncmp(argv[argidx], "ISOTOL=", 7)) {
      arg = string(argv[argidx] + 7);
      isotol = atof(arg.c_str());
    }
   
    if (!strncmp(argv[argidx], "THREADS=", 8)) {
      arg = string(argv[argidx] + 8);
      threads = atoi(arg.c_str());
    }

    if (!strncmp(argv[argidx], "MINCHG=", 7)) {
      arg = string(argv[argidx] + 7);
      minChg = atoi(arg.c_str());
    }

    if (!strncmp(argv[argidx], "MAXCHG=", 7)) {
      arg = string(argv[argidx] + 7);
      maxChg = atoi(arg.c_str());
    }

    if (!strncmp(argv[argidx], "KEEPCHG", 7)) {
      keepChg = true;
    }

    if (!strncmp(argv[argidx], "MZML", 4)) {
      mzML = true;
    }


    optString += string(argv[argidx]) + ' ' ;
  }

  optString += string(argv[argc-1]);

  glob_t g;
  
  

  RespectParser* p = new RespectParser(tol, isotol, threads, minProb, minChg, maxChg);
  
  //p->setOutDir(outDir);

  string in_file = "";
  string out_file = "";
  for (int argidx = testarg+1; argidx<argc; argidx++) {
    if (strncmp(argv[argidx], "MINCHG=", 7) && strncmp(argv[argidx], "MAXCHG=", 7) &&
	strncmp(argv[argidx], "MZML", 4) && strncmp(argv[argidx], "MINPROB=", 8) && 
	strncmp(argv[argidx], "THREADS=", 8) && strncmp(argv[argidx], "ISOTOL=",7) && 
	strncmp(argv[argidx], "MZTOL=",6) ) { // && strncmp(argv[argidx], "TMPDIR=",7) ) {

      if (in_file.empty()) {
	in_file = argv[argidx];
      }
      else {
	out_file = argv[argidx];
      }

    }
    
  }

 
  p->run(in_file.c_str(), optString.c_str());


  cerr << "INFO: done." << endl;
}
