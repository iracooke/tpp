#ifndef _PTMPROPHET_H_
#define _PTMPROPHET_H_
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

#include "common/tpp_hashmap.h"
#include "Search/SpectraST/Peptide.hpp"
#include "Search/SpectraST/SpectraSTPepXMLLibImporter.hpp"
#include "Search/SpectraST/SpectraSTPeakList.hpp"
#include <sstream>

#ifndef __LGPL__
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multifit.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort_vector.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>
#endif

using namespace std;
class PTMProphet {

 public:
  PTMProphet(string& pep, int charge, cRamp* cramp, long scan, string& modaas, double shift);

  ~PTMProphet();
  
  void init();
  
  double getModPrior();
  double getNumMods();

  double getNumModSites();

  double getNextMaxProbBelow(double P);
  string getPepProbString() { return pep_prob_str_; }

  double getProbAtPosition(int i) {
    if (pos_prob_hash_.find(i) != pos_prob_hash_.end()) {
      return  pos_prob_hash_[i];
    }
    else {
      return -1.;
    }
  }

  bool isModAA(char aa) {
    if (modaas_.find(aa) != string::npos) return true;
    return false;
  }

  int NAA() { return NAA_; }
  
  Peptide* getPeptide() { return pep_; }
  
  bool evaluateModSites(double tolerance);

 private:
  Peptide* pep_;
  string* pep_str_;
  SpectraSTLibEntry* entry_;
  long scan_;
  int charge_;
  cRamp* cramp_;
  int NAA_;

  bool etd_;
  string modaas_;
  double shift_;
  string pep_prob_str_;
 
  TPP_HASHMAP_T<int, double> pos_prob_hash_;

  vector<int> nomods_;
  vector<int> nomodsite_;
  vector<int> mods_;
};



#endif
