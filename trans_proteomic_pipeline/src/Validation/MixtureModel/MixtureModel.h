#ifndef MIX_MODEL_H
#define MIX_MODEL_H

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <map>
#include <set>

#include "common/tpp_hashmap.h" 

#include "Validation/MixtureDistribution/NMCMixtureDistr.h"
#include "Validation/MixtureDistribution/NTTMixtureDistr.h"
#include "Validation/MixtureDistribution/GlycMixtureDistr.h"
#include "Validation/MixtureDistribution/PhosphoMixtureDistr.h"
#include "Validation/DiscriminateFunction/DiscrimValMixtureDistr.h"
#include "Validation/DiscriminateFunction/Mascot/MascotDiscrimValMixtureDistr.h"
#include "Validation/MixtureDistribution/ICATMixtureDistr.h"
#include "Validation/MixtureDistribution/MixtureDistr.h"
#include "common/Array.h"
#include "Spectrum.h"
#include "OrderedResult.h"
#include "Validation/MixtureDistribution/MassDifferenceDiscrMixtureDistr.h"
#include "Validation/MixtureDistribution/DecayContinuousMultimixtureDistr.h"
#include "Validation/MixtureDistribution/AccurateMassDiffDiscrMixtureDistr.h"
#include "Validation/PeptideProphet/PeptideProphetOptions/PeptideProphetOptions.h"
#include "Validation/MixtureDistribution/MixtureDistrFactory.h"
#include "Parsers/Parser/Tag.h"
#include "Parsers/Algorithm2XML/SearchResult/SearchResult.h"
#include "Quantitation/Option.h"
#include <boost/algorithm/string/trim.hpp>


#define PEPTIDEPROPHET_VERSION "PeptideProphet Andrew Keller  ISB"
#ifndef PROGRAM_VERSION
#define PROGRAM_VERSION "PeptideProphet "
#endif
#ifndef PROGRAM_AUTHOR
#define PROGRAM_AUTHOR "AKeller@ISB"
#endif
//#define PROGRAM_VERSION "3.0"
//#define PROGRAM_AUTHOR "Andrew Keller"

