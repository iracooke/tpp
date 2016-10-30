/*

Program       : main for parser                                             
Author        : Andrew Keller <akeller@systemsbiology.org> 
Date          : 11.27.02 

Various XML parsing and overwriting programs

Copyright (C) 2003 Andrew Keller

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

Andrew Keller
Institute for Systems Biology
401 Terry Avenue North 
Seattle, WA  98109  USA
akeller@systemsbiology.org

*/

#include <iostream>
#include <fstream>
#include <stdlib.h>

//#include "ASAPRatioParser.h"
//#include "ASAPRatio_prophet.h"
#include "ASAPRatioPvalueParser.h"

#include "common/TPPVersion.h" // contains version number, name, revision
#include "Parsers/Parser/TagListComparator.h" // for REGRESSION_TEST_CMDLINE_ARG defn


int main(int argc, char** argv) {
  hooks_tpp handler(argc,argv); // set up install paths etc


  if(argc < 2) {
    cerr << " " << argv[0] << "(" << szTPPVersionInfo << ")" << endl;
    cout << " usage: ASAPRatioPvalueParser <xmlfile> [<pngfile>]" << endl;;
    cout << endl << endl;
  }
  //ASAPRatioPvalueParser* pval = new ASAPRatioPvalueParser(argv[1]);

  /*
  ASAPCGIDisplayParser* parser1 = new ASAPCGIDisplayParser("interact-xml-prot.xml.tmp", "IPI00023845");
  proDataStrct* protein = parser1->getProDataStrct();
  if(protein != NULL) {
    cout << "files: " << (parser1->getInputFiles())[0] << endl;
    protein->ratio[0] = 5.0; // modify
    protein->ratio[1] = -1.0;
    new ASAPCGIParser("interact-xml-prot.xml.tmp", "IPI00023845", protein);
  }
  return 0;
  */
  //cout << "num args: " << argc <<  " " << argv[2] << endl;
  Parser* parser = NULL;
  char *testMode=NULL;
  int filearg = 1;
  int pngarg = 2;
  int nCtorArgs = argc-1;
  for (int i=1;i<argc;i++) {
    if (!strncmp(argv[i],REGRESSION_TEST_CMDLINE_ARG,strlen(REGRESSION_TEST_CMDLINE_ARG))) {
        testMode = argv[i];
        nCtorArgs--;
        if (i==filearg) {
           filearg++;
           pngarg++;
        }
     }
  }
  if(2==nCtorArgs)
    parser = new ASAPRatioPvalueParser(argv[filearg], argv[pngarg], testMode);
  else
    parser = new ASAPRatioPvalueParser(argv[filearg], testMode);
  delete parser;
   return 0;
}

