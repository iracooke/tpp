#ifndef _INTERPROPHET_H_
#define _INTERPROPHET_H_

//#define PROGRAM_VERSION "InterProphet v3.0 April 1, 2004  ISB"
//#define PROGRAM_AUTHOR "David Shteynberg"
/*

Program       : InterProphet                                                       
Author        : David Shteynberg <dshteynb  AT systemsbiology.org>                                                       
Date          : 12.12.07

Primary data object holding all mixture distributions for each precursor ion charge

Copyright (C) 2007 David Shteynberg

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

#include "Validation/MixtureModel/MixtureModel.h"
#include "SearchHit.h"
#include "common/util.h"
#include "common/sysdepend.h"
#include "KDModel.h"
#include "BoolModel.h"

#include "common/tpp_hashmap.h" // deals with different compilers
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Parsers/Parser/Tag.h"

using namespace std;

struct eqstr {
  bool operator()(const string& s1, const string& s2) const {
    return strcmp(s1.c_str(), s2.c_str()) == 0;
  }
};

#ifndef _MSC_VER
struct hashstr {
  size_t operator()(const string& s) const {
    __gnu_cxx::hash<const char *> h;
    return h(s.c_str());
  }
};
#endif

typedef TPP_STDSTRING_HASHMAP(SearchHit*) hit_hash;
typedef TPP_STDSTRING_HASHMAP(Array<SearchHit*>*) arrhit_hash;
typedef TPP_STDSTRING_HASHMAP(Array<double>*) dblarr_hash;
typedef TPP_STDSTRING_HASHMAP(Array<int>*) intarr_hash;
typedef TPP_STDSTRING_HASHMAP(double) dbl_hash;
typedef TPP_STDSTRING_HASHMAP(string) str_hash;
typedef TPP_STDSTRING_HASHMAP(int) int_hash;

//typedef hash_map<const char*, Array<double>*, hash<const char*>, eqstr> dblarr_hash;
//typedef hash_map<const char*, double, hash<const char*>, eqstr> dbl_hash;
//typedef hash_map<const char*, string, hash<const char*>, eqstr> str_hash;

//TODO implement probability storage for all NTTs typedef hash_map<const char*, double[3], hash<const char*>, eqstr> ntt_dbl_hash;

class InterProphet {
 public:

  InterProphet(bool nss_flag, bool nrs_flag, bool nse_flag, bool sharp_nse, bool nsi_flag, bool nsm_flag, bool nsp_flag, bool use_fpkm, bool use_length, bool use_cat, int max_threads) ;

  //  void getAllModelAdjProbs(dbl_hash* inprobs, dbl_hash* outprobs, dblarr_hash* allntt_inprobs, dblarr_hash* allntt_outprobs);
  bool getAllModelAdjProbs();
  bool getAllModelAdjProbs(bool nss_flag, bool nrs_flag, bool nse_flag,
			   bool nsi_flag, bool nsm_flag, bool nsp_flag, 
			   bool use_fpkm, bool use_length, bool use_cat);

  //void getNSSModelAdjProbs(Array<dbl_hash*>* inprobs, Array<dbl_hash*>* outprobs,Array<dblarr_hash*>* allntt_inprobs, Array<dblarr_hash*>* allntt_outprobs);
  void getNSSModelAdjProbs();

  // void getModelAdjProbs(Array<dbl_hash*>* inprobs, Array<dbl_hash*>* outprobs, Array<dblarr_hash*>* allntt_inprobs, Array<dblarr_hash*>* allntt_outprobs,KDModel* mod, Array<dbl_hash*>* vals);
  
  void insertResult(const int run_idx, string& spectrum, double prob, 
		    Array<double>* allntt_prob, string& pepseq, 
		    string& modpep, string& swathpep, double calcnmass, string& exp_lbl, 
		    string& charge, Array<string*>* prots, Boolean is_decoy, double maxFPKM = 0, double emp_iRT = 1, int swath_window = -1, int alt_swath = -1);
  void addSearch(string*& name);

  void addPeptideCategory(string& pep, string& cat);

  double getAdjProb(string& exp_lbl, string& spectrum, string& swath);
  double getNTTAdjProb(string& exp_lbl, string& spectrum, string& swath, int ntt);
  string getMsRunStr(int runidx, string& exp_lbl, string& spectrum, string& swath);

  void computeModels();


  void computeModelsThreaded();

  void findRunTopCats();

  void computeCatModel();

  bool isInTopCat(SearchHit* hit);

  void computeFPKMModel();
  bool updateFPKMModel(); 

  int getSpectrumCharge(string& spectrum);
  double getNSSCounts();
  void computeNSSModel(); 
  bool updateNSSModel(); 
  void getNSSAdjProbs();
  double getNSSAdjProb(int runidx, string& exp_lbl, string& spectrum, string& swath);
  double getNSSValue(int runidx, string& exp_lbl, string& spectrum, string& swath);

  double getNRSCounts();
  double getNRSCount(SearchHit* hit);
  void computeNRSModel(); 
  bool updateNRSModel(); 
  void getNRSAdjProbs();
  double getNRSAdjProb(int runidx, string& exp_lbl, string& spectrum, string& swath);
  double getNRSValue(int runidx, string& exp_lbl, string& spectrum, string& swath);

  double getNSECounts();  
  double getNSECount(SearchHit* hit);
  void computeNSEModel();
  bool updateNSEModel();  
  void getNSEAdjProbs();
  double getNSEAdjProb(int runidx, string& exp_lbl, string& spectrum, string& swath);
  double getNSEValue(int runidx, string& exp_lbl, string& spectrum, string& swath);


  double getNSICounts();
  double getNSICount(SearchHit* hit);
  void computeNSIModel(); 
  bool updateNSIModel();  
  void getNSIAdjProbs();
  double getNSIAdjProb(int runidx, string& exp_lbl, string& spectrum, string& swath);
  double getNSIValue(int runidx, string& exp_lbl, string& spectrum, string& swath);

  double getNSMCounts();
  double getNSMCount(SearchHit* hit);
  void computeNSMModel(); 
  bool updateNSMModel();  
  void getNSMAdjProbs();
  double getNSMAdjProb(int runidx, string& exp_lbl, string& spectrum, string& swath);
  double getNSMValue(int runidx, string& exp_lbl, string& spectrum, string& swath);

  double getNSPCounts();
  double getNSPCount(SearchHit* hit, int thread);
  void computeNSPModel();

  double getPeptideLengths();
  double getPeptideLength(SearchHit* hit, int thread);

  void computeLengthModel();

  bool updateNSPModel();  
  void getNSPAdjProbs();
  double getNSPAdjProb(int runidx, string& exp_lbl, string& spectrum, string& swath);
  double getNSPValue(int runidx, string& exp_lbl, string& spectrum, string& swath);

  double getFPKMValue(int runidx, string& exp_lbl, string& spectrum, string& swath);

  //  double getPeptideLengths();

  bool getCatValue(int runidx, string& exp_lbl, string& spectrum, string& swath);
  
  void findBestMatches();
  void buildAllModelHitArray();
  void buildAllRunHitArray();

  bool useNSSModel() { return use_nss_; }
  bool useNRSModel() { return use_nrs_; }
  bool useNSEModel() { return use_nse_; }
  bool useNSIModel() { return use_nsi_; }
  bool useNSMModel() { return use_nsm_; }
  bool useNSPModel() { return use_nsp_; }
  bool useFPKMModel() {  return use_fpkm_; }
  bool useLengthModel() {  return use_length_; }
  bool useCatModel() { return use_cat_; }


  void progress(int tic, int step, int &tot);
  int computeBestMatch(string& exp_lbl, string& spectrum, string& swath);
  int getBestMatch(string& exp_lbl, string& spectrum, string& swath);
  void computeTopProbs(); 

  double getEstNumCorrectPSM() { return  max_spec_prob_tot_; }
  double getEstNumCorrectPep() { return  max_pep_prob_tot_; }

  double adjDoubleTripleCharge(double prob_2_adj, double prob_of_partner);

  Array<Tag*>* getRocDataPointTags();

  void reportModels(ostream& out);

  void printAdjProbs();
  // void NSPThread(void* offset);

  ~InterProphet();

  //private:
  size_t num_runs_;

  int num_engines_;


  KDModel* nss_model_; //sibling searches
  KDModel* nrs_model_; //replicate spectra
  KDModel* nse_model_; //sibling experiments
  KDModel* nsi_model_; //sibling ions (same mods)
  KDModel* nsm_model_; //sibling mods
  KDModel* nsp_model_; //sibling peps
  KDModel* len_model_; //sibling peps

  KDModel* fpkm_model_; //proteogenomics
  
  BoolModel * cat_model_;
  
  int_hash* top_i_hash_;
  dbl_hash* top_prob_hash_;
  dbl_hash* top_adjprob_hash_;
  dblarr_hash* top_allntt_prob_hash_;
  dblarr_hash* top_allntt_adjprob_hash_;

  //elements in the hash must be > -1
  double SumDoubleHash(dbl_hash* & hash);
  
  //Array<str_hash*>* msrunstr_hash_;
  //Array<str_hash*>* pepstr_hash_;   


  //Array<dbl_hash*>* pepproph_hash_;   // maps a given spectrum search to a PeptideProph prob
  //Array<dbl_hash*>* calcnmass_hash_;   // stores peptide masses
  //Array<dbl_hash*>* nssadjprob_hash_;
  //Array<dbl_hash*>* nrsadjprob_hash_;
  //Array<dbl_hash*>* nseadjprob_hash_;
  //Array<dbl_hash*>* nsiadjprob_hash_;
  //Array<dbl_hash*>* nsmadjprob_hash_;

  //Array<dbl_hash*>* mdladjprob_hash_;
  //Array<dbl_hash*>* nss_hash_;     // maps a given spectrum to its NSS adjusted prob
  //Array<dbl_hash*>* nrs_hash_;     // maps a given peptide ion to its NRS adjusted prob - Number of Replicate Spectra Matching Same Peptide Ion
  //Array<dbl_hash*>* nse_hash_;     // maps a given peptide ion to its NSE adjusted prob
  //Array<dbl_hash*>* nsi_hash_;     // maps a given peptide+mods to its NSI adjusted prob
  //Array<dbl_hash*>* nsm_hash_;     // maps a given peptide-mods to its NSM adjusted prob


  //All NTT probs
  //Array<dblarr_hash*>* allntt_pepproph_hash_;   
  //Array<dblarr_hash*>* allntt_nssadjprob_hash_;
  //Array<dblarr_hash*>* allntt_nrsadjprob_hash_;
  //Array<dblarr_hash*>* allntt_nseadjprob_hash_;
  //Array<dblarr_hash*>* allntt_nsiadjprob_hash_;
  //Array<dblarr_hash*>* allntt_nsmadjprob_hash_;

  //Array<dblarr_hash*>* allntt_mdladjprob_hash_;
  //Array<dblarr_hash*>* allntt_adjprob_hash_;


  Array<string*>* search_names_;

  //Array<dbl_hash*>* adjprob_hash_;


  Array<double>* allprobs_;

  Array<hit_hash*>* hits_hash_;

  Array<bool_hash*>* decoy_hits_hash_;
  Array<SearchHit*>* hit_arr_;
  Array<bool>* decoy_hit_arr_;



  Array<arrhit_hash*>* byprot_tophit_hashes_;

  Array<Array<hit_hash*>*>* hits_hashes_;

  dbl_hash* maxprob_hash_;
  
  dbl_hash* maxprob_sp_hash_;

  Array<dbl_hash*>* bypep_nsp_hash_;

  int num_cats_;
  
  arrhit_hash* bypep_hit_hash_;
  arrhit_hash* bypep_tophit_hash_;

  arrhit_hash* byprot_tophit_hash_;

  dblarr_hash* catsarr_byrun_hash_;

  int_hash* byrun_topcat_hash_;
  intarr_hash* bypep_cat_hash_;

  int_hash* cat_index_hash_;

  int_hash* exp_hash_;

  double new_prob_tot_;
  double old_prob_tot_;

  double max_pep_prob_tot_;
  
  double max_spec_prob_tot_;

  double nss_cnt_;
  double nrs_cnt_;
  double nse_cnt_;
  double nsi_cnt_;
  double nsm_cnt_;
  double nsp_cnt_;
  double fpkm_cnt_;


  bool use_nss_;
  bool use_nrs_;
  bool use_nse_;
  bool use_nsi_;
  bool use_nsm_;
  bool use_nsp_;


  bool sharp_nse_;  //Use a more discriminating NSE model

  bool use_fpkm_;

  bool use_length_;

  bool use_cat_;

  bool last_iter_;

  bool swath_mode_;

  double iRT_sum_;

  int max_threads_;

};

#endif // _INTERPROPHET_H_
