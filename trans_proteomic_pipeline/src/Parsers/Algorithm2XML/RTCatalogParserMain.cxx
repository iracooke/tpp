/*

Program       : RTCatalog                                                   
Author        : David Shteynberg <dshteynb  AT systemsbiology.org>                                                       
Date          : 9.29.2010

Primary data object holding all mixture distributions for each precursor ion charge

Copyright (C) 2010 David Shteynberg

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

#include "common/tpp_hashmap.h" 
#include "RTCatalogParser.h"
#include "common/TPPVersion.h"
#include "Parsers/Parser/TagListComparator.h" // for REGRESSION_TEST_CMDLINE_ARG defn

int main(int argc, char** argv) {
  hooks_tpp handler(argc,argv); // set up install paths etc
  
  //TODO: Add input error handling
  if(argc < 3) {
    cerr <<  argv[0] << " (" << szTPPVersionInfo << ")" << endl;
    cerr << "USAGE: " << argv[0] << " <OPTIONS> <file1"<<get_pepxml_dot_ext()<<"> <file2"<<get_pepxml_dot_ext()<<">... <outfile>" << endl 
	 << "\nOPTIONS:\n" 
         << "\tACNGRAD=<file_with_AcN_gradient>\t- file specifying AcN gradient\n" 
         << "\tTRACKCHROMS=<file_of_chroms_to_track>\t- use chromatograms when available\n" 
         << "\tGRADIENT\t- apply gradient correction using theoretical RTs\n"
	 << "\tXICS\t- use peptide IDs to extract Q1 signal for RT\n"
         << "\tTABLES\t- input files are tables, one table file per run\n" 
         << "\tWORKLISTS=<files_of_runs>\t - files specifying the order of runs, separated by commas\n"
      //<< "\tRTMIXTAG=<string_tag>\t - tag specifying RTmix runs in the worklist\n"
         << "\tIGNORERUNS=<file_of_runs_to_ignore>\t- file specifying run names to ignore\n"
	 << "\tTRACKPEPS=<file_of_peps_to_track>\t- file specifying peptides to track across different runs\n"
	 << "\tGRADPEPS=<file_of_peps>\t- file specifying peptides to use for per-run gradient correction\n"
	 << "\tiRTPEPS=<file_of_peps_and_iRTs>\t- file specifying peptides to use for per-run gradient correction\n"
	 << "\tMINGRADPEPS=<min_number_of_peps>\t- minimum number of peptides required for gradient correction\n"
	 << "\tMINPROB=<min_prob>\t- specify minimum probability of results to include in rt catalog (default 0.9)\n"
	 << "\tMINRT=<min_rt_in_secs>\t- specify minimum RT (secs) of results to include in rt catalog (default 0)\n"
	 << "\tMAXRT=<max_rt_in_secs>\t- specify maximum RT (secs) of results to include in rt catalog (default 2700)\n"
         << "\tRTMIXCHROMS=<string_tag>,<file_of_rtpep_chroms>\t- RTMix tag and file defining the Q1,Q3 pairs (can be specified multiple times)\n"
	 << endl ;
    exit(1);
  }

  // regression test stuff- bpratt Insilicos LLC
  int testarg = 0;
  eTagListFilePurpose testType=NO_TEST;
  char *testArgArg=NULL;
  char *testFileName=NULL;
  string* catFile = NULL;
  string* ignoreFile = NULL;
  string* trackFile = NULL;
  string* gradPepsFile = NULL;

  

  string* iRTsFile = NULL;

  string* chromsFile = NULL;

  string* workListFiles = NULL;
  string* rtmixTag = NULL;

  string* acnFile = NULL;
  string* arg = NULL;
  string* rtCatalog = NULL;

  string * tmp = NULL;

  string* mixTag = NULL;
  string* mixQ1Q3 = NULL;

  double minProb =0.9;
  int minRT = 0;
  int maxRT = 2700;

  int minGradPeps = 0;

  bool gradient=false;
  bool tables=false;
  bool chroms=false;

  bool xics=false;

  str_hash* rtmix_chroms = NULL;new str_hash();


  if (!strncmp(argv[1],REGRESSION_TEST_CMDLINE_ARG,strlen(REGRESSION_TEST_CMDLINE_ARG))) {
     checkRegressionTestArgs(testArgArg = strdup(argv[testarg=1]),testType);
     if (testType!=NO_TEST) {  
        testFileName = constructTagListFilename(argv[1+testarg], // input data file
           testArgArg, // args to the program
           "InterProphetParser",  // program name
           testType); // user info output
    }
  }



  //TODO Param passing needs work!!!
  for (int argidx = testarg+1; argidx < argc-1; argidx++) {
    string arg = argv[argidx];

    if (!strncmp(argv[argidx], "RTMIXCHROMS=", 12)) {
      arg = string(argv[argidx] + 12);
      tmp = new string(arg);
      mixTag = new string(tmp->substr(0,tmp->find(',')));
      mixQ1Q3 = new string(tmp->substr(tmp->find(',')+1));
      
      if (!rtmix_chroms) {
	rtmix_chroms = new str_hash();
      }
      
      str_hash::iterator s_it = (*rtmix_chroms).find(*mixTag);

    

      if (s_it == (*rtmix_chroms).end() ) {
	rtmix_chroms->insert(make_pair(*mixTag, *mixQ1Q3));
      }
      delete tmp;
    }

    if (!strncmp(argv[argidx], "MINPROB=", 8)) {
      arg = string(argv[argidx] + 8);
      minProb = atof(arg.c_str());
    }
    if (!strncmp(argv[argidx], "IGNORERUNS=", 11)) {
      arg = string(argv[argidx] + 11);
      ignoreFile = new string(arg);
    }
    
    if (!strncmp(argv[argidx], "WORKLISTS=", 10)) {
      arg = string(argv[argidx] + 10);
      workListFiles = new string(arg);
    }

    if (!strncmp(argv[argidx], "RTMIXTAG=", 9)) {
      arg = string(argv[argidx] + 9);
      rtmixTag = new string(arg);
    }
    
    if (!strncmp(argv[argidx], "TRACKPEPS=", 10)) {
      arg = string(argv[argidx] + 10);
      trackFile = new string(arg);
    }

    if (!strncmp(argv[argidx], "GRADPEPS=", 9)) {
      arg = string(argv[argidx] + 9);
      gradPepsFile = new string(arg);
    }

    if (!strncmp(argv[argidx], "iRTPEPS=", 8)) {
      arg = string(argv[argidx] + 8);
      iRTsFile = new string(arg);
    }
    
    if (!strncmp(argv[argidx], "MINGRADPEPS=", 12)) {
      arg = string(argv[argidx] + 12);
      minGradPeps = atoi(arg.c_str());
    }

    if (!strncmp(argv[argidx], "ACNGRAD=", 8)) {
      arg = string(argv[argidx] + 8);
      acnFile = new string(arg);
    }
    if (!strncmp(argv[argidx], "MINRT=", 6)) {
      arg = string(argv[argidx] + 6);
      minRT = atoi(arg.c_str());
    }
    if (!strncmp(argv[argidx], "MAXRT=", 6)) {
      arg = string(argv[argidx] + 6);
      maxRT = atoi(arg.c_str());
    }
    if (!strncmp(argv[argidx], "GRADIENT", 8)) {
      gradient = true;
    }

    if (!strncmp(argv[argidx], "TRACKCHROMS=", 12)) {
      chroms = true;
      arg = string(argv[argidx] + 12);
      chromsFile = new string(arg);
    }
    if (!strncmp(argv[argidx], "TABLES", 6)) {
      tables = true;
    }
  
    if (!strncmp(argv[argidx], "XICS", 4)) {
      xics = true;
    }

  }
  
  RTCatalogParser* p = 
    new RTCatalogParser(minProb,minRT, maxRT, gradient, 
			tables, xics, ignoreFile, acnFile, 
			gradPepsFile, iRTsFile, workListFiles, minGradPeps);
  
  if (rtmix_chroms) {
    p->setRTMixChromatograms(rtmix_chroms);
  }

  for (int argidx = testarg+1; argidx<argc-1; argidx++) {
    if (strncmp(argv[argidx], "TRACKCHROMS=", 12) && 
	strncmp(argv[argidx], "IGNORERUNS=", 11) && 
	strncmp(argv[argidx], "MINPROB=", 8) && 
	strncmp(argv[argidx], "GRADIENT", 8) &&
	strncmp(argv[argidx], "TABLES", 6) &&
	strncmp(argv[argidx], "XICS", 4) &&
	strncmp(argv[argidx], "MAXRT=", 6) &&
	strncmp(argv[argidx], "MINRT=", 6) &&
	strncmp(argv[argidx], "TRACKPEPS=", 10) &&
	strncmp(argv[argidx], "GRADPEPS=", 9) &&
	strncmp(argv[argidx], "iRTPEPS=", 8) &&
	strncmp(argv[argidx], "RTMIXTAG=", 9) &&
	strncmp(argv[argidx], "WORKLISTS=", 10) &&
	strncmp(argv[argidx], "MINGRADPEPS=", 12) &&
	strncmp(argv[argidx], "RTMIXCHROMS=", 12) &&
	strncmp(argv[argidx], "ACNGRAD=", 8)) {
        
      p->addFile(argv[argidx]);
    
    }
  }
  
  if (rtmix_chroms)
    p->computeRTMixRuns();

  

  if (tables) {
    p->parseTables(NULL);
  }
  else {
    p->parse(NULL);
  }

    
  const char *outfilename;
  outfilename=argv[argc-1];
  
  if (workListFiles!=NULL) {
    p->setUpRTMixRuns();
  }

  if (xics || chromsFile != NULL || trackFile != NULL) {
    p->trackPeptidesAcrossRuns(chromsFile, trackFile);
  }

  if (workListFiles!=NULL) {
    //   p->setUpRTMixRuns();
    if (gradient) {
      p->correctRTsByAdjacentRun();
    }

  }
  
  else if (gradient) {
    p->correctRTsByRun();
  }




  if (trackFile != NULL) {
    if (!gradient) {
       p->calcRTsByRun();
    }
    string* trackFileName = new string(outfilename);
    trackFileName->append("_");
    trackFileName->append(*trackFile);
    p->trackPeptidesReport(trackFileName);
    delete trackFileName;
  }
  if (chromsFile != NULL) {
    if (!gradient) {
       p->calcRTsByRun();
    }
    string* chromsFileName = new string(outfilename);
    chromsFileName->append("_");
    chromsFileName->append(*chromsFile);
    p->chromPeptidesReport(chromsFileName);
    delete chromsFileName;
  }

 
  p->writeRTCatalog(outfilename);



  // regression test?
  if (testType!=NO_TEST) {
     TagListComparator("RTCatalogParser",testType,outfilename,testFileName);
	 free(testArgArg);
     delete[] testFileName;
  }
  delete ignoreFile;
}
