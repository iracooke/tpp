#ifndef _RESPECTPARSER_H_
#define _RESPECTPARSER_H_

/*

Program       : reSpect                                                    
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


#include "RespectFilter.h"
#include "common/TPPVersion.h"
#include "Parsers/Parser/Parser.h"
#include "Search/SpectraST/SpectraSTPepXMLLibImporter.hpp"
#include "cramp.hpp"
#include <sstream>
#include <string>
#include <vector>
#include <ostream>
#include <istream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

#include "common/tpp_hashmap.h"

#ifndef _MSC_VER
struct hashstr {
  size_t operator()(const string& s) const {
    __gnu_cxx::hash<const char *> h;
    return h(s.c_str());
  }
};
#endif
typedef TPP_STDSTRING_HASHMAP(int) int_hash;

typedef vector<vector<streampos>*> stream_vec_vec;

typedef TPP_STDSTRING_HASHMAP(RespectFilter* ) filter_hash;

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




class RespectParser : public Parser {

 public:
  RespectParser( double tol = 0.1, double isotol = 0.1, int threads = 8, double minProb = 0.2, bool keepChg = false, bool mzML = false );

  

  ~RespectParser() {};
  
  void parse(const char* c);
  void parseReadFilter(const char* c, int t, int b);

  void parseBaseNames(const char* c);

  void run(const char* c, const char* opts);


  void setEM(bool em); 
  void setFilter(bool mpx); 

  //  void setOutDir(string);

  string getOutFileName(string& in_file);

  std::string pepx_file_;


  int max_threads_; 

 
 private:
  //  Peptide* pep_;
  std::string tmp_file_;

  // std::string out_dir_;

  std::string opts_;
  //  Respect* ptm_proph_;

  TPP_HASHMAP_T<char, double> stat_mods_hash_;


  TPP_HASHMAP_T<char, double> stat_prot_termods_hash_;

  int_hash* baseNames_;

  stream_vec_vec* baseOffsets_; 


  filter_hash* filters_byname_;


  double mzTol_;
  double isoTol_;

  double minProb_;

  unsigned long total_spectra_;
  unsigned long read_spectra_;

  bool keepChg_;
  bool mzML_;

  //  int minChg_;
  //int maxChg_;
};






#endif
