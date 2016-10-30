#ifndef INTERACT_PEP_PARSER_H
#define INTERACT_PEP_PARSER_H

/*

Program       : Interact                                                    
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

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <ostream>
#include <sstream>
#include "Parsers/Parser/Parser.h"
#include "Parsers/Parser/TagFilter.h"
#include "Quantitation/Option.h"
#include "common/Array.h"
#include "common/hooks_tpp.h"
#include "common/ModificationInfo/ModificationInfo.h"
#include "common/constants.h"
#include "Parsers/Parser/OuterTag.h"
#include "Enzyme/ProteolyticEnzyme/ProteolyticEnzyme.h"
#include "Enzyme/ProteolyticEnzyme/ProteolyticEnzymeFactory/ProteolyticEnzymeFactory.h"
#include <pwiz/data/msdata/MSDataFile.hpp>
#include <pwiz/data/msdata/SpectrumInfo.hpp>


class InteractParser : public Parser {

 public:
  InteractParser(Array<char*>* inputfiles, char* outfile, char* database, char* datapath,
		 char* dbtype, char* enz, bool prot_name=false, bool update_chg=false,
		 char* exp_lbl=NULL, int min_peplen=7, int max_rank=1, bool collision_eng=false,  bool fix_pyro_mods=false, bool comp_volt=false, bool prec_intens=false, bool get_rt=false, bool write_ref=true, bool check_pepproph=true);

  ~InteractParser();
  void setFilter(Tag* tag);

 protected:

  void parse(const char* xmlfile);
  void verify_and_correct_path(Tag *tag);
  char* options_;

  ModelOptions modelOpts_;
  ScoreOptions scoreOpts_;

  Array<char*>* inputfiles_; 
  char* outfile_;
  int dirsize_; 
  int min_peplen_;
  int max_rank_;
  char* curr_dir_; 
  char* datapath_; 
  char* database_; 
  char* dbtype_;
  char* enz_;
  char* exp_lbl_;
  bool prot_name_;
  bool update_chg_;
  bool collision_eng_;
  bool comp_volt_;
  bool fix_pyro_mods_;
  bool prec_intens_;
  bool get_rt_;
  bool write_ref_;
  bool check_pepproph_;

  ProteolyticEnzyme* enzyme_;
  pwiz::msdata::MSDataFile* msd_;
  pwiz::msdata::SpectrumListPtr sl_;

};











#endif
