#ifndef PEP_PROPH_PARSER_H
#define PEP_PROPH_PARSER_H

//#define PROGRAM_VERSION "PeptideProphet v3.0 April 1, 2004  ISB"
//#define PROGRAM_AUTHOR "AKeller"
/*

Program       : PeptideProphet                                                       
Author        : Andrew Keller <akeller@systemsbiology.org>                                                       
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


#include "common/sysdepend.h"
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "Parsers/Parser/Parser.h"
#include "common/Array.h"
#include "Parsers/Parser/TagFilter.h"
#include "Quantitation/Option.h"
#include "Validation/MixtureModel/MixtureModel.h"
#include "Validation/MixtureDistribution/ICATMixtureDistr.h"
#include "Enzyme/ProteolyticEnzyme/ProteolyticEnzyme.h"

class PeptideProphetParser : public Parser {

 public:

  PeptideProphetParser();
  PeptideProphetParser(const char* xmlfile, const char* options, const char *testMode);
  ~PeptideProphetParser();
  void setFilter(Tag* tag);

 protected:

  void parse(const char* xmlfile);
  void displayOptions(const char* eng);


  MixtureModel* model_;
  char* options_;

  Boolean icat_;
  Boolean force_;
  ModelOptions modelOpts_;
  ScoreOptions scoreOpts_;

  char *testMode_; // regression test stuff - bpratt Insilicos LLC, Nov 2005

};











#endif
