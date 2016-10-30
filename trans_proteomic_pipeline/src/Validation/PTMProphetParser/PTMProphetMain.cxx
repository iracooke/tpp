/*

Program       : PTMProphet                                                       
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

#include "PTMProphetParser.h"
#include "common/util.h"
#include "common/TPPVersion.h"
#include "Parsers/Parser/TagListComparator.h" // for REGRESSION_TEST_CMDLINE_ARG defn


int main(int argc, char** argv) {
  hooks_tpp handler(argc,argv); // set up install paths etc
  
  //TODO: Add input error handling
  if(argc < 2) {
    //    cerr <<  argv[0] << " (" << szTPPVersionInfo << ")" << endl;
    cerr << "USAGE: " << argv[0] << " <OPTIONS> <input_file"<<get_pepxml_dot_ext()<<"> [<output_file>]"<< endl 
	 << "\nOPTIONS:\n"
         << "\tNOUPDATE:\tDon't update modification_info tags in pepXML\n"
      //<< "\tNOEM:\tDon't apply EM iterations\n"
      // << "\tMULTIPLEX:\tEvaluate all possible mod combinations.\n"
         << "\tMZTOL=<number>:\t Use specified +/- mz tolerance on site specific ions (default=0.1 dalton).\n"
         << "\t<amino acids, n, or c>,<mass_shift>\tSpecify modifications (e.g. STY,80)\n"
         << "\nVERSION: " << szTPPVersionInfo 
	 << endl ;
    exit(1);
  }

  // regression test stuff- bpratt Insilicos LLC
  int testarg = 0;
  eTagListFilePurpose testType=NO_TEST;
  char *testArgArg=NULL;
  char *testFileName=NULL;
  string* catFile = NULL;
  string* arg = NULL;
  string modString = "STY,79.9663";
  string optString = "";
  double minProb =-100;
  double rtMinProb = 0.9;
  double tol = 0.1;
  bool em = false;
  bool update = true;
  bool mpx = true;

  if (!strncmp(argv[1],REGRESSION_TEST_CMDLINE_ARG,strlen(REGRESSION_TEST_CMDLINE_ARG))) {
     checkRegressionTestArgs(testArgArg = strdup(argv[testarg=1]),testType);
     if (testType!=NO_TEST) {  
        testFileName = constructTagListFilename(argv[1+testarg], // input data file
           testArgArg, // args to the program
           "PTMProphetParser",  // program name
           testType); // user info output
    }
  }

  for (int argidx = testarg+1; argidx < argc-1; argidx++) {
    string arg = argv[argidx];
    
    if (!strncmp(argv[argidx], "NOEM", 4)) {
      em = false;
    }
    else if (!strcmp(argv[argidx], "NOUPDATE")) {
      update = false;
    }
    else if (!strcmp(argv[argidx], "MULTIPLEX")) {
      mpx = true;
    }
    else if (!strncmp(argv[argidx], "MZTOL=", 6)) {
      arg = string(argv[argidx] + 6);
      tol = atof(arg.c_str());
    }
    else if (strstr(argv[argidx], ",")!=NULL) {
      modString = arg;
    }
    optString += string(argv[argidx]) + ' ' ;
  }

  optString += string(argv[argc-1]);

  PTMProphetParser* p = new PTMProphetParser(modString, tol);
  
  p->setEM(em);
  p->setUpdate(update);
  p->setMpx(mpx);

  string in_file = "";
  string out_file = "";
  for (int argidx = testarg+1; argidx<argc; argidx++) {
    if (strcmp(argv[argidx], "NOEM") && strcmp(argv[argidx], "MULTIPLEX") && strcmp(argv[argidx], "NOUPDATE") &&
	strncmp(argv[argidx], "MZTOL=",6) && strstr(argv[argidx], ",")==NULL) {
      if (in_file.empty()) {
	in_file = argv[argidx];
      }
      else {
	out_file = argv[argidx];
      }
    }
    
  }

  if (!out_file.empty()) {
    p->setOutFile(out_file);
  }
  else {
    out_file = in_file;
  }
  p->run(in_file.c_str(), optString.c_str());
  
 
  const char *outfilename = out_file.c_str();
  // regression test?
  if (testType!=NO_TEST) {
    TagListComparator("PTMProphetParser",testType,outfilename,testFileName);
    free(testArgArg);
    delete[] testFileName;
  }
}
