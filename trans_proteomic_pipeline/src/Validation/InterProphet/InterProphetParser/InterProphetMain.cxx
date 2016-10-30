/*

Program       : InterProphet                                                       
Author        : David Shteynberg <dshteynb  AT systemsbiology.org>                                                       
Date          : 12.12.07

Primary data object holding all mixture distributions for each precursor ion charge

Copyright (C) 2007 David Shteynberg

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
akeller@systemsbiology.org

*/

#include "InterProphetParser.h"


int main(int argc, char** argv) {
  hooks_tpp handler(argc,argv); // set up install paths etc
  
  //TODO: Add input error handling
  if(argc < 3) {
    //    cerr <<  argv[0] << " (" << szTPPVersionInfo << ")" << endl;
    cerr << "USAGE: " << argv[0] << " <OPTIONS> <file1"<<get_pepxml_dot_ext()<<"> <file2"<<get_pepxml_dot_ext()<<">... <outfile>" << endl 
	 << "\nOPTIONS:\n" 
         << "\tTHREADS=<max_threads>\t- specify threads to use (default 1)\n"
	 << "\tDECOY=<decoy_tag>\t- specify the decoy tag\n"
	 << "\tCAT=<category_file>\t- specify file listing peptide categories\n"
	 << "\tMINPROB=<min_prob>\t- specify minimum probability of results to report\n"
	 << "\tLENGTH\t- Use Peptide Length model\n"
	 << "\tNOFPKM\t- Do not use FPKM model\n"
	 << "\tNONSS\t- Do not use NSS model\n"
	 << "\tNONSE\t- Do not use NSE model\n"
	 << "\tNONRS\t- Do not use NRS model\n"
	 << "\tNONSM\t- Do not use NSM model\n"
	 << "\tNONSP\t- Do not use NSP model\n"
      	 << "\tSHARPNSE\t- Use more discriminating model for NSE in SWATH mode (default: Enabled, use NONSE to disable) \n"
	 << "\tNONSI\t- Do not use NSI model\n" << endl ;
    exit(1);
  }

  // regression test stuff- bpratt Insilicos LLC
  int testarg = 0;
  eTagListFilePurpose testType=NO_TEST;
  char *testArgArg=NULL;
  char *testFileName=NULL;
  string* catFile = NULL;
  string* arg = NULL;
  string* rtCatalog = NULL;
  double minProb =-100;
  double rtMinProb = 0.9;
  string* decoyTag = NULL;
  string* optString = NULL; 

  int threads = 1;

  if (!strncmp(argv[1],REGRESSION_TEST_CMDLINE_ARG,strlen(REGRESSION_TEST_CMDLINE_ARG))) {
     checkRegressionTestArgs(testArgArg = strdup(argv[testarg=1]),testType);
     if (testType!=NO_TEST) {  
        testFileName = constructTagListFilename(argv[1+testarg], // input data file
           testArgArg, // args to the program
           "InterProphetParser",  // program name
           testType); // user info output
    }
  }


  bool NSS=true;
  bool NRS=true;
  bool NSE=true;
  bool NSI=true;
  bool NSM=true;
  bool NSP=true;
  bool SHARPNSE=false; //DDS: Thinking of using for Sibling Swath Windows, but better to repurpose NRS
  bool CAT=false;
  bool RT=false;
  bool FPKM=true;
  bool PEPLEN = false;


  //TODO Param passing needs work!!!
  for (int argidx = testarg+1; argidx < argc-1; argidx++) {
    string arg = argv[argidx];

    if (!strncmp(argv[argidx], "CAT=", 4)) {
      CAT = true;
      catFile =  new string(argv[argidx] + 4);
    }
    if (!strncmp(argv[argidx], "DECOY=", 6)) {
      decoyTag =  new string(argv[argidx] + 6);
    }

    if (!strncmp(argv[argidx], "THREADS=", 8)) {
      arg =  string(argv[argidx] + 8);
      threads = atoi(arg.c_str());
    }
    if (!strncmp(argv[argidx], "MINPROB=", 8)) {
      arg = string(argv[argidx] + 8);
      minProb = atof(arg.c_str());
    }
    else if (!strcmp(argv[argidx], "NONSS")) {
      NSS = false;
    }
    else if (!strcmp(argv[argidx], "SHARPNSE")) {
      SHARPNSE = true;
    }
    else if (!strcmp(argv[argidx], "NOFPKM")) {
      FPKM = false;
    }
    else  if (!strcmp(argv[argidx], "NONRS")) {
      NRS = false;
    }
    else if (!strcmp(argv[argidx], "NONSE")) {
      NSE = false;
    }
    else if (!strcmp(argv[argidx], "NONSI")) {
      NSI = false;
    }
    else if (!strcmp(argv[argidx], "NONSM")) {
      NSM = false;
    }
    else if (!strcmp(argv[argidx], "NONSP")) {
      NSP = false;
    }
    else if (!strcmp(argv[argidx], "LENGTH")) {
      PEPLEN = true;
    }
    
  }
  
  InterProphetParser* p = new InterProphetParser(NSS, NRS, NSE, SHARPNSE, NSI, NSM, NSP, FPKM, PEPLEN, catFile, decoyTag, threads);

  for (int argidx = testarg+1; argidx<argc-1; argidx++) {
    if (strcmp(argv[argidx], "NONSS") && 
	strcmp(argv[argidx], "NONRS") && 
	strcmp(argv[argidx], "NONSE") && 
	strcmp(argv[argidx], "NONSW") && 
	strcmp(argv[argidx], "NONSI") && 
	strcmp(argv[argidx], "NONSM") && 
	strcmp(argv[argidx], "NONSP") && 
	strcmp(argv[argidx], "SHARPNSE") && 
	strcmp(argv[argidx], "NOFPKM") && 
	strcmp(argv[argidx], "LENGTH") && 
	strncmp(argv[argidx], "CAT=", 4) &&
	strncmp(argv[argidx], "DECOY=", 6) &&
	strncmp(argv[argidx], "THREADS=", 6) &&
	strncmp(argv[argidx], "MINPROB=", 8)) {
        
      p->addFile(argv[argidx]);
    
    }
    else {
      if (optString == NULL) {
	optString = new string("");
      }
      else {
	*optString += " " ;
      }
      *optString += argv[argidx];
    }
  }
  p->parse_catfile();
  
  const char *outfilename;
  p->setOutFile(outfilename=argv[argc-1]);
  p->run();
  p->writePepXMLFast(minProb, optString);
  
  // regression test?
  if (getIsInteractiveMode() && testType!=NO_TEST) {
     TagListComparator("InterProphetParser",testType,outfilename,testFileName);
	 free(testArgArg);
     delete[] testFileName;
  }
}
