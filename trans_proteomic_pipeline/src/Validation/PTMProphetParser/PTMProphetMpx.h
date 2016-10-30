#ifndef _PTMPROPHETMPX_H_
#define _PTMPROPHETMPX_H_
/*

Program       : PTMProphetmpx                                                       
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
#include <gsl/gsl_combination.h>
#include <gsl/gsl_sort_vector.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>
#endif

using namespace std;

typedef TPP_STDSTRING_HASHMAP(string*) strp_hash;


class PTMProphetMpx {

 public:
  PTMProphetMpx(string& pep, int charge, double calc_neut_mass, 
		cRamp* cramp, long scan, 
		vector<string>& modaas, vector<double>& shift, 
		TPP_HASHMAP_T<char, double>* stat_mods, TPP_HASHMAP_T<char, double>* stat_prot_termods, bool is_nterm_pep, bool is_cterm_pep);

  ~PTMProphetMpx();
  
  bool init();

  void insertMod(string& modaas, double shift);
  
  double getModPrior(int type);
  double getNumMods(int type);



  vector<vector<int>*>* getModCombo(vector<gsl_combination*>& compare);

  void computePosnProbs();
  void evaluateModPositions();

  void evaluateModPositions(int type);

  double getNumModSites(int type);

  double getNextMaxProbBelow(int type, double P);
  void combineMods(vector<gsl_combination*>& compare);//, int index);
  vector<vector<int>*>* combinations();

  string getPepProbString(int type) { return pep_prob_str_[type]; }

  double getProbAtPosition(int type, int i);

  bool nextCombo(vector<gsl_combination*>& compare, int type);
  void rewindCombo(vector<gsl_combination*>& compare, int type);
  void combineMods(vector<gsl_combination*>& compare, int depth);

  bool isModAA(int type, char aa) {
    if (modaas_[type].find(aa) != string::npos) return true;
    return false;
  }

  int nTermMod(int type) { return ntermod_[type]; }
  int cTermMod(int type) { return ctermod_[type]; }

  int nTermMod();
  int cTermMod();

  bool hasNTermMod() { return (*pep_str_)[0] == 'n' && (*pep_str_)[1] == '[' ; }; 
  
  bool isNtermPep() { return is_nterm_pep_; }
  bool isCtermPep() { return is_cterm_pep_; }


  void setPrecision(ostringstream & stream , double& value);

  int NAA() { return NAA_; }
  
  Peptide* getPeptide() { return pep_; }
  
  void setTolerance(double tol) {
    tolerance_ = tol;
  }
  
  void generateDecoys(int nDECOY);

  void deleteDecoys();

  void evaluateModPep(Peptide* mpep);

  void processPeakList();
  
 private:
  Peptide* pep_;
  string* pep_str_;
  SpectraSTLibEntry* entry_;
  SpectraSTPeakList* peakList_;
  long scan_;
  int charge_;
  cRamp* cramp_;
  int NAA_;
  double tolerance_;
  bool etd_;

  bool unknown_mod_;
  
  vector<int> ntermod_;
  vector<int> ctermod_;

  int recur_index_; 

  int nDECOY_;


  double TIC_;

  double minInt_;

  double maxInt_;

  vector<string> modaas_;
  vector<double> shift_;

  vector<strp_hash*> label_;

  vector<string> pep_prob_str_;
  
  double calc_neut_mass_;

  string pep_unmod_str_;
  
  vector<TPP_HASHMAP_T<int, double>* > pos_prob_hash_;

  vector<SpectraSTLibEntry*>* decoyEntries_; 
  vector<SpectraSTPeakList*>* decoyPeakLists_;

  vector<vector<double>*> siteprob_;
  vector<vector<double>*> sitesum_;
  vector<vector<double>*> site_MaxEvidence_;
  vector<vector<double>*> site_MinEvidence_;
  vector<vector<double>*> site_decoyMaxEvidence_;
  vector<vector<double>*> site_decoyMinEvidence_;
  vector<vector<double>*> site_decoyEvidence_;

  vector<vector<vector<double>*>*> site_nomodAllEvidence_;
  vector<vector<vector<double>*>*> site_AllEvidence_;

  vector<vector<double>*> site_Pval_;

  vector<vector<double>*> site_ObsModEvidence_;

  vector<vector<double>*> site_ObsUnModEvidence_;

  vector<vector<double>*> site_ExpModEvidence_;
  vector<vector<double>*> site_ExpUnModEvidence_;


  vector<vector<double>*> site_nomodMaxEvidence_;
  vector<vector<double>*> site_nomodMinEvidence_;

  vector<vector<double>*> site_nomodDecoyMaxEvidence_;
  vector<vector<double>*> site_nomodDecoyMinEvidence_;

  vector<vector<int>*> site_N_;

  vector<vector<int>*> nomods_;
  vector<vector<int>*> nomodsite_;
  vector<vector<int>*> modsite_;
  vector<vector<int>*> mods_;
  vector<int> nmods_;

  vector<vector<vector<int>*>*> combs_of_mods_;

  TPP_HASHMAP_T<char, double>* stat_mods_;

  bool is_nterm_pep_;

  bool is_cterm_pep_;

  TPP_HASHMAP_T<char, double>* stat_prot_termods_;
};



#endif
