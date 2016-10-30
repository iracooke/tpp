
#include <iostream>
#include <fstream>

#include "Validation/Distribution/GammaDistribution.h"
#include "Validation/Distribution/GaussianDistribution.h"
#include "Validation/Distribution/DiscreteDistribution.h"
#include "Validation/Distribution/Distribution.h"
#include "Validation/MixtureDistribution/MixtureDistr.h"
#include "Validation/DiscriminateFunction/DiscrimValMixtureDistr.h"
#include "Validation/MixtureDistribution/DiscreteMixtureDistr.h"
#include "Validation/MixtureDistribution/NTTMixtureDistr.h"
#include "Validation/MixtureDistribution/ICATMixtureDistr.h"
#include "Validation/MixtureModel/MixtureModel.h"


/*

Program       : main.cxx for PeptideProphet                                                       
Author        : Andrew Keller <akeller@systemsbiology.org> 
Date          : 11.27.02 
                                                                       
C++ version of PeptideProphet                                


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

#include "common/TPPVersion.h" // contains version number, name, revision

int main(int argc, char** argv) {

  int MAX_NUM_ITERS = 500;

  Boolean icat = False;
  Boolean glyc = False;
  Boolean force = False;
  Boolean massd = False;
  //Boolean mascot = False;
  Boolean qtof = False;

  char* datafile = argv[1];
  char* outfile = argv[2];
  char* modelfile = argv[3];
  char interact_command[10000];

  char search_engine[100];
  strcpy(search_engine, "SEQUEST"); // default


  //for(int k = 0; k < argc; k++)
  //  cerr << k << ": " << argv[k] << endl;

  if(argc < 4) {
    cerr << argv[0] << "(" << szTPPVersionInfo << ")" << endl;
    cerr << "usage: PeptideProphet <datafile><outputfile><modelfile>(ICAT)(GLYC)"<< endl << endl;
    exit(1);
  }

  if(argc > 4) {
    for(int k = 4; k < argc; k++) {
      if(! icat && strcmp(argv[k], "ICAT") == 0) {
	icat = True;
      }
      else if(! glyc && strcmp(argv[k], "GLYC") == 0) {
	glyc = True;
      }
      else if(! force && strcmp(argv[k], "FORCE") == 0) {
	force = True;
      }
      else if(! massd && strcmp(argv[k], "MASSD") == 0) {
	massd = True;
      }
      else if(! strcasecmp(argv[k], "MASCOT")) {
	strcpy(search_engine, argv[k]);
	//mascot = True;
      }
      else if(! qtof && strcmp(argv[k], "QTOF") == 0) {
	qtof = True;
      }
      else if(k == argc-1) {
	strcpy(interact_command, argv[k]);
      }
    } // next arg
  }

  //cerr << "command: " << interact_command << endl; exit(1);

  MixtureModel* mixmodel = new MixtureModel(datafile, MAX_NUM_ITERS, icat, glyc, massd, search_engine, force, qtof);

  mixmodel->writeModelfile(modelfile, interact_command);

  //mixmodel->writeResults(outfile);
  mixmodel->writeResultsInOrder(outfile);

  return 0;
}
