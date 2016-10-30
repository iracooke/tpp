/*
Program       : LibraProteinRatioParser                                                   
Author        : J.Eng and Andrew Keller <akeller@systemsbiology.org>, Robert Hubley, and 
                open source code                                                       
Date          : 11.27.02 

Primary data object holding all mixture distributions for each precursor ion charge

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

#include "LibraProteinRatioParser.h"
#include "common/TPPVersion.h" // contains version number, name, revision
#include "Parsers/Parser/TagListComparator.h" // for REGRESSION_TEST_CMDLINE_ARG defn


int main(int argc, char** argv) {
  hooks_tpp handler(argc,argv); // set up install paths etc

    char* conditionFileName;

    char* xmlfile;

    conditionFileName = NULL;

    xmlfile = NULL;

    int fnameArg = 1;
    char *testArg = NULL;

    if (argc < 3) {
     cout << "LibraProteinRatioParser (" << szTPPVersionInfo << ")" << endl;
    }

    for ( int argNum = 1 ; argNum < argc ;  argNum++ )
    {
       if (!strncmp(argv[argNum],REGRESSION_TEST_CMDLINE_ARG,strlen(REGRESSION_TEST_CMDLINE_ARG))) {
          // learn or run a test
          testArg = argv[argNum];
          if (1==argNum) {
             fnameArg++; // test arg got in before filename arg
          }
       } else {      
          if (argNum == fnameArg) {
             xmlfile = argv[argNum];
          } else if( !strncmp( argv[argNum] , "-c", 2 ) )
          {
             conditionFileName = argv[argNum] + 2;
          }
       }
    }


    if ( conditionFileName == NULL )
    {
   
        cout << "ERROR: xinteract options for Libra are <interact-prot.xml> -c<condition-file.xml>" << endl;

        exit(1);

    }
    if ( xmlfile == NULL )
    {

        cout << "ERROR: xinteract LibraProteinParserMain not finding "
        << "argument for interact-prot.xml file name" << endl;

        exit(1);
    }


   LibraProteinRatioParser* libraProteinRatioParser =
        new LibraProteinRatioParser(xmlfile, conditionFileName, testArg);

    if( libraProteinRatioParser != NULL)
        delete libraProteinRatioParser;
    else
    {
        cout << "error: no protein libra information computed" << endl;

        exit(1);

    }

    // gradual replacement of command line argument with condition xml params
    // so have a condition file reader in prot package now too.

/*
  LibraProteinRatioParser* parser = new LibraProteinRatioParser(argv[1], atoi(argv[2]));
  if(parser != NULL)
    delete parser;
  else {
    cout << "error: no protein libra information computed" << endl;
    exit(1);
  }
*/

    return 0;

}
