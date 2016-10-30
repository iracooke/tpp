#ifndef _PTMPROPHETPARSER_H_
#define _PTMPROPHETPARSER_H_

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

#include "PTMProphet.h"
#include "PTMProphetMpx.h"
#include "common/TPPVersion.h"
#include "Parsers/Parser/Parser.h"
#include "Validation/InterProphet/InterProphetParser/KDModel.h"
#include "Search/SpectraST/SpectraSTPepXMLLibImporter.hpp"
#include "cramp.hpp"
#include <sstream>
#include <ostream>


using namespace std;

class ProbPos {
 public:
  ProbPos(double pr, int ps) {
    prob_ = pr;
    pos_ = ps;
  }
 

  ~ProbPos() {}
  double prob_;
  int pos_;
};




class PTMProphetParser : public Parser {

 public:
  PTMProphetParser(string& modstring, double tol);

  

  ~PTMProphetParser() {};
  
  void parse(const char* c);
  void parseRead(const char* c);
  void parseWrite(const char* c);
  void parseWriteUpdate(const char* c);
  void computePTMModel();

  void parseReadMpx(const char* c);
  void parseWriteMpx(const char* c) ;
  void parseWriteUpdateMpx(const char* c);
  void computePTMModelMpx();


  void run(const char* c, const char* opts);


  void setEM(bool em); 
  void setMpx(bool mpx); 

  void setOutFile(string);

  void setUpdate(bool up);

  
  
  void writeUpdatedModTags(PTMProphet* proph, ofstream& fout, string & mod_pep_str);
  void writeUpdatedModTags(PTMProphetMpx* proph, ofstream& fout, string& mod_pep_str);
  
 private:
  //  Peptide* pep_;
  std::string tmp_file_;
  std::string out_file_;

  std::string opts_;
  //  PTMProphet* ptm_proph_;
  KDModel* ptm_model_;
  bool em_;
  bool mpx_;
  bool update_mod_tags_;
  vector<string> aminoacids_;
  vector<double> massshift_;
  vector<double> priors_;

  TPP_HASHMAP_T<char, double> stat_mods_hash_;


  TPP_HASHMAP_T<char, double> stat_prot_termods_hash_;

  double mzTol_;
};






#endif