/*

Program       : MixtureModel for PeptideProphet                                                       
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

int comp_nums(const void* num1, const void* num2);
int comp_specs(const void* num1, const void* num2);
int comp_ords(const void* num1, const void* num2);

using namespace std;
typedef TPP_STDSTRING_HASHMAP(Array<Spectrum*>*) SpectHash;
//typedef map< char*, Array<Spectrum*>*, strcmp> SpectHash;

class MixtureModel {

 public:
  //MixtureModel(char* filename, int maxnumiters, Boolean icat, Boolean glyc, Boolean massd, Boolean mascot, Boolean force, Boolean qtof);
  MixtureModel(char* filename, int maxnumiters, Boolean icat, Boolean glyc, Boolean massd, char* search_engine, Boolean force, Boolean qtof);

  //MixtureModel(char* engine, char* enzyme, char* massspec, ModelOptions modelOptions, ScoreOptions scoreOptions, int max_num_iters);
  MixtureModel(ModelOptions& modelOptions, ScoreOptions& scoreOptions, int max_num_iters);

  virtual ~MixtureModel();

  void process();
  Boolean updateDistr(char* name, int charge);
  Boolean updateFinalDistr(int charge);
  Boolean iterate(int counter, int charge, Boolean force);
  Boolean iterate(int counter);
  double computeProb(int charge, int index);
  Boolean updatePriors(int charge);
  Boolean updateProbs(int charge);
  Boolean updateProbs(int charge, Boolean force);
  double getProb(int charge, int index);
 
  virtual Boolean enterDistribution(int charge, int index, char* tag, char* value);  
  void writeResults(char* filename);
  double getAdjDoublyTriplyProb(double prob_2_adj, double prob_of_partner);
  void printDistr();

  void readData(istream& is);
  void setNegativeDiscrimValDistrs();
  void writeDiscrimValDistr(char* filename);
  void computeEstimatedSensAndError(char* filename);
  void computeAdjDoubleTriplySpectraProbs();
  void computeAdjMultiChargeSpectraProbs();
  Boolean isAdjusted(int index);
  void deriveModel(int maxnumiters);
  Boolean negOnly(int charge);
  void writeDistr(char* filename, char* interact_command);
  void writeModelfile(char* modelfile, char* interact_command);
  int getNegOnlyCharge(int charge);
  double getTotalProb(int charge);
  double getTotalAdjProb(int charge);
  Boolean maldi(char* spec);
  void setMaldi(char* spec);
  Boolean isNumber(char c);
  Boolean getMaldiInfo(char* filename);
  double computeProbWithNTT(int charge, int origntt, double orgiprob, int ntt, int index);
  void setMixtureDistributionNames(const char* discrim, const char* ntt, const char* nmc);
  void validateModel();
  void writeResultsInOrder(char* filename);
  void assessPeptideProperties(char* filename, Boolean icat, Boolean glyc);
  char* getTagValue(const char* data, const char*tag);
  char* getEnzyme(const char* filename);
  void enterData(Array<Tag*>* tags, int run_idx, string* run_name); //DDS: void enterData(Array<Tag*>* tags)
  Array<Tag*>* getOrderedProbTags(const char* prog_name, int index);
  void setResultsInOrder();
  void initResultsInOrder();
  void findRepSpecs();
  double getOrderedProb(int index);
  double getOrderedProb(int run_idx, int index);
  int getOrderedNTT(int run_idx, int index);
  Boolean isOrderedSpectrum(char* spectrum, int charge, int index);
  //  Tag* getOrderedProbScoreTag(int charge, int index);
  Array<Tag*>* getOrderedProbScoreTags(int charge, int index);
  Array<Tag*>* getModelSummaryTags(const char* prog_name, const char* time, const Array<char*>* inputfiles, double minprob, const char* options);
  Array<Tag*>* getRocDataPointTags(int charge);
  Array<Tag*>* getDiscrimValDistrTags();
  int getOrderedDataIndex(int index);
  void setEnzyme(char* enz);
  void calc_pIstats();
  void write_pIstats(ostream& out); 
  void write_RTstats(ostream& out); 
  void write_RTcoeff(ostream& out); 

 protected:
  void common_constructor_init(); // object initialization


  void enterData(SearchResult* result);

 MixtureDistr* getMixtureDistr(char* name, int charge);

 Array<Array<char*>*>* spectra_;  // spectra by precursor ion charge (0:1+, 1:2+, 2:3+, 3:4+, 4:5+)
 Array<Array<double>*>* probs_;    // probs by charge


 // HENRY
 Array<Array<double>*>* libprobs_;
 Boolean multiply_by_spectrast_lib_probs_;
 void multiplyBySpectraSTLibProbs(); 
// END HENRY

 Array<Array<MixtureDistr*>*>* mixtureDistrs_;  // mixture distributions by charge
 Array<Array<int>*>* inds_;
 Array<Array<Array<int>*>*>* run_chrg_inds_; //DDS: indeces ordered by run
 Array<Array<int>*>* run_inds_; //DDS: indeces ordered by run
 Array<Array<Boolean>*>* isdecoy_inds_;
 Array<Array<double>*>* accmass_inds_;

 int num_runs_;
 
 double* priors_; // for each charge
 int* numspectra_;
 int* done_;  // iterations to model termination (used to restart the model)
 int* extradone_; // extra iterations
 int* totitrs_;  // total iterations to model termination (never resets)
 Boolean verbose_; // do you really want to see the iteration count etc?
 Boolean icat_;  // use icat peptide cys information for probs
 Boolean glyc_;  // use peptide N-glycosylation motif for probs
 Boolean phospho_;  // use peptide phospho motif for probs
 Boolean use_adj_probs_;  // constraint for 2+ and 3+ interpretations of same spectrum
 Boolean gamma_;  // use gamma distribution for fval distribution among incorrect search results
 
 Boolean no_neg_init_;
 Boolean no_ntt_;
 Boolean no_nmc_;
 Boolean nonparam_;
 Boolean forcedistr_;
 Boolean use_expect_;

 int num_pairs_;
 int num_reps_;
 Spectrum* pairedspecs_;  
 
 SpectHash* rep_specs_;

 set<int> adj_set_;

 Array<int>* adjusted_;
 char* filename_;
 int max_num_iters_;
 int extraitrs_;
 Boolean* ignore_; 
 Boolean* negOnly_;  // no model results, use alternative means to crudely estimate prob ('0' or '-charge')
 Boolean maldi_;  // maldi data
 Boolean maldi_set_;
 Boolean* pseudonegs_;
 int min_num_specs_;
 char* discrim_name_;
 char* ntt_name_;
 char* nmc_name_;
 char* enzyme_;


 // HENRY: Need to store away the search_engine type so as to set the convergence threshold differently for SpectraST
 char* search_engine_;
 // END HENRY

 // HENRY: Use to keep track of which ICAT type 
 int spectrast_icat_;
 bool spectrast_delta_;
 bool optimize_fval_;
 // END HENRY

 Boolean qtof_;
 Boolean pI_;
 Boolean RT_;
 Boolean alldone_;
 Boolean accMass_;

 float conservative_;
 Boolean ppm_;

 Boolean no_mass_;
 
 int min_pI_ntt_; 
 double min_pI_prob_; 
 int min_RT_ntt_; 
 double min_RT_prob_; 

 Boolean use_decoy_;
 Boolean has_decoy_;
 Boolean output_decoy_probs_;
 char decoy_label_[100];

 MixtureDistrFactory* factory_;
 int index_;
 int counter_;
 int extra_counter_;

 int dec_count_;
 int fwd_count_;

 Array<double>* lastitr_totdiff_;

 MixtureDistr* pI_ptr_;
 MixtureDistr* RT_ptr_;
 MixtureDistr* AM_ptr_;

 Array<MixtureDistr*>* isoMass_ptr_;
 OrderedResult* ordered_;
 int num_ordered_;
 int allchg_index_;
 int maxnumiters_;
 double* prev_log_term_;

 char* rtcat_file_;

 bool dups_linked_;
 
};

#endif
