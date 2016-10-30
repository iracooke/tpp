#include "MixtureModel.h"
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

#include "common/TPPVersion.h"
#include <math.h>
#include <float.h>

MixtureModel::MixtureModel(char* filename, int max_num_iters, Boolean icat, Boolean glyc, Boolean massd, char* search_engine, Boolean force, Boolean qtof) {
  common_constructor_init();
  rep_specs_ = NULL;
  rtcat_file_ = NULL;
  num_runs_ = 0;
  run_inds_ = new Array<Array<int>*>;
  lastitr_totdiff_ = new Array<double>;
  filename_ = new char[strlen(filename)+1];
  strcpy(filename_, filename);

  // HENRY: Need to store away the search_engine type so as to set the convergence threshold differently for SpectraST
  search_engine_ = new char[strlen(search_engine)+1];
  strcpy(search_engine_, search_engine);
  // END HENRY

  // HENRY: initialize spectrast_icat_ to 0 for now
  spectrast_icat_ = 0;
  // spectrast_delta_ = false;
  optimize_fval_ = false;
  // END HENRY

  filename_[strlen(filename)] = 0;
  max_num_iters_ = max_num_iters;
  extraitrs_ = EXTRAITRS;
  min_pI_ntt_ = 2;
  min_pI_prob_ = 0.9;
  min_RT_ntt_ = 2;
  min_RT_prob_ = 0.9;
  icat_ = icat;
  glyc_ = glyc;
  qtof_ = qtof;
  nonparam_ = False;
  use_expect_ = False;
  priors_ = new double[MAX_CHARGE];
  numspectra_ = new int[MAX_CHARGE];
  done_ = new int[MAX_CHARGE];
  extradone_ = new int[MAX_CHARGE];
  spectra_ = new Array<Array<char*>*>;
  probs_ = new Array<Array<double>*>;
  inds_ = new Array<Array<int>*>;
  mixtureDistrs_ = new Array<Array<MixtureDistr*>*>;
  gamma_ = True;  
  // HENRY: Don't need to adjust +2/+3 probs for SpectraST
  //  if (strcasecmp(search_engine, "SPECTRAST") == 0) {
  //  use_adj_probs_ = False;
  //} 
  //else { 
  //  use_adj_probs_ = True;
  //}
  // END HENRY
  no_neg_init_ = False;
  pairedspecs_ = NULL;
  adjusted_ = NULL;
  negOnly_ = new Boolean[MAX_CHARGE];
  maldi_ = False;
  maldi_set_ = False;
  pseudonegs_ = new Boolean[MAX_CHARGE];
  min_num_specs_ = 35; //50;
  discrim_name_ = NULL;
  ntt_name_ = NULL;
  nmc_name_ = NULL;
  enzyme_ = NULL;
  factory_ = NULL;
  forcedistr_ = False;
  prev_log_term_ = new double[MAX_CHARGE];
  cerr << "\n PeptideProphet v. 1.0 A.Keller 11.7.02 ISB";

  Boolean getmaldi = getMaldiInfo(filename_);
  if(! getmaldi) {
     cerr << " could not get maldi information from " << filename_ << std::endl;
    exit(1);
  }
  if(maldi_)
    cerr << " (maldi data)";
  cerr << endl;

  enzyme_ = getEnzyme(filename_);
  char enz_dig_distr_name[100];
  strcpy(enz_dig_distr_name, "no. tolerable ");
  if(enzyme_ == NULL)
    strcat(enz_dig_distr_name, "tryptic");
  else strcat(enz_dig_distr_name, enzyme_);
  strcat(enz_dig_distr_name, " term. [ntt]");

  char discr_distr_name[100];
  if(search_engine != NULL) 
    strcpy(discr_distr_name, search_engine);
  else {
    cerr << "error: null search engine" << endl;
    exit(1);
  }
  strcat(discr_distr_name, " discrim score [fval]");
  if(! force)
    assessPeptideProperties(filename, True, False); // first Boolean for icat, second for glyc
  
  char nmc_distr_name[100];
  strcpy(nmc_distr_name, "no. missed enz. cleavages [nmc]");
  setMixtureDistributionNames(discr_distr_name, enz_dig_distr_name, nmc_distr_name); // fval and ntt mixture model names

  for(int charge = 0; charge < MAX_CHARGE; charge++) {
    prev_log_term_[charge] = 0;
    numspectra_[charge] = 0;
    done_[charge] = -1;
    totitrs_[charge] = -1;
    extradone_[charge] = -1;
    priors_[charge] = -1.0;
    negOnly_[charge] = False;
    pseudonegs_[charge] = False;
    Array<char*>* nextspec = new Array<char*>;
    spectra_->insertAtEnd(nextspec);
    Array<double>* nextprob = new Array<double>();
    probs_->insertAtEnd(nextprob);
    Array<int>* nextind = new Array<int>;
    inds_->insertAtEnd(nextind);
    lastitr_totdiff_->insertAtEnd(-1);
    Array<MixtureDistr*>* next = new Array<MixtureDistr*>;
    //                    add new distributions to model here
    /////////////////////////////////////////////////////////////////////////////////////////////
    if(search_engine != NULL && ! strcasecmp(search_engine, "MASCOT")) 
      next->insertAtEnd(new MascotDiscrimValMixtureDistr(charge, discrim_name_, "fval", maldi_, qtof_));
    else if(search_engine != NULL && ! strcasecmp(search_engine, "SEQUEST")) 
      next->insertAtEnd(new DiscrimValMixtureDistr(charge, discrim_name_, "fval", gamma_, maldi_, qtof_, nonparam_, use_expect_));
    else {
      cerr << "cannot accept search engine: " << search_engine << endl;
      exit(1);
    }
      
    next->insertAtEnd(new NTTMixtureDistr(charge, ntt_name_, "ntt"));

    if(search_engine == NULL || strcasecmp(search_engine, "MASCOT")) //! mascot)
      next->insertAtEnd(new NMCMixtureDistr(charge, "no. missed enz. cleavages [nmc]", "nmc"));

    if(massd) // now only available for charge 2/3
      //next->insertAtEnd(new MassDifferenceDiscrMixtureDistr(charge, "mass diff", "massd", 5.0, 1.0));
      next->insertAtEnd(new VariableOffsetMassDiffDiscrMixtureDistr(charge, "var offset mass diff [massd]", "massd", 5.0, 1.0, 0.0));
    if(icat_) {
      next->insertAtEnd(new ICATMixtureDistr(charge, "icat cys [icat]", "pep"));
    }
    if(glyc_) {
      next->insertAtEnd(new GlycMixtureDistr(charge, "N glyc motif [glyc]", "pep"));
    }
    if(phospho_) {
      next->insertAtEnd(new PhosphoMixtureDistr(charge, "N phospho motif [phospho]", "pep"));
    }
    /////////////////////////////////////////////////////////////////////////////////////////////
    mixtureDistrs_->insertAtEnd(next);
  }
  
  

  // open stream and read Data, then derive model...
  ifstream fin(filename_);
  if(! fin) {
    cerr << "cannot open " << filename_ << endl;
    exit(1);
  }

  readData(fin);
  fin.close();

  validateModel();

  deriveModel(max_num_iters_);

}


MixtureModel::MixtureModel(ModelOptions& modelOptions, ScoreOptions& scoreOptions, int max_num_iters) {
  common_constructor_init();
  max_num_iters_ = max_num_iters;
  lastitr_totdiff_ = new Array<double>;
  rep_specs_ = NULL;
  // parse through modelOptions for these....
  
  icat_ = modelOptions.icat_ == ICAT_ON; //0; //icat;
  glyc_ = modelOptions.glyc_; //0; //glyc;
  phospho_ = modelOptions.phospho_; //0; //phospho;
  extraitrs_ = modelOptions.extraitrs_;
  min_pI_ntt_ = modelOptions.min_pI_ntt_;
  min_pI_prob_ = modelOptions.min_pI_prob_;
  min_RT_ntt_ = modelOptions.min_RT_ntt_;
  min_RT_prob_ = modelOptions.min_RT_prob_;
  no_neg_init_ = modelOptions.no_neg_init_;
  no_ntt_ = modelOptions.no_ntt_;
  no_nmc_ = modelOptions.no_nmc_;
  maldi_ = modelOptions.maldi_;
  nonparam_ = modelOptions.nonparam_;
  use_expect_ = modelOptions.use_expect_;
  forcedistr_ = modelOptions.forcedistr_;
  use_decoy_ = modelOptions.use_decoy_;
  rtcat_file_ = modelOptions.rtcat_file_;
  has_decoy_ = false;
  output_decoy_probs_ = modelOptions.output_decoy_probs_;
  strcpy(decoy_label_, modelOptions.decoy_label_);
  isdecoy_inds_ = new Array<Array<Boolean>*>;
  accmass_inds_ = new Array<Array<double>*>;
  qtof_ = 0; //qtof;
  priors_ = new double[MAX_CHARGE];
  Boolean force = False;

  numspectra_ = new int[MAX_CHARGE];
  done_ = new int[MAX_CHARGE];
  totitrs_ = new int[MAX_CHARGE];
  extradone_ = new int[MAX_CHARGE];
  spectra_ = new Array<Array<char*>*>;
  probs_ = new Array<Array<double>*>;
  inds_ = new Array<Array<int>*>;
  run_inds_ = new Array<Array<int>*>;
  mixtureDistrs_ = new Array<Array<MixtureDistr*>*>;
  gamma_ = True;
  
  // HENRY: Need to store away the search_engine type so as to set the convergence threshold differently for SpectraST
  search_engine_ = new char[strlen(modelOptions.engine_)+1];
  strcpy(search_engine_, modelOptions.engine_);
  // END HENRY

  // HENRY: For SpectraST, also need to keep track of the library entry probability for each spectrum query
  libprobs_ = new Array<Array<double>*>;
  multiply_by_spectrast_lib_probs_ = modelOptions.multiply_by_spectrast_lib_probs_;
  // END HENRY

  // HENRY: initialize spectrast_icat_ to 0 for now
  spectrast_icat_ = modelOptions.spectrast_icat_;
  // spectrast_delta_ = modelOptions.spectrast_delta_;
  optimize_fval_ = modelOptions.optimize_fval_;
  // END HENRY

  // HENRY: Don't need to adjust +2/+3 probs for SpectraST
  //  if (strcasecmp(search_engine_, "SPECTRAST") == 0) {
  //  use_adj_probs_ = False;
  //} else { 
  //  use_adj_probs_ = True;
  //}
  // END HENRY

  pairedspecs_ = NULL;
  adjusted_ = NULL;
  negOnly_ = new Boolean[MAX_CHARGE];
  ignore_ = new Boolean[MAX_CHARGE];
  //  maldi_ = False;
  maldi_set_ = True;
  pseudonegs_ = new Boolean[MAX_CHARGE];
  min_num_specs_ = 35; //50;
  discrim_name_ = NULL;
  ntt_name_ = NULL;
  nmc_name_ = NULL;
  factory_ = new MixtureDistrFactory(modelOptions, scoreOptions);
  prev_log_term_ = new double[MAX_CHARGE];
  index_ = 0;
  enzyme_ = new char[strlen(modelOptions.enzyme_)+1];
  strcpy(enzyme_, modelOptions.enzyme_);

  char nmc_distr_name[100];
  strcpy(nmc_distr_name, "no. missed enz. cleavages [nmc]");
  setMixtureDistributionNames(factory_->getDiscrMixtureDistrName(), factory_->getNTTMixtureDistrName(), nmc_distr_name);

  MixtureDistr* next_distr = NULL;
  KernelDensityPIMixtureDistr* pI_distr = NULL;
  RTMixtureDistr* RT_distr = NULL;
  AM_ptr_ = NULL;
  conservative_ = modelOptions.conservative_;
  ppm_ = modelOptions.ppm_;

  for(int charge = 0; charge < MAX_CHARGE; charge++) {
    //cout << "charge: " << charge+1 << endl;
    prev_log_term_[charge] = 0;
    numspectra_[charge] = 0;
    done_[charge] = -1;
    totitrs_[charge] = -1;
    extradone_[charge] = -1;
    priors_[charge] = -1.0;
    negOnly_[charge] = ! modelOptions.use_chg_[charge];
    ignore_[charge] = ! modelOptions.use_chg_[charge];
    pseudonegs_[charge] = False;
    Array<char*>* nextspec = new Array<char*>;
    spectra_->insertAtEnd(nextspec);
    Array<double>* nextprob = new Array<double>();
    probs_->insertAtEnd(nextprob);

    // HENRY
    Array<double>* nextlibprob = new Array<double>();
    libprobs_->insertAtEnd(nextlibprob);
    // END HENRY

    Array<int>* nextind = new Array<int>;
    inds_->insertAtEnd(nextind);

    lastitr_totdiff_->insertAtEnd(-1);

    Array<MixtureDistr*>* next = new Array<MixtureDistr*>;
    //                    add new distributions to model here
    
    /////////////////////////////////////////////////////////////////////////////////////////////
    next_distr = factory_->getDiscrimValMixtureDistr(charge,  modelOptions.neg_gamma_, modelOptions.nonparam_, modelOptions.use_expect_);

    next_distr->setConservative(conservative_);
    

    if(next_distr != NULL)
      next->insertAtEnd(next_distr);
    next_distr = factory_->getNTTMixtureDistr(charge);
    if(next_distr != NULL)
      next->insertAtEnd(next_distr);
    next_distr = factory_->getNMCMixtureDistr(charge);
    if(next_distr != NULL)
      next->insertAtEnd(next_distr);

    no_mass_ = modelOptions.no_mass_;
    // HENRY: Precursor m/z distribution
    if (!no_mass_) {      
      if (modelOptions.accMass_) {
	if (AM_ptr_ == NULL) {
	  cout << "adding Accurate Mass mixture distr" << endl;
	  next_distr = factory_->getMassDifferenceDiscrMixtureDistr(charge);    
	  AM_ptr_ = (MassDifferenceDiscrMixtureDistr*)next_distr;
	}
	else {
	  next_distr = AM_ptr_;
	}
      }
      else {
	next_distr = factory_->getMassDifferenceDiscrMixtureDistr(charge);    
      }
      if(next_distr != NULL) 
	next->insertAtEnd(next_distr);
      accMass_ = modelOptions.accMass_;
      if (modelOptions.accMass_) {
	next_distr = factory_->getIsoMassDiffDiscrMixtureDistr(charge);    
	if(next_distr != NULL) 
	  next->insertAtEnd(next_distr);
      }
    } 
    // END HENRY

    // HENRY: modify to determine if we need ICAT for SpectraST
    if (!(strcasecmp(search_engine_, "SPECTRAST") == 0)) {
      if(icat_) {
        next_distr = factory_->getICATMixtureDistr(charge);    
        if(next_distr != NULL)
	  next->insertAtEnd(next_distr);
      }
    } else {
      // spectrast case, checks spectrast_icat_
      if (spectrast_icat_ > 0) {
	next_distr = factory_->getICATMixtureDistr(charge, spectrast_icat_);
	if (next_distr != NULL) {
	  next->insertAtEnd(next_distr);
	}  
      }
      
      use_adj_probs_ = False;
      
      // add delta mixture distribution
      // if (spectrast_delta_) {
      //	cerr << "Adding spectrast delta distribution..." << endl;
      //	next_distr = factory_->getDeltaMixtureDistr(charge);
      //	if (next_distr != NULL) {
      //	  next->insertAtEnd(next_distr);
      //	}
      //    }
      

    }	
    // END HENRY
    
    if(glyc_) {
      next_distr = factory_->getGlycMixtureDistr(charge);    
      if(next_distr != NULL)
	next->insertAtEnd(next_distr);
    }

    if(phospho_) {
      next_distr = factory_->getPhosphoMixtureDistr(charge);    
      if(next_distr != NULL)
	next->insertAtEnd(next_distr);
    }

    if (use_decoy_) {
      Array<Boolean>* next_dec_ind = new Array<Boolean>;
      isdecoy_inds_->insertAtEnd(next_dec_ind);
    }
    if (accMass_) {
      Array<double>* next_accm_ind = new Array<double>;
      accmass_inds_->insertAtEnd(next_accm_ind);
    }


    //DDS: Separate these models by MSrun elements
    pI_ = modelOptions.pI_;
    RT_ = modelOptions.RT_;

    if(pI_) {
      //Using same pI distribution for all charges
      if (pI_distr == NULL) {
	cout << "adding pI mixture distr" << endl;
	next_distr = factory_->getPIMixtureDistr(charge);    
	pI_distr = (KernelDensityPIMixtureDistr*)next_distr;
      }
      else {
	next_distr = pI_distr;
      }

      if(next_distr != NULL)
	next->insertAtEnd(next_distr);

    }

    if(RT_) {
      //Using same RT distribution for all charges
      if (RT_distr == NULL) {
	cout << "adding Retention Time mixture distr" << endl;
	next_distr = factory_->getRTMixtureDistr(charge);    
	RT_distr = (RTMixtureDistr*)next_distr;
      }
      else {
	next_distr = RT_distr;
      }

      if(next_distr != NULL)
	next->insertAtEnd(next_distr);

    }

  


    /////////////////////////////////////////////////////////////////////////////////////////////
    mixtureDistrs_->insertAtEnd(next);
  }
  

}

void MixtureModel::common_constructor_init() {
   spectra_=NULL;  // spectra by precursor ion charge (0:1+, 1:2+, 2:3+)
   probs_=NULL;    // probs by charge

   // HENRY
   libprobs_ = NULL;
   // END HENRY


   dec_count_ = 0;
   fwd_count_ = 0;
  
  rep_specs_ = NULL;


   mixtureDistrs_=NULL;  // mixture distributions by charge
   inds_=NULL;
   
   run_inds_=NULL; //DDS: indeces ordered by run
   run_chrg_inds_=NULL;
   num_runs_=0;
   counter_=0;
   extra_counter_=0;
   max_num_iters_=0;
   prev_log_term_=NULL;
   maxnumiters_=0;
   
   priors_=NULL; // for each charge
   numspectra_=NULL;
   done_=NULL;  // iterations to model termination
   extradone_=NULL;  // iterations to model termination
   totitrs_ = NULL;
   verbose_=False; // quiet!!!
   icat_=False;  // use icat peptide cys information for probs
   glyc_=False;  // use peptide N-glycosylation motif for probs
   phospho_=False;  // use peptide phospho motif for probs
   use_adj_probs_=True;  // constraint for 2+ and 3+ interpretations of same spectrum
   gamma_=False;  // use gamma distribution for fval distribution among incorrect search results
   num_pairs_=0;
   pairedspecs_=NULL;  
   adjusted_=NULL;
   filename_=NULL;
   max_num_iters_=0;
   negOnly_=NULL;  // no model results, use alternative means to crudely estimate prob ('0' or '-charge')
   maldi_=False;  // maldi data
   maldi_set_=False;
   pseudonegs_=NULL;
   min_num_specs_=0;
   discrim_name_=NULL;
   ntt_name_=NULL;
   nmc_name_=NULL;
   enzyme_=NULL;
   qtof_ = False;
   
   factory_=NULL;
   index_=0;
   
   pI_ptr_=NULL;
   RT_ptr_=NULL;
   isoMass_ptr_= new Array<MixtureDistr*>;
   for (int charge=0; charge<MAX_CHARGE; charge++) {
     isoMass_ptr_->insertAtEnd(NULL);
   }
   ordered_=NULL;
   num_ordered_=0;
   allchg_index_=0;
   use_decoy_=false; // BSP
   has_decoy_=false; // BSP
   nonparam_=false; // BSP
   forcedistr_=false; // BSP
   pI_=false; // BSP
   RT_=false; // BSP
   alldone_=false; // BSP
   accMass_=false; // BSP
   no_mass_=false; // BSP
   min_pI_ntt_=0;  // BSP
   min_pI_prob_=0; // BSP
   min_RT_ntt_=0;  // BSP
   min_RT_prob_=0; // BSP
}

MixtureModel::~MixtureModel() {
  set<MixtureDistr*> itemsToDelete;
  for(int charge = 0; charge < mixtureDistrs_->size(); charge++)
    for(int k = 0; k < (*mixtureDistrs_)[charge]->size(); k++)
      itemsToDelete.insert((*(*mixtureDistrs_)[charge])[k]);

  set<MixtureDistr*>::iterator itdelete;
  for (itdelete = itemsToDelete.begin(); itdelete != itemsToDelete.end(); itdelete++)
    delete *itdelete;
  for(int charge = 0; charge < mixtureDistrs_->size(); charge++)
    delete (*mixtureDistrs_)[charge];

  delete mixtureDistrs_;  // mixture distributions by charge

  for (int i=0; i<spectra_->length(); i++) {
    for (int j=0; j<(*spectra_)[i]->length(); j++)
      delete[] (*(*spectra_)[i])[j];
    delete (*spectra_)[i];
  }
  delete spectra_;  // spectra by precursor ion charge (0:1+, 1:2+, 2:3+)
  for (int i=0; i<probs_->length(); i++)
    delete (*probs_)[i];
  delete probs_;    // probs by charge

  for (int i=0; i<libprobs_->length(); i++)
    delete (*libprobs_)[i];
  delete libprobs_;

  SpectHash::iterator it;
  for (it = rep_specs_->begin(); it != rep_specs_->end(); it++) {
    for (int i=0; i < it->second->length(); i++)
      delete (*(it->second))[i];
    delete it->second;
  }
  delete rep_specs_;
  delete lastitr_totdiff_;
  for (int i=0; i<isdecoy_inds_->length(); i++)
    delete (*isdecoy_inds_)[i];
  delete isdecoy_inds_;
  for (int i=0; i<accmass_inds_->length(); i++)
    delete (*accmass_inds_)[i];
  delete accmass_inds_;

  for (int i=0; i<inds_->length(); i++)
    delete (*inds_)[i];
  delete inds_;
   
  for (int i=0; i<run_inds_->length(); i++)
    delete (*run_inds_)[i];
   delete run_inds_; //DDS: indeces ordered by run
   
   delete[] priors_; // for each charge
   delete[] numspectra_;
   delete[] done_;  // iterations to model termination
   delete[] extradone_;  // iterations to model termination
   delete[] totitrs_;  // iterations to model termination
   delete[] pairedspecs_;  
   delete adjusted_;
   delete[] filename_;
   delete[] pseudonegs_;
   delete[] discrim_name_;
   delete[] ntt_name_;
   delete[] nmc_name_;
   delete[] enzyme_;
   delete[] negOnly_;
   delete[] ignore_;
   delete[] prev_log_term_;
   
   delete factory_;
   
   //pointer to mixture distribution not allocated delete pI_ptr_;
   //pointer to mixture distribution not allocated delete RT_ptr_;

   if (isoMass_ptr_) {
     for (int charge=0; charge<MAX_CHARGE; charge++) {
       delete (*isoMass_ptr_)[charge];
	 }
   }
   delete isoMass_ptr_;
   delete[] ordered_;
   delete[] search_engine_;
}

void MixtureModel::calc_pIstats() {
  for(int k = 0; k < (*mixtureDistrs_)[0]->size(); k++) {
    (*(*mixtureDistrs_)[0])[k]->calc_pIstats();
  }
}

void MixtureModel::process() {
  cerr << " " << PROGRAM_VERSION << " (" << szTPPVersionInfo << ") " << PROGRAM_AUTHOR << endl;
  cerr << " read in ";
  for (int i=0; i<spectra_->length() - 1; i++) {
    cerr << (*spectra_)[i]->size() << " " << i+1 << "+, ";
  }
  cerr << "and " << (*spectra_)[spectra_->length() - 1]->size() << " " << spectra_->length() << "+ spectra." << endl;

  if(strcasecmp(search_engine_, "SPECTRAST") == 0 && optimize_fval_) {
    for (int ch = 0; ch < MAX_CHARGE; ch++) {
      SpectraSTDiscrimValMixtureDistr* spectrast_discrim = (SpectraSTDiscrimValMixtureDistr*)(getMixtureDistr(discrim_name_, ch));
      if (spectrast_discrim) {
	spectrast_discrim->optimizeFval();
      }
    }
  }
  
  validateModel();
  //  for (int i=0; i<run_inds_->size(); i++) {
  //  cout << "DDS: MixMod run=" << i+1 << " spectra=" << (*run_inds_)[i]->size() << endl;
  //}

  deriveModel(max_num_iters_);

  /*
  // HENRY
  if (strcasecmp(search_engine_, "SPECTRAST") == 0) {
    if (multiply_by_spectrast_lib_probs_) {
      multiplyBySpectraSTLibProbs();
    }
  }
  // END HENRY
  */



  // this should ultimately be commented out (for diagnostics only)
  //writeModelfile("new.model", "");


}

void MixtureModel::setEnzyme(char* enz) {
  if(enz == NULL || (enzyme_ != NULL && ! strcmp(enz, enzyme_))) // nothing to do
    return;

  if(enzyme_ != NULL)
    delete enzyme_;
  enzyme_ = new char[strlen(enz)+1];
  strcpy(enzyme_, enz);
  
  // now pass to factory
  if(factory_ != NULL) {
    for(int ch = 0; ch < MAX_CHARGE; ch++) {
      ((NTTMixtureDistr*)(getMixtureDistr(ntt_name_, ch)))->updateName(factory_->getNTTMixtureDistrName(enzyme_));
    }
    if(ntt_name_ != NULL)
     delete ntt_name_;
    ntt_name_ = new char[strlen(factory_->getNTTMixtureDistrName(enzyme_))+1];
    strcpy(ntt_name_, factory_->getNTTMixtureDistrName(enzyme_));
  }
}

//DDS: void MixtureModel::enterData(Array<Tag*>* tags)
void MixtureModel::enterData(Array<Tag*>* tags, int run_idx, string* run_name) {
  if(factory_ == NULL) {
    cerr << "null factory" << endl;
    exit(1);
  }

  SearchResult* result = factory_->getSearchResultWithAppliedOpts(tags);


  if(result != NULL) {
    result->setRunIdx(run_idx);
    result->setRunName(run_name);
    while (run_idx >= num_runs_) {
      num_runs_++;
      Array<int>* nextind = new Array<int>;
      run_inds_->insertAtEnd(nextind);
    }
    (*run_inds_)[run_idx]->insertAtEnd(index_);
    enterData(result);
    delete result;
  }
}


void MixtureModel::enterData(SearchResult* result) {
  
  // must first enter spectrum..., but strip off end charge
  char* spectrum = new char[strlen(result->spectrum_)+1];
  
  std::string tmp = result->spectrum_;
  boost::algorithm::trim(tmp);

  strcpy(spectrum, tmp.c_str());

  if (!strcmp("20100422_04_control_07.10165.10165.10",spectrum) ) {
    cerr << "DDS: DEBUG" << endl;
  }
  //DDS: Doesn't work if charge is more than 2 characters!!!
  // HENRY: strip off charge also if assumed_charge is zero (SpectraST case)
  if((strlen(spectrum) > 2) && 
     (spectrum[strlen(spectrum)-2] == '.') && 
     ((spectrum[strlen(spectrum)-1] == result->charge_ + '0') || (spectrum[strlen(spectrum)-1] == '0'))) {

    spectrum[strlen(spectrum)-2] = 0;
  }
  // END HENRY
  // DDS OMSSA assumed_charge doesn't match spectrum name
  else if (spectrum[strlen(spectrum)-1] != result->charge_ + '0') {
    char* lastdot = strrchr(spectrum, '.');
    int encoded_chg = atoi(lastdot+1);
    if (encoded_chg != result->charge_) {
      cerr << "WARNING: Encoded charge of spectrum " << spectrum << " does not match the assumed_charge " << result->charge_ << endl;
      return;
    }
    *lastdot = 0;

    //    spectrum[strlen(spectrum)-2] = 0;
  }
  // END DDS
 
  int charge = result->charge_ - 1;
  
  if (accMass_) {
    (*accmass_inds_)[charge]->insertAtEnd(result->neutral_mass_);
  }

  // HENRY
  if (strcasecmp(search_engine_, "SPECTRAST") == 0) {
    (*libprobs_)[charge]->insertAtEnd(((SpectraSTResult*)result)->libprob_);  
  }
  // END HENRY

  bool valid = true;

  for(int k = 0; k < (*mixtureDistrs_)[charge]->size(); k++) {
    if (! strcmp((*(*mixtureDistrs_)[charge])[k]->getName(),  discrim_name_) ) {
      valid = (*(*mixtureDistrs_)[charge])[k]->valid(result);
      if (! valid) return;
    }
    (*(*mixtureDistrs_)[charge])[k]->enter(result);
  }
  
  (*spectra_)[charge]->insertAtEnd(spectrum);
  (*probs_)[charge]->insertAtEnd(0.5);
  (*inds_)[charge]->insertAtEnd(index_++);
   if (use_decoy_) {
     bool thisdecoy = true;

     for (int t = 0; t < result->proteins_->size(); t++) {
     //     for (int t = 0; t < 1; t++) {
       if ((*result->proteins_)[t] == strstr((*result->proteins_)[t], decoy_label_) ||
	   (strcasecmp(search_engine_, "SPECTRAST") == 0 && 
	    ((SpectraSTResult*)result)->libremark_.find(decoy_label_) == 0)) {
	 thisdecoy &= true;
       }
       else {
	 thisdecoy = false;
	 break;
       }
	 //protein name starts with DECOY tag, or in SpectraST's case, either protein libremark starts with DECOY tag
	 
     }


     if (thisdecoy) {
       (*isdecoy_inds_)[charge]->insertAtEnd(True);
       //cerr << result->spectrum_ << endl;
       has_decoy_ = true;
       dec_count_++;
       
     }
     else {
       (*isdecoy_inds_)[charge]->insertAtEnd(False);
       fwd_count_++;
     }
   }
   
}


void MixtureModel::assessPeptideProperties(char* filename, Boolean icat, Boolean glyc) {

  ifstream fin(filename);
  if(! fin) {
    cerr << "could not read " << filename << endl;
    exit(1);
  }
  char tag[75];
  char value[100];
  char pep[100];
  char database_tag[] = "DATABASE=";
  char* match;
  icat = icat_ || ! icat; // False to look for icat
  glyc = glyc_ || ! glyc; // whether or not to look for glyc
  int num_unmod = 0;
  int num_mod = 0;
  int num_neg = 0;
  double fval = -999;
  double min_fval = 1.8;

  if(icat && glyc)
    return; // nothing to do

  const int datasize = 300;
  char data[datasize];

  // eat first line with database
  if(! fin.getline(data, datasize)) {
    cerr << "could not read " << filename << endl;
    exit(1);
  }

  
  while(fin >> tag) {

    if(strcmp(tag, "pep") == 0) {
      fin >> pep;
      if(! icat && strstr(pep, "C") != NULL && fval >= min_fval) {
	if(strstr(pep, "C*") != NULL || strstr(pep, "C#") != NULL || strstr(pep, "C@") != NULL) 
	  num_mod++;
	else
	  num_unmod++;
      }
      else if(fval >= min_fval)
	num_neg++;
      if(! glyc) {
	match = strstr(pep, "N*");
	if(match != NULL && strlen(match) > 3 && (match[3] == 'S' || match[3] == 'T'))
	  glyc_ = True;
	if(! glyc_) {
	  match = strstr(pep, "N#");
	  if(match != NULL && strlen(match) > 3 && (match[3] == 'S' || match[3] == 'T'))
	    glyc_ = True;
	}
      }
      glyc = glyc || glyc_;
      if(icat && glyc) {
	fin.close();
	return;
      }
    } // if spec tag
    else if(strcmp(tag, "fval") == 0) {
      fin >> fval;
    }
    else {
      fin >> value;
    }

  } // while
  fin.close();
  double pct_mod = 0; 
  double pct_pos = 0; 

  if(num_mod + num_unmod > 0) {
    pct_mod = double(num_mod)/(num_mod + num_unmod);
    pct_pos = double(num_mod + num_unmod)/(num_mod + num_unmod + num_neg);
    if(pct_pos > 0.75 && pct_mod > 0)
      icat_ = True;
    else if(pct_pos > 0.5 && pct_mod > 0.2 && pct_mod < 0.8)
      icat_ = True;
  }

}

void MixtureModel::setMixtureDistributionNames(const char* discrim, const char* ntt, const char* nmc) {
  discrim_name_ = new char[strlen(discrim)+1];
  strcpy(discrim_name_, discrim);
  discrim_name_[strlen(discrim)] = 0;
  ntt_name_ = new char[strlen(ntt)+1];
  strcpy(ntt_name_, ntt);
  ntt_name_[strlen(ntt)] = 0;
  nmc_name_ = new char[strlen(nmc)+1];
  strcpy(nmc_name_, nmc);
  nmc_name_[strlen(nmc)] = 0;
}

// after data entry, make sure have same number of data points in each mixture distr
void MixtureModel::validateModel() {
  int tot = 0;
  MixtureDistr* pI_dist = NULL;
  MixtureDistr* RT_dist = NULL;

  for(int charge = 0; charge < MAX_CHARGE; charge++) {
    tot += (*spectra_)[charge]->size();
    for(int k = 0; k < (*mixtureDistrs_)[charge]->size(); k++) {
      if (pI_dist == (*(*mixtureDistrs_)[charge])[k] || 
	  strcmp((*(*mixtureDistrs_)[charge])[k]->getName(), "kernel density calc pI [pI]")==0) {
	pI_dist = (*(*mixtureDistrs_)[charge])[k];
	continue;
      }
      if (RT_dist == (*(*mixtureDistrs_)[charge])[k] || 
	  strcmp((*(*mixtureDistrs_)[charge])[k]->getName(), "kernel density SSRCalc RT [RT]")==0) {
	RT_dist = (*(*mixtureDistrs_)[charge])[k];
	continue;
      }

      if (AM_ptr_ == (*(*mixtureDistrs_)[charge])[k]) {
	continue;
      }
      if((*spectra_)[charge]->size() != (*(*mixtureDistrs_)[charge])[k]->getNumVals()) {
	cerr << "have " << (*spectra_)[charge]->size() << " spectra and " << (*(*mixtureDistrs_)[charge])[k]->getNumVals() << " values for " << (*(*mixtureDistrs_)[charge])[k]->getName() << " charge " << (charge+1) << endl;
	exit(1);
      }
    } // next distr
  } // next charge
  if(tot == 0) { // nothing to do...
    cerr << " read in no data" << endl;
    exit(1);
  }
 
  // if (pI_dist != NULL && tot != pI_dist->getNumVals()) {
  //  cerr << "have " << tot << " spectra and " << pI_dist->getNumVals() 
  //	 << " values for " << pI_dist->getName() << endl;
  //  exit(1);
  //}

}

char* MixtureModel::getEnzyme(const char* filename) {
  char enz_tag[] = "ENZYME=";
  ifstream fin(filename);
  if(! fin) {
    cerr << "could not read " << filename << endl;
    exit(1);
  }
  const int datasize = 300;
  char data[datasize];
  if(! fin.getline(data, datasize)) {
    cerr << "could not open " << filename << endl;
    fin.close();
    exit(1);
  }
  fin.close();
  return getTagValue(data, enz_tag);
}

// based on spectrum names, deduces whether or not maldi
Boolean MixtureModel::getMaldiInfo(char* filename) {
  ifstream fin(filename);
  if(! fin) {
    cerr << "could not read " << filename << endl;
    exit(1);
  }


  char maldi_tag[] = "MALDI="; // should be in first line of file
  const int datasize = 500;
  char data[datasize];

  if(! fin.getline(data, datasize)) {
    cerr << "cannot open " << filename << endl;
    exit(1);
  }

  char* result = strstr(data, maldi_tag);

  if(result != NULL) {
    if(strlen(result) > strlen(maldi_tag) && result[strlen(maldi_tag)] == '1') 
      maldi_ = True;
    else maldi_ = False;
    maldi_set_ = True;
    fin.close();
    return True; // done
  }


  // if no tag, find from spectrum name (to be taken away in future)

  char tag[75];
  char value[100];
  char spec[100];
      
  // eat first line
  if(! fin.getline(data, datasize)) {
    cerr << "cannot open " << filename << endl;
    exit(1);
  }

  while(fin >> tag) {

    if(strcmp(tag, "spec") == 0) {
      fin >> spec;
      if(! maldi_set_) {
	setMaldi(spec);
      }
      fin.close();
      return True;
    } // if spec tag
    else {
      fin >> value;
    }

  } // while
  fin.close();
  return False;
}

// initialize fval negative distributions with 0 tryptic termini data, if available
void MixtureModel::setNegativeDiscrimValDistrs() { 
  cout << "Initialising statistical models ..." << endl;
  
  if (use_decoy_) {
    cerr << "Found " << dec_count_ << " Decoys, and " << fwd_count_ << " Non-Decoys" << endl;
    
    if (!has_decoy_) {
      cerr << "WARNING: No decoys with label " << decoy_label_ << " were found in this dataset. " 
	   << "reverting to fully unsupervised method." << endl;
      use_decoy_ = False;
    }
  }

  for(int charge = 0; charge < MAX_CHARGE; charge++) {
    if ( use_decoy_ && getMixtureDistr(discrim_name_, charge) != NULL) {
      pseudonegs_[charge] = ((DiscrimValMixtureDistr*)(getMixtureDistr(discrim_name_, charge)))->
         initializeNegDistribution((*isdecoy_inds_)[charge]);
    }
    else 
    if( !no_ntt_ && getMixtureDistr(discrim_name_, charge) != NULL && getMixtureDistr(ntt_name_, charge) != NULL) {
      pseudonegs_[charge] = ((DiscrimValMixtureDistr*)(getMixtureDistr(discrim_name_, charge)))->
         initializeNegDistribution(((NTTMixtureDistr*)(getMixtureDistr(ntt_name_, charge))));
    }
  }

}


MixtureDistr* MixtureModel::getMixtureDistr(char* name, int charge) {
  for(int k = 0; k < (*mixtureDistrs_)[charge]->size(); k++) {
    if(strcmp((*(*mixtureDistrs_)[charge])[k]->getName(), name) == 0) {
      return (*(*mixtureDistrs_)[charge])[k];
    }
  }
  return NULL;
}


double MixtureModel::getTotalProb(int charge) {
  if(ignore_[charge] || (negOnly_[charge] && !forcedistr_)) {
    return 0.0;
  }
  double tot = 0.0;

  for(int k = 0; k < (*probs_)[charge]->size(); k++) {
    tot += (*(*probs_)[charge])[k];
  }
  
  return tot;
}

double MixtureModel::getTotalAdjProb(int charge) {
  if(ignore_[charge] || (negOnly_[charge] && !forcedistr_)) {
    return 0.0;
  }

  double tot = 0.0;
  if(charge > 0 && charge < MAX_CHARGE && use_adj_probs_) {
    if (rep_specs_ == NULL) {
      //      computeAdjDoubleTriplySpectraProbs();

      computeAdjMultiChargeSpectraProbs();
    }
    /* BSP implicit assumption ("k+=2") that each spectra produced 2+ and 3+, not always true
     for(int k = 0; k < num_pairs_; k+=2) {
      if(! negOnly_[atoi(pairedspecs_[k].name_ + strlen(pairedspecs_[k].name_)-1)-1]) {
	tot += pairedspecs_[k+charge-1].prob_;  BSP crash when num_pairs_=3097 k=3906 charge=2
      }
    */
    for(SpectHash::const_iterator z = rep_specs_->begin(); z != rep_specs_->end(); z++) {
      for (int i=0; i< (*(*z).second).size(); i++) {
	if (charge==(*z->second)[i]->charge_) {
	  tot += (*(*probs_)[(*z->second)[i]->charge_])[(*z->second)[i]->data_index_]; 
	  break;
	}
      }
    }

  }
  else {
    for(int k = 0; k < (*probs_)[charge]->size(); k++) {
      tot += (*(*probs_)[charge])[k];
    }
  }
  return tot;
}

Boolean MixtureModel::updateDistr(char* name, int charge) {
  for(int k = 0; k < (*mixtureDistrs_)[charge]->size(); k++) {
    if(strcmp((*(*mixtureDistrs_)[charge])[k]->getName(), name) == 0) {
      return ((*(*mixtureDistrs_)[charge])[k]->update((*probs_)[charge]));
    }
  }

  return False;
}

Boolean MixtureModel::updateFinalDistr(int charge) {
  for(int k = 0; k < (*mixtureDistrs_)[charge]->size(); k++) {
    if(strcmp((*(*mixtureDistrs_)[charge])[k]->getName(), discrim_name_) == 0) {
	return (((DiscrimValMixtureDistr*)((*(*mixtureDistrs_)[charge])[k]))->finalupdate((*probs_)[charge]));
    }
  }

  return False;
}

void MixtureModel::deriveModel(int maxnumiters) {
  maxnumiters_ = maxnumiters;
  // here if use mixture positives....then initialize with NTTMixtureDistr....
  if(maldi_ && getMixtureDistr(discrim_name_, 0) != NULL && getMixtureDistr(ntt_name_, 0) != NULL &&
     getMixtureDistr(discrim_name_, 0)->getPosDistr() != NULL) {
    ((DecayContinuousMultimixtureDistr*)(getMixtureDistr(discrim_name_, 0)->getPosDistr()))->initWithNTTs((NTTMixtureDistr*)(getMixtureDistr(ntt_name_, 0)));

  } // maldi
  if(qtof_)
    for(int ch = 0; ch < MAX_CHARGE; ch++)
      if(getMixtureDistr(discrim_name_, ch) != NULL && getMixtureDistr(ntt_name_, ch) != NULL &&
	 getMixtureDistr(discrim_name_, ch)->getPosDistr() != NULL) 
	((DecayContinuousMultimixtureDistr*)(getMixtureDistr(discrim_name_, ch)->getPosDistr()))->initWithNTTs((NTTMixtureDistr*)(getMixtureDistr(ntt_name_, ch)));

  if ( ! no_neg_init_ ) {
    setNegativeDiscrimValDistrs();
  }

  counter_ = 1;
  extra_counter_ = 1;
  
  cout << "Iterations: " ;
  while( iterate(counter_)) {
     if (verbose_) {
       if(counter_ == 1) {
	 cout << " iteration ";
       }
       else {
	 cout << '\b';
	 cout << '\b';
       }
       if(counter_ > 10) {
	 cout << '\b';
       }
       if(counter_ > 100) {
	 cout << '\b';
       }
       cout << counter_ << " ";
       cout.flush();
     } // end if verbose
     else {
       if (counter_ % 10 == 0) {
	 cout << counter_;
       }
       else {
	 cout << ".";
       }
       cout .flush();
     }
     counter_++;
  }
  if (verbose_) {
    for(int k = 0; k < 12; k++) {
      cout << '\b';
    }
    if(counter_ > 10) {
      cout << '\b';
    }
    if(counter_ > 100) {
      cout << '\b';
    }
     } //end if verbose_
  cout << "model complete after " << counter_ << " iterations" << endl;

  /* *****
  if (pI_ptr_ != NULL) {
    //Remove outliers after convergence
     Array<Array<double>*>* probs = new Array<Array<double>*>;
    for (int i=0; i<num_runs_; i++) {
      probs->insertAtEnd(new Array<double>);
      for (int j=0; j<(*run_inds_)[i]->size(); j++) {
	(*probs)[i]->insertAtEnd(getOrderedProb(i, j));
      }    
    }

  
    
    pI_ptr_->update(probs, 0.99);
  
    for (int i=0; i<num_runs_; i++) {
      delete (*probs)[i];
    }
    delete probs;
  }
  ****** */

  // if not both satisfactorily modeled
  for(int charge = 0; charge < MAX_CHARGE; charge++) {
    //    if (!ignore_[charge]) {
      if((*spectra_)[charge]->size() > 0 && 
	 ((*spectra_)[charge]->size() < min_num_specs_ || 
	  (priors_[charge] == 0.0 && (*spectra_)[charge]->size() > 0))) {
	negOnly_[charge] = negOnly(charge);
      }
      //    }
      //else {
      //negOnly_[charge] = ignore_[charge];
      //}
  }
  //  for(int charge = 0; charge < MAX_CHARGE; charge++) {
  //  negOnly_[charge] = ignore_[charge];
  //}

}


Boolean MixtureModel::iterate(int counter, int charge, Boolean force) {
  Boolean verbose = False;

  
  if(!use_decoy_ && !forcedistr_ &&
     (priors_[charge] == 0.0 || 
      ((DiscrimValMixtureDistr*)(getMixtureDistr(discrim_name_, charge)))->noDistr()) ) {

    if(((DiscrimValMixtureDistr*)(getMixtureDistr(discrim_name_, charge)))->reinitialize()) {
      priors_[charge] = -1;
      updateProbs(charge, force);
      updatePriors(charge);
      return True;
    }
    else {
      priors_[charge] = 0.0;
      if(verbose)
	cout << "counter: " << counter << " returning false..." << endl;
      return False;
    }
  }
  
  if((*spectra_)[charge]->size() == 0) {
    updatePriors(charge);
    return False; // done
  }
  else if((*spectra_)[charge]->size() < min_num_specs_) {
    return False; // done, and will handle via negonly later
  }

  Boolean output = False;
  
  if(updateProbs(charge, force)) {
    output = True;
    if(verbose)
      cout << "probs true" << endl;
  }

  if(updateDistr(discrim_name_, charge)) {
    output = True;
    if(verbose)
      cout << "discrim true" << endl;
  }

  if(updatePriors(charge)) {
    output = True;
    if(verbose)
      cout << "priors true" << endl;
  }

  if(!no_ntt_ && updateDistr(ntt_name_, charge)) {
    output = True;
  }

  if(!no_nmc_ && updateDistr(nmc_name_, charge)) {
    output = True;
  }
  
  if(updateProbs(charge, force)) {
    ; 
  }

  if(verbose)
    cout << "Update distributions" << endl;

  // do the rest here
  for(int k = 2; k < (*mixtureDistrs_)[charge]->size(); k++) {
    if(verbose)
      cout << "next: " << (*(*mixtureDistrs_)[charge])[k]->getName() << endl;
    
    //DDS: update pI only once per iter
    if ((AM_ptr_ != NULL && (*(*mixtureDistrs_)[charge])[k] == AM_ptr_) ||
	strcmp((*(*mixtureDistrs_)[charge])[k]->getName(), "kernel density calc pI [pI]")==0 || 
	strcmp((*(*mixtureDistrs_)[charge])[k]->getName(), "kernel density SSRCalc RT [RT]")==0 ) { 
      
      //||
      //  strcmp((*(*mixtureDistrs_)[charge])[k]->getName(), "isotopic peak mass difference [IsoMassDiff]")==0) {
      
      //  for (int i=0; i<num_runs_; i++) {
      //	Array<double>* run_probs = new Array<double>;
      //	probs->insertAtEnd(run_probs);
      //	for (int j=0; j<(*run_inds_)[i]->size(); j++) {
      //	  (*probs)[i]->insertAtEnd(getOrderedProb(i, j));
      //	}    
      //}
    
     
    

      //pI_distr = (*(*mixtureDistrs_)[charge])[k];
      // skip update of pI, done only once per iteration 
      continue;
      // }
    }
    else {
      if (no_ntt_ && strcmp((*(*mixtureDistrs_)[charge])[k]->getName(), ntt_name_)==0) {
	output = False;
      }
      else if (no_nmc_ && strcmp((*(*mixtureDistrs_)[charge])[k]->getName(), nmc_name_)==0) {
	output = False;
      }
      else if((*(*mixtureDistrs_)[charge])[k]->update(probs_, charge)) {
	output = True;
      }
    }
  }
  

  return output;
}

Boolean MixtureModel::iterate(int counter) {
  Boolean output = False;
  alldone_ = False;
  int charge;
  // reached max number of iterations
  if (counter >= maxnumiters_) {
    alldone_ = True;
    output = False;
  }
  else {
    for(charge = 0; charge < MAX_CHARGE; charge++) {
      if (done_[charge] < 0) {
	if (ignore_[charge] || (negOnly_[charge] && !forcedistr_)) {
	  totitrs_[charge] = counter;
	  done_[charge] = counter;
	}
	else if(! iterate(counter, charge, false)) {
	  totitrs_[charge] = counter;
	  done_[charge] = counter;
	}
      }
      if (done_[charge] >= 0 && extradone_[charge] < 0 ) {
	if (ignore_[charge] || (negOnly_[charge] && !forcedistr_)) {
	  totitrs_[charge] = counter;
	  extradone_[charge] = counter;
	}      
	else if (counter - done_[charge] < extraitrs_) {
	  iterate(counter, charge, true);
	}
	else {
	  totitrs_[charge] = counter;
	  extradone_[charge] = counter;
	}
      }
      if(done_[charge] < 0 || extradone_[charge] < 0) {
	output = True;
      }
    }
  }

  if (AM_ptr_ != NULL) {
    Array<Array<double>*>* probs = new Array<Array<double>*>;
    for (int i=0; i<num_runs_; i++) {
      Array<double>* run_probs = new Array<double>;
      probs->insertAtEnd(run_probs);
      for (int j=0; j<(*run_inds_)[i]->size(); j++) {
	  (*probs)[i]->insertAtEnd(getOrderedProb(i, j));
      }
    }
    output = AM_ptr_->update(probs) || output;

    for (int j=0; j<num_runs_; j++) {
      delete (*probs)[j];
    }
    delete probs;

    if (!output) {
      for(int charge = 0; charge < MAX_CHARGE; charge++) {
	done_[charge] = -1;
	extradone_[charge] = -1;
      }
    }
  }

  alldone_ = !output;
  
  //DDS: Update pI model
  MixtureDistr* pI_ptr = NULL;
  if (pI_ && alldone_ && pI_ptr_ == NULL) {
    if (pI_ptr_ == NULL) {
      for(int k = 0; k < (*mixtureDistrs_)[0]->size(); k++) {
  	if (strcmp((*(*mixtureDistrs_)[0])[k]->getName(), "kernel density calc pI [pI]")==0) {
  	  pI_ptr = (*(*mixtureDistrs_)[0])[k];
  	  output = True;
  	  //for(int charge = 0; charge < MAX_CHARGE; charge++) {
  	  //  done_[charge] = -1;
  	  //}
  	  break;
  	}
      }
      pI_ptr_ = pI_ptr;
    }
     
    Array<Array<double>*>* probs = new Array<Array<double>*>;
    Array<Array<int>*>* ntts = new Array<Array<int>*>;
    for (int i=0; i<num_runs_; i++) {
      Array<double>* run_probs = new Array<double>;
      Array<int>* run_ntts = new Array<int>;
      probs->insertAtEnd(run_probs);
      ntts->insertAtEnd(run_ntts);
      for (int j=0; j<(*run_inds_)[i]->size(); j++) {
	  (*probs)[i]->insertAtEnd(getOrderedProb(i, j));
	  (*ntts)[i]->insertAtEnd(getOrderedNTT(i, j));
      }
    }
    
    
    output = pI_ptr_->update(probs, min_pI_prob_, ntts, min_pI_ntt_);
 
    output = False;

    for (int j=0; j<num_runs_; j++) {
      delete (*probs)[j];
      delete (*ntts)[j];
    }
    delete probs;
    delete ntts;

  }
  
  //DDS: Update RT model
  MixtureDistr* RT_ptr = NULL;
  if (RT_ptr_ != NULL) {

//DDS: Learn RT coeffs from all peptides together (not by run) 
//     Array<double>* probs = new Array<double>(index_);
//     for (int i=0; i<num_runs_; i++) {
//       for (int j=0; j<(*run_inds_)[i]->size(); j++) {
// 	(*probs)[(*(*run_inds_)[i])[j]] = getOrderedProb(i, j);
//       }
//     }
    
//  RT_ptr_->update(probs);
//    delete probs;


    RT_ptr_->update(probs_, rtcat_file_);
  }
  else if (RT_ && alldone_ && RT_ptr_ == NULL) {
    cout << "Estimating Retention Time Model ... please wait ... " << endl;
    if (RT_ptr_ == NULL) {
      for(int k = 0; k < (*mixtureDistrs_)[0]->size(); k++) {
  	if (strcmp((*(*mixtureDistrs_)[0])[k]->getName(), "kernel density SSRCalc RT [RT]")==0) {
  	  RT_ptr = (*(*mixtureDistrs_)[0])[k];
  	  output = True;
  	  //for(int charge = 0; charge < MAX_CHARGE; charge++) {
  	  //  done_[charge] = -1;
  	  //}
  	  break;
  	}
      }
      RT_ptr_ = RT_ptr;
    }
     
    Array<Array<double>*>* probs = new Array<Array<double>*>;
    Array<Array<int>*>* ntts = new Array<Array<int>*>;
    for (int i=0; i<num_runs_; i++) {
      Array<double>* run_probs = new Array<double>;
      Array<int>* run_ntts = new Array<int>;
      probs->insertAtEnd(run_probs);
      ntts->insertAtEnd(run_ntts);
      for (int j=0; j<(*run_inds_)[i]->size(); j++) {
	  (*probs)[i]->insertAtEnd(getOrderedProb(i, j));
	  (*ntts)[i]->insertAtEnd(getOrderedNTT(i, j));
      }
    }

   

//    Array<double>* probs = new Array<double>(index_);
//    Array<int>* ntts = new Array<int>(index_);

//DDS: Learn RT coeffs from all peptides together (not by run) 
    //for (int i=0; i<num_runs_; i++) {
    //  for (int j=0; j<(*run_inds_)[i]->size(); j++) {
    //	  (*probs)[(*(*run_inds_)[i])[j]] = getOrderedProb(i, j);
    //	  (*ntts)[(*(*run_inds_)[i])[j]] = getOrderedNTT(i, j);
    //     }
    //}
    
    
    int code=0;

    output = RT_ptr_->update(probs, min_RT_prob_, ntts, min_RT_ntt_, code, rtcat_file_);
      
    if (code >= 1) {
      //delete RT_ptr_;
      //RT_ptr_ = NULL;
      RT_ = False;
    }

    alldone_ = !output;
    
    for (int j=0; j<num_runs_; j++) {
      delete (*probs)[j];
      delete (*ntts)[j];
    }

    delete probs;
    delete ntts;
    for(int charge = 0; charge < MAX_CHARGE; charge++) {
      done_[charge] = -1;
      maxnumiters_ *= 2;
    }
  }

  if (alldone_) {
    cout << endl;
    for(int charge = 0; charge < MAX_CHARGE; charge++) {
      updatePriors(charge);
      if (!forcedistr_) {
	negOnly_[charge] = updateFinalDistr(charge);
	if (ignore_[charge] || (negOnly_[charge] && !forcedistr_)) {
	  priors_[charge] = 0.0;
	}
	updateProbs(charge, True);
      }
      else {
	updateFinalDistr(charge);
	if (ignore_[charge] || (negOnly_[charge] && !forcedistr_)) {
	  priors_[charge] = 0.0;
	}
	updateProbs(charge, True);
      }
      
      if (done_[charge] < 0) {
        // DCT - no assignment? comment to stop clang warning
	//done_[charge];
      }
      else if (priors_[charge] > 0.0) {
	done_[charge] += extraitrs_;
      }

    }
  }
  
  //DDS: Done pI model update

  return output;
}

void MixtureModel::write_pIstats(ostream& out) {
  if (pI_ptr_ != NULL) pI_ptr_->write_pIstats(out);
}

void MixtureModel::write_RTstats(ostream& out) {
  if (RT_ptr_ != NULL) RT_ptr_->write_RTstats(out);
}

void MixtureModel::write_RTcoeff(ostream& out) {
  if (RT_ptr_ != NULL) RT_ptr_->write_RTcoeff(out);
}

Boolean MixtureModel::updateProbs(int charge, Boolean force) {
  Boolean output = False;

  const double maxdiff = 0.1;

  Array<double>* newprobs = new Array<double>;

  for(int k = 0; k < (*probs_)[charge]->size(); k++) {
    newprobs->insertAtEnd(computeProb(charge, k));
     if(fabs((*newprobs)[k] - (*(*probs_)[charge])[k]) >= maxdiff) {
       output = True;
    } 
  }
 
  if(output || force) {
    assert(newprobs->size() == (*probs_)[charge]->size());
    delete (*probs_)[charge];
    (*probs_)[charge] = newprobs;
  }
  else {
    delete newprobs;
  }

  return output;
}


Boolean MixtureModel::updatePriors(int charge) {

  Boolean output = False;
  double maxdiff = 0.002;
    
  // HENRY: Need tighter convergence tolerance for probabilities!!
  if (strcasecmp(search_engine_, "SPECTRAST") == 0) {
  	maxdiff = 0.00001;
  }
  // END HENRY

  double next = 0;
  int tot = 0;
  for(int k = 0; k < (*probs_)[charge]->size(); k++) {
    if((*(*probs_)[charge])[k] >= 0) {
      if ( !use_decoy_  || !(*(*isdecoy_inds_)[charge])[k] || (output_decoy_probs_ && alldone_) ) {
	next += (*(*probs_)[charge])[k];
	tot++;
      }
    }
  }
  if(tot > 0) {
    next /= tot;
  }
  double diff = next - priors_[charge];
  if(priors_[charge] == -1 || myfabs(diff) >= maxdiff) {
    output = True;
  }
  priors_[charge] = next;
  return output;
}


int MixtureModel::getNegOnlyCharge(int charge) {
  int alternatives[] = {1, 2, 1, 2, 3, 3, 3};
  int secondalts[] = {2, 0, 0, 1, 2, 2, 2};
  if(! negOnly_[alternatives[charge]] && priors_[alternatives[charge]] != 0.0 && (*spectra_)[alternatives[charge]]->size() >= min_num_specs_) {
    return alternatives[charge];
  }
  if(! negOnly_[secondalts[charge]] && priors_[secondalts[charge]] != 0.0 && (*spectra_)[secondalts[charge]]->size() >= min_num_specs_) {
    return secondalts[charge];
  }
  return -1;
}


// deprecated
Boolean MixtureModel::maldi(char* spec) {
  return False; 

}

void MixtureModel::setMaldi(char* spec) {
  if(maldi_set_) {
    return;
  }
  maldi_ = False;  
  // rely on maldi tag from .esi file in future...

  maldi_set_ = True;
}


Boolean MixtureModel::isNumber(char c) {
  return c >= '0' && c <= '9';
}

Boolean MixtureModel::negOnly(int charge) {
  double min_prob = 0.9; // below that, call a '0'
  Boolean output = False;
  double negonlyprior = 0.1; // default setting

  int alt = getNegOnlyCharge(charge);

  if(alt < 0) {

    // use training data only....
    ((DiscrimValMixtureDistr*)(getMixtureDistr(discrim_name_, charge)))->reset(); // use training settings
  }
  else {

    if((*spectra_)[charge]->size() < 15) {
      // use training data anyway.
      ((DiscrimValMixtureDistr*)(getMixtureDistr(discrim_name_, charge)))->reset(); // use training settings
    }

    for(int k = 0; k < (*mixtureDistrs_)[charge]->size(); k++) {
      if((*(*mixtureDistrs_)[charge])[k]->negOnly()) {
    	(*(*mixtureDistrs_)[charge])[k]->setPosDistr((*(*mixtureDistrs_)[alt])[k]);
    	(*(*mixtureDistrs_)[charge])[k]->setNegDistr((*(*mixtureDistrs_)[alt])[k]);
      }
    } // next distr

    negonlyprior = priors_[alt];
  }

  output = True;

  // now compute rough estimated probs using the new distribution and negonlypriors....
  double posprob;
  double negprob;
  double prob;

  for(int d = 0; d < (*probs_)[charge]->size(); d++) {
    prob = ((DiscrimValMixtureDistr*)(getMixtureDistr(discrim_name_, charge)))->getRightCumulativeNegProb(1.0, d, 10.0);
    posprob = (1.0 - prob) * negonlyprior;
    negprob = prob * (1.0 - negonlyprior);

    if(alt >= 0) {
      for(int k = 0; k < (*mixtureDistrs_)[charge]->size(); k++) {
	if((*(*mixtureDistrs_)[charge])[k]->negOnly()) {
	  posprob *= (*(*mixtureDistrs_)[charge])[k]->getPosProb(d);
	  negprob *= (*(*mixtureDistrs_)[charge])[k]->getNegProb(d);
	}
      }
    }
    if(posprob == 0.0) {
      (*probs_)[charge]->replace(d, 0.0);
    }
    else {
      prob = posprob / (posprob + negprob);
      if(!ignore_[charge] && prob >= min_prob) {
	prob = -1.0 * (charge) - 1.0;
      }
      else {
	prob = 0.0;
      }
      (*probs_)[charge]->replace(d, prob);
    }
  } // next data member
  priors_[charge] = 0.0; // set this to 0

  ((DiscrimValMixtureDistr*)(getMixtureDistr(discrim_name_, charge)))->resetTot(); // set prior to 0  

  //  if(charge == 1 || charge == 2) {
  //  use_adj_probs_ = False;
  //}

  return output;
}

double MixtureModel::computeProb(int charge, int index) {
  assert(charge < probs_->size());
  assert(index < (*probs_)[charge]->size());



  double posprob = 1.0;
  double negprob = 1.0;

  if ( ignore_[charge] ) {
    return 0;
  }

  if(priors_ != NULL && priors_[charge] >= 0 ) {
    posprob = priors_[charge];
    negprob = (1.0 - priors_[charge]);
  }

  MixtureDistr* pI_ptr = NULL;
  MixtureDistr* RT_ptr = NULL;
  MixtureDistr* massDiff_ptr = NULL;
  MixtureDistr* isoMass_ptr = NULL;
  for(int k = 0; k < (*mixtureDistrs_)[charge]->size(); k++) {
    if (strcmp((*(*mixtureDistrs_)[charge])[k]->getName(), "kernel density calc pI [pI]")==0) {
      pI_ptr = (*(*mixtureDistrs_)[charge])[k]; 
    }
    else if (strcmp((*(*mixtureDistrs_)[charge])[k]->getName(), "kernel density SSRCalc RT [RT]")==0) {
      RT_ptr = (*(*mixtureDistrs_)[charge])[k]; 
    }
    else if (strcmp((*(*mixtureDistrs_)[charge])[k]->getName(), "isotopic peak mass difference [IsoMassDiff]")==0) {
      isoMass_ptr = (*(*mixtureDistrs_)[charge])[k]; 
    }
    else if (accMass_ && strcmp((*(*mixtureDistrs_)[charge])[k]->getName(), "accurate mass diff [massd]")==0 )  {
      massDiff_ptr = (*(*mixtureDistrs_)[charge])[k]; 
    }
    else if (no_ntt_ && strcmp((*(*mixtureDistrs_)[charge])[k]->getName(), ntt_name_)==0 )  {
    }
    else if (no_nmc_ && strcmp((*(*mixtureDistrs_)[charge])[k]->getName(), nmc_name_)==0 )  {
    }
    else {
      assert(index < (*(*mixtureDistrs_)[charge])[k]->getNumVals());
      posprob *= (*(*mixtureDistrs_)[charge])[k]->getPosProb(index);
      negprob *= (*(*mixtureDistrs_)[charge])[k]->getNegProb(index);
    }
  }

  double tmppos = 1;
  double tmpneg = 1;



  if (massDiff_ptr != NULL ) {
    tmppos *= massDiff_ptr->getPosProb((*(*inds_)[charge])[index]);
    tmpneg *= massDiff_ptr->getNegProb((*(*inds_)[charge])[index]);   
  }
  if (isoMass_ptr != NULL) {
    tmppos *= isoMass_ptr->getPosProb(index);
    tmpneg *= isoMass_ptr->getNegProb(index);         
  }

  double maxrwd = 10;
  double maxPIrwd = 10;
  double maxRTrwd = 10;

  
  if (conservative_ > 0.01) {
    if (!strcasecmp(search_engine_, "INSPECT")) {
      maxrwd = 3;
      maxPIrwd = 5;
    }
    
    if (!strcasecmp(search_engine_, "OMSSA")) {
      maxrwd = 5;
    }
    
    if (!strcasecmp(search_engine_, "MASCOT")) {
      maxrwd = 5;
      maxPIrwd = 4;
      maxRTrwd = 2;
    }
    
    if (!strcasecmp(search_engine_, "XTANDEM")) {
      maxRTrwd = 2;
    }
    
    
    if (!strcasecmp(search_engine_, "SEQUEST")) {
      maxRTrwd = 2;
    }
    
    if (!strcasecmp(search_engine_, "COMET")) {
      maxRTrwd = 2;
    }
    
    if (!strcasecmp(search_engine_, "MYRIMATCH")) {
      maxRTrwd = 2;
      maxrwd = 3;
      maxPIrwd = 5;
    }
  }


  // if (!strcasecmp(search_engine_, "SEQUEST")) {
  //  maxPIrwd = 4;
  //}



  if ( !strcasecmp(search_engine_, "INSPECT") || !strcasecmp(search_engine_, "MASCOT") || alldone_) { // last iteration
    //if ( (no_ntt_ || (accMass_ && (*(*accmass_inds_)[charge])[index] < 800)) && tmppos > tmpneg) {

    if (tmppos > tmpneg) {
      tmppos = tmppos > tmpneg * maxrwd ? tmpneg * maxrwd : tmppos;
    }

    if (use_decoy_ && (*(*isdecoy_inds_)[charge])[index] && !output_decoy_probs_) {
      return 0.0;
    }
  }
  else if (use_decoy_ && (*(*isdecoy_inds_)[charge])[index]) {
    return 0.0;
  }

  posprob *= tmppos;
  negprob *= tmpneg;
 
  tmppos = 1; tmpneg = 1;

  
  if (pI_ptr_ != NULL) { 
    tmppos *=  pI_ptr_->getPosProb((*(*inds_)[charge])[index]);
    tmpneg *= pI_ptr_->getNegProb((*(*inds_)[charge])[index]);  
    if (tmppos > tmpneg) {
      tmppos = tmppos > tmpneg * maxPIrwd ? tmpneg * maxPIrwd : tmppos;
    }

  }

  posprob *= tmppos;
  negprob *= tmpneg;

  tmppos = 1; tmpneg = 1;

  
  if (RT_ && RT_ptr_ != NULL) { 
    tmppos *=  RT_ptr_->getPosProb((*(*inds_)[charge])[index]);
    tmpneg *= RT_ptr_->getNegProb((*(*inds_)[charge])[index]);  
    if (tmppos > tmpneg) {
      tmppos = tmppos > tmpneg * maxRTrwd ? tmpneg * maxRTrwd : tmppos;
    }

  }
  
  posprob *= tmppos;
  negprob *= tmpneg;
  

  if(posprob + negprob <= 0.0) {
    return 0.0;
  }  
  double prob = (posprob / (posprob + negprob));
  if (!isfinite(prob)) { // BSP - catch INF, NAN, -INF
	  return 0.0;
  }
 
  // HENRY
  if (strcasecmp(search_engine_, "SPECTRAST") == 0) {
    if (multiply_by_spectrast_lib_probs_) {
      return ((*(*libprobs_)[charge])[index] * prob);
    }
  }
  // END HENRY
  
  return prob;
}

// this must be modified for maldi case (must substitute DiscrimValMixtureDistr values for positive as well)
double MixtureModel::computeProbWithNTT(int charge, int orig_ntt, double orig_prob, int ntt, int index = -1) {
  assert(charge < probs_->size());

  if ( ignore_[charge] ) {
    return 0.0;
  }

  if(orig_ntt == ntt || orig_prob <= 0.0 || (negOnly_[charge] && !forcedistr_) ) {
    return orig_prob;
  }

  double posntt1 = ((NTTMixtureDistr*)(getMixtureDistr(ntt_name_, charge)))->getNTTPosFraction(orig_ntt);  
  double negntt1 = ((NTTMixtureDistr*)(getMixtureDistr(ntt_name_, charge)))->getNTTNegFraction(orig_ntt); 
  if(maldi_) {
    posntt1 *= ((DecayContinuousMultimixtureDistr*)(((DiscrimValMixtureDistr*)(getMixtureDistr(discrim_name_, charge)))->getPosDistr()))->getMixtureProbWithNTT(orig_ntt, index);
    negntt1 *= ((DiscrimValMixtureDistr*)(getMixtureDistr(discrim_name_, charge)))->getNegProb(index);
  } // maldi case
  double ratio1;
  if(posntt1 == 0.0) {
    ratio1 = 999.0;
  }
  else {
    ratio1 = negntt1 / posntt1;
  }
  double posntt2 = ((NTTMixtureDistr*)(getMixtureDistr(ntt_name_, charge)))->getNTTPosFraction(ntt);  
  double negntt2 = ((NTTMixtureDistr*)(getMixtureDistr(ntt_name_, charge)))->getNTTNegFraction(ntt);  
  if(maldi_) {
    posntt2 *= ((DecayContinuousMultimixtureDistr*)(((DiscrimValMixtureDistr*)(getMixtureDistr(discrim_name_, charge)))->getPosDistr()))->getMixtureProbWithNTT(ntt, index);
    negntt2 *= ((DiscrimValMixtureDistr*)(getMixtureDistr(discrim_name_, charge)))->getNegProb(index);
  } // maldi case
  double ratio2;
  if(posntt2 == 0.0 && negntt2 == 0.0) {
    return 0.0;
  }

  if(posntt2 == 0.0) {
    ratio2 = 999.0;
  }
  else {
    ratio2 = negntt2 / posntt2;
  }

  double denom;
  if(ratio1 == 0.0) {
    denom = 999.0;
  }
  else {
    denom  = ((1.0 - orig_prob) * ratio2 / (orig_prob * ratio1)) + 1.0;
  }
  return 1 / denom;
}

double MixtureModel::getProb(int charge, int index) {
  return (*(*probs_)[charge])[index];
}


Boolean MixtureModel::enterDistribution(int charge, int index, char* tag, char* value) {
  Boolean output = False;
  for(int k = 0; k < (*mixtureDistrs_)[charge]->size(); k++) {
    if(strcmp((*(*mixtureDistrs_)[charge])[k]->getTag(), tag) == 0) {
      //cout << "found mx distr " << k << " for " << name << endl;
      (*(*mixtureDistrs_)[charge])[k]->enter(index, value);
      output = True;
    }
  }
  return output;
}

char* MixtureModel::getTagValue(const char* data, const char*tag) {
  char* output = NULL;
  const char* result = strstr(data, tag);
  if(result == NULL)
    return output;
  int k = (int)strlen(tag);
  while(k < (int) strlen(result) && result[k] != ' ' && result[k] != '\t' &&
	result[k] != '\n')
    k++;
  output = new char[k - strlen(tag) + 1];
  strncpy(output, result + strlen(tag), k - strlen(tag));
  output[k - strlen(tag)] = 0;
  return output;
}


void MixtureModel::readData(istream& is) {
  char tag[75];
  char value[100];
  char spec[2000];
  int charge=-1;
  int index = 0; //-1;
  char* nextspec = NULL;
  const int datasize = 300;
  char data[datasize];
  if(! is.getline(data, datasize)) {
    cerr <<  "cannot read data" << endl;
    exit(1);
  }
      
  while(is >> tag) {

      if(strcmp(tag, "spec") == 0) {
	is >> spec;
	nextspec = new char[strlen(spec)+1];
	strcpy(nextspec, spec);
	nextspec[strlen(spec)] = 0;
      }
      else if(strcmp(tag, "charge") == 0) {
	is >> value;
	charge = atoi(value) - 1;
	(*spectra_)[charge]->insertAtEnd(nextspec);
	(*probs_)[charge]->insertAtEnd(0.5);
	(*inds_)[charge]->insertAtEnd(index++);
      }
      else {
	is >> value;
	enterDistribution(charge, index, tag, value);
      }

  } // next entry

  cerr << " read in " << (*spectra_)[0]->size() << " 1+, " << (*spectra_)[1]->size() << " 2+, and " << (*spectra_)[2]->size() << " 3+ spectra" << endl;


}

Boolean MixtureModel::isOrderedSpectrum(char* spectrum, int charge, int index) {

  if(index >= num_ordered_) {
    return False;
    //cout << "error in isOrderedSpectrum, " << index << " exceeds " << num_ordered_ << endl;
    //exit(1);
  }

  /*
  if (strstr( (*(*spectra_)[ordered_[index].charge_])[ordered_[index].index_],"022008_F8_ETD_1.03159.03159" ) != NULL ) {
    cerr <<"DDS: DEBUG"<<endl;
  }  
  if (strstr( spectrum, "022008_F8_ETD_1.03159.03159" ) != NULL ) {
    cerr <<"DDS: DEBUG"<<endl;
  }  

  if (strstr( spectrum, (*(*spectra_)[ordered_[index].charge_])[ordered_[index].index_] ) == NULL ) {
    cerr <<"DDS: DEBUG"<<endl;
  }  
  */

  if (rep_specs_ == NULL) {
    findRepSpecs();
  }


  char* spec = NULL;
  Boolean modify_spec = False;

  // if spec does not end in '.x' where x is charge, add on this terminus

  // HENRY: allow for spectrum name ending in .0 for SpectraST
  //  if(strlen(spectrum) > 2 && spectrum[strlen(spectrum)-2] == '.' && (spectrum[strlen(spectrum)-1] == charge + '0' || spectrum[strlen(spectrum)-1] == '0'))

  //  spec = spectrum; // use as is

  // else {
  spec = new char[strlen(spectrum)+3];
  strcpy(spec, spectrum);
  //if (spec[strlen(spec)-1] != charge + '0') {

  char* lastdot = strrchr(spec, '.');
  *lastdot = 0;
  //    spec[strlen(spec)-2] = 0;
    //}
    //int len = (int)strlen(spec);
    //spec[len] = '.';
    //spec[len+1] = charge + '0';
    //spec[len+2] = 0;
  modify_spec = True;
    //}

  SpectHash::const_iterator z = rep_specs_->find(spec);
  Boolean result = False;

  if(z != rep_specs_->end() &&  use_adj_probs_ && ordered_[index].charge_ >= 0 &&  ordered_[index].charge_ < MAX_CHARGE) {
    
    //Boolean result = ! strcmp(spec, pairedspecs_[ordered_[index].index_].name_);
    for (int i=0; i < (*(*z).second).size(); i++) {
      result = ! strcmp(spectrum, (*(*(*z).second)[i]).name_) ;
      if (result) break;
      
    }
    if(modify_spec)
      delete [] spec;
    return result;
  }

  result = ! strncmp(spectrum, (*(*spectra_)[ordered_[index].charge_])[ordered_[index].index_], strlen((*(*spectra_)[ordered_[index].charge_])[ordered_[index].index_])) && spectrum[strlen(spectrum)-1] != ordered_[index].charge_ - 47;
  
  if(modify_spec && spec != NULL)
    delete [] spec;
  return result;

}

double MixtureModel::getOrderedProb(int index) {
  if(index >= num_ordered_ ) {
    cout << "error in getOrderedProb" << endl;
    cout << index << " exceeds " << num_ordered_ << endl;
    exit(1);
  }
  if (rep_specs_ == NULL) {
    findRepSpecs();
  }
  SpectHash::const_iterator z = rep_specs_->find((*(*spectra_)[ordered_[index].charge_])[ordered_[index].index_]);
  
  // if (!strcmp("20100422_01_control_04.07416.07416", (*(*spectra_)[ordered_[index].charge_])[ordered_[index].index_]) ) {
  //    cerr << "DDS: DEBUG" << endl;
  //}
  
  if (ignore_[ordered_[index].charge_] || (negOnly_[ordered_[index].charge_] && !forcedistr_)  ){
    return 0;
  }
  
  if( z != rep_specs_->end() && use_adj_probs_ && ordered_[index].charge_ >= 0 && ordered_[index].charge_ < MAX_CHARGE) {
    //return pairedspecs_[ordered_[index].index_].prob_;
    for (int i=0; i < (*(*z).second).size(); i++) {
      if (ordered_[index].charge_ == (*(*(*z).second)[i]).charge_) {
	return (*(*probs_)[(*z->second)[i]->charge_])[(*z->second)[i]->data_index_];

      }
    }
  }
   
  return (*(*probs_)[ordered_[index].charge_])[ordered_[index].index_];
}

double MixtureModel::getOrderedProb(int run_idx, int index) {
  
  return this->getOrderedProb((*(*run_inds_)[run_idx])[index]);
} 

int MixtureModel::getOrderedNTT(int run_idx, int index) {
  int idx = (*(*run_inds_)[run_idx])[index];
  return ((NTTMixtureDistr*)getMixtureDistr(ntt_name_, ordered_[idx].charge_))->getNTTValue(ordered_[idx].index_);
}

//double MixtureModel::getOrderedProb(int run_idx, int charge, int index) {
  
//  return this->getOrderedProb((*(*run_chrg_inds_)[run_idx])[index]);
//}


  
Array<Tag*>* MixtureModel::getModelSummaryTags(const char* prog_name, const char* time, const Array<char*>* inputfiles, double minprob, const char* options) {
  Array<Tag*>* output = new Array<Tag*>;
  Array<Tag*>* next = NULL;
  Tag* nexttag = NULL;
  char text[1000];

  nexttag = new Tag("analysis_summary", True, False);
  nexttag->setAttributeValue("analysis", prog_name);
  nexttag->setAttributeValue("time", time);
  output->insertAtEnd(nexttag);

  nexttag = new Tag("peptideprophet_summary", True, False);
  snprintf(text,sizeof(text),"%s (%s)",PROGRAM_VERSION,szTPPVersionInfo);
  nexttag->setAttributeValue("version", text);
  nexttag->setAttributeValue("author", PROGRAM_AUTHOR);

  sprintf(text, "%0.2f", minprob);
  nexttag->setAttributeValue("min_prob", text);
  if(strlen(options) > 0)
    nexttag->setAttributeValue("options", options);

  sprintf(text, "%0.1f", getTotalAdjProb(0)+getTotalAdjProb(1)+getTotalAdjProb(2)+getTotalAdjProb(3)+getTotalAdjProb(4)+getTotalAdjProb(5)+getTotalAdjProb(6));
  nexttag->setAttributeValue("est_tot_num_correct", text);
  output->insertAtEnd(nexttag);

  
  // handle inputfiles
  for(int k = 0; k < inputfiles->size(); k++) {
    nexttag = new Tag("inputfile", True, True);

#define MAX_DIRPATH_LEN 4096

    if (!isAbsolutePath((*inputfiles)[k])) {
      char* curr_dir = new char[MAX_DIRPATH_LEN];
      safepath_getcwd(curr_dir,  MAX_DIRPATH_LEN-1);
      nexttag->setAttributeValue("directory", curr_dir);
      delete [] curr_dir;
    }
    
    if (strstr((*inputfiles)[k], get_pepxml_dot_ext()) == NULL) {
      if (strstr((*inputfiles)[k], ".pep.xml") != NULL || strstri((*inputfiles)[k], ".pepxml") != NULL ) {
	cerr << "WARNING: msms_run_summary base_name " << (*inputfiles)[k] << " contains the pepXML extension, paths may be broken ...  " << endl;
      }
      else {
	strcat((*inputfiles)[k],get_pepxml_dot_ext()); 
      }
    }

    nexttag->setAttributeValue("name", (*inputfiles)[k]);
    
    output->insertAtEnd(nexttag);
  }

  	//generate info for charge specific error and roc curves RBS
	for (int i = 0; i <= MAX_CHARGE; i++)
	{	
		next = getRocDataPointTags(i); //generate tag group for each charge state, when i = 0 all charge states are taken into account
		if(next != NULL)
		{
			for(int j = 0; j != next->size(); j++)
				output->insertAtEnd((*next)[j]);
			delete next;
		}
	}

  // here distribution points
  next = getDiscrimValDistrTags();
  if(next != NULL) {
    for(int i = 0; i < next->size(); i++) {
      output->insertAtEnd((*next)[i]);
    }
    delete next;
  }

  for(int charge = 0; charge < MAX_CHARGE; charge++) {
    if (ignore_[charge]) {
      continue;
    }
    nexttag = new Tag("mixture_model", True, False);
    sprintf(text, "%d", charge+1);
    nexttag->setAttributeValue("precursor_ion_charge", text);
    if(ignore_[charge] || (negOnly_[charge] && !forcedistr_) ) {
      if(getNegOnlyCharge(charge) < 0)
	sprintf(text, "using training data positive distributions to identify candidates ('-%d') above background ('0')", charge+1);
      else 
	sprintf(text, "using %d+ positive distributions to identify candidates ('-%d') above background ('0')", getNegOnlyCharge(charge)+1, charge+1);
    }
    else if(pseudonegs_[charge]) {
      sprintf(text, "using %s 0 data as pseudonegatives", ntt_name_);
    }
    else {
      sprintf(text, "using training data negative distributions");
    }
    nexttag->setAttributeValue("comments", text);
    sprintf(text, "%0.3f", priors_[charge]);
    nexttag->setAttributeValue("prior_probability", text);
    sprintf(text, "%0.1f", getTotalAdjProb(charge));
    nexttag->setAttributeValue("est_tot_correct", text);
    sprintf(text, "%d", (*spectra_)[charge]->size());
    nexttag->setAttributeValue("tot_num_spectra", text);
    sprintf(text, "%d", totitrs_[charge]);
    nexttag->setAttributeValue("num_iterations", text);

    output->insertAtEnd(nexttag);
    for(int k = 0; k < (*mixtureDistrs_)[charge]->size(); k++) {
      next = (*(*mixtureDistrs_)[charge])[k]->getMixtureDistrTags(NULL);
      if(next != NULL) {
	for(int i = 0; i < next->size(); i++) {
	  output->insertAtEnd((*next)[i]);
	}
	delete next;
      }
    } // next model
    output->insertAtEnd(new Tag("mixture_model", False, True));
  } // next charge
  output->insertAtEnd(new Tag("peptideprophet_summary", False, True));
  nexttag = new Tag("analysis_summary", False, True);
  output->insertAtEnd(nexttag);

  return output;
}


Array<Tag*>* MixtureModel::getOrderedProbTags(const char* prog_name, int index) {
  allchg_index_ = index;
  if(index >= num_ordered_) {
    cout << "error in getOrderedProbTags" << endl;
    cout << index << " exceeds " << num_ordered_ << endl;
    exit(1);
  }

  Boolean verbose = False; //index == 66;

  Array<Tag*>* output = new Array<Tag*>;

  Tag* nexttag = NULL;
  nexttag = new Tag("analysis_result", True, False);
  nexttag->setAttributeValue("analysis", prog_name);
  output->insertAtEnd(nexttag);

  nexttag = new Tag("peptideprophet_result", True, False);
  char next[500];
  double prob = getOrderedProb(index);


  SpectHash::const_iterator z;
   int i;

  if (ignore_[ordered_[index].charge_] || (negOnly_[ordered_[index].charge_] && !forcedistr_) ) {
    sprintf(next, "%0.0f", prob);
    nexttag->setAttributeValue("probability", next);
    sprintf(next, "(%0.0f,%0.0f,%0.0f)", prob, prob, prob);
    nexttag->setAttributeValue("all_ntt_prob", next);
    nexttag->setAttributeValue("analysis", "incomplete");
  }
  else if(rep_specs_ != NULL && ordered_[index].charge_ < MAX_CHARGE && ordered_[index].charge_ >= 0 && use_adj_probs_) {
    sprintf(next, "%0.4f", prob);
    nexttag->setAttributeValue("probability", next);
    if (no_ntt_) {
      sprintf(next, "(%0.4f,%0.4f,%0.4f)", prob, prob, prob);
    }
    else {
      
      //      if (!strcmp("20100422_01_control_04.07416.07416", (*(*spectra_)[ordered_[index].charge_])[ordered_[index].index_]) ) {
      //	cerr << "DDS: DEBUG" << endl;
      //}

      z = rep_specs_->find((*(*spectra_)[ordered_[index].charge_])[ordered_[index].index_]);
        
      if(z != rep_specs_->end() &&  use_adj_probs_ && ordered_[index].charge_ >= 0 &&  ordered_[index].charge_ < MAX_CHARGE) {
	
	//Boolean result = ! strcmp(spec, pairedspecs_[ordered_[index].index_].name_);
	for (i=0; i < (*(*z).second).size(); i++) {
	  if ( ordered_[index].charge_ == (*(*(*z).second)[i]).charge_ ) break;
	}
    }

       //DDS: ASSUME its always in the hash

      //sprintf(next, "(%0.4f,%0.4f,%0.4f)", computeProbWithNTT(pairedspecs_[ordered_[index].index_].charge_, pairedspecs_[ordered_[index].index_].ntt_, pairedspecs_[ordered_[index].index_].prob_, 0, ordered_[index].index_),computeProbWithNTT(pairedspecs_[ordered_[index].index_].charge_, pairedspecs_[ordered_[index].index_].ntt_, pairedspecs_[ordered_[index].index_].prob_, 1, ordered_[index].index_),computeProbWithNTT(pairedspecs_[ordered_[index].index_].charge_, pairedspecs_[ordered_[index].index_].ntt_, pairedspecs_[ordered_[index].index_].prob_, 2, ordered_[index].index_));
      
      sprintf(next, "(%0.4f,%0.4f,%0.4f)", computeProbWithNTT((*(*(*z).second)[i]).charge_, (*(*(*z).second)[i]).ntt_, (*(*probs_)[(*z->second)[i]->charge_])[(*z->second)[i]->data_index_], 0, ordered_[index].index_),computeProbWithNTT((*(*(*z).second)[i]).charge_, (*(*(*z).second)[i]).ntt_, (*(*probs_)[(*z->second)[i]->charge_])[(*z->second)[i]->data_index_], 1, ordered_[index].index_),computeProbWithNTT((*(*(*z).second)[i]).charge_, (*(*(*z).second)[i]).ntt_, (*(*probs_)[(*z->second)[i]->charge_])[(*z->second)[i]->data_index_], 2, ordered_[index].index_));
    }

    nexttag->setAttributeValue("all_ntt_prob", next);
    if(isAdjusted(index))
      nexttag->setAttributeValue("analysis", "adjusted");
  }
  else { // conventional
    if(! negOnly_[ordered_[index].charge_] && priors_[ordered_[index].charge_] == 0.0) { // definite 0
      nexttag->setAttributeValue("probability", "0");
    }
    else {
      if(ignore_[ordered_[index].charge_] || (negOnly_[ ordered_[index].charge_] && !forcedistr_) )
	sprintf(next, "%0.0f", prob);
      else
	sprintf(next, "%0.4f", prob);
      nexttag->setAttributeValue("probability", next);
    }
    
    if (no_ntt_ && (ignore_[ordered_[index].charge_] || (negOnly_[ ordered_[index].charge_] && !forcedistr_)) ) {
      sprintf(next, "(%0.0f,%0.0f,%0.0f)", prob, prob, prob);
    }
    else if (no_ntt_) {
      sprintf(next, "(%0.4f,%0.4f,%0.4f)", prob, prob, prob);
    }
    else if(ignore_[ordered_[index].charge_] || (negOnly_[ ordered_[index].charge_] && !forcedistr_))
      sprintf(next, "(%0.0f,%0.0f,%0.0f)", computeProbWithNTT(ordered_[index].charge_, ((NTTMixtureDistr*)(getMixtureDistr(ntt_name_, ordered_[index].charge_)))->getNTTValue(ordered_[index].index_), (*(*probs_)[ordered_[index].charge_])[ordered_[index].index_], 0, ordered_[index].index_), computeProbWithNTT(ordered_[index].charge_, ((NTTMixtureDistr*)(getMixtureDistr(ntt_name_, ordered_[index].charge_)))->getNTTValue(ordered_[index].index_), (*(*probs_)[ordered_[index].charge_])[ordered_[index].index_], 1, ordered_[index].index_), computeProbWithNTT(ordered_[index].charge_, ((NTTMixtureDistr*)(getMixtureDistr(ntt_name_, ordered_[index].charge_)))->getNTTValue(ordered_[index].index_), (*(*probs_)[ordered_[index].charge_])[ordered_[index].index_], 2, ordered_[index].index_));
    else
      sprintf(next, "(%0.4f,%0.4f,%0.4f)", computeProbWithNTT(ordered_[index].charge_, ((NTTMixtureDistr*)(getMixtureDistr(ntt_name_, ordered_[index].charge_)))->getNTTValue(ordered_[index].index_), (*(*probs_)[ordered_[index].charge_])[ordered_[index].index_], 0, ordered_[index].index_), computeProbWithNTT(ordered_[index].charge_, ((NTTMixtureDistr*)(getMixtureDistr(ntt_name_, ordered_[index].charge_)))->getNTTValue(ordered_[index].index_), (*(*probs_)[ordered_[index].charge_])[ordered_[index].index_], 1, ordered_[index].index_), computeProbWithNTT(ordered_[index].charge_, ((NTTMixtureDistr*)(getMixtureDistr(ntt_name_, ordered_[index].charge_)))->getNTTValue(ordered_[index].index_), (*(*probs_)[ordered_[index].charge_])[ordered_[index].index_], 2, ordered_[index].index_));
    


    nexttag->setAttributeValue("all_ntt_prob", next);
    if(ignore_[ordered_[index].charge_] || (negOnly_[ordered_[index].charge_] && !forcedistr_)) 
      nexttag->setAttributeValue("analysis", "incomplete");
    else if(ordered_[index].charge_ < MAX_CHARGE && ordered_[index].charge_ >= 0 && isAdjusted(ordered_[index].index_))
      nexttag->setAttributeValue("analysis", "adjusted");
  } // conventional

  output->insertAtEnd(nexttag);
  Array<Tag*>* scoretags = getOrderedProbScoreTags(ordered_[index].charge_, getOrderedDataIndex(index));
  for(int s = 0; s < scoretags->size(); s++)
    output->insertAtEnd((*scoretags)[s]);
  delete scoretags;
  output->insertAtEnd(new Tag("peptideprophet_result", False, True));

  if(verbose) {
    cout << "terminating order prob tags with " << output->size() << " tags" << endl;
    for(int k = 0; k < output->size(); k++)
      (*output)[k]->write(cout);
  }
  nexttag = new Tag("analysis_result", False, True);
  output->insertAtEnd(nexttag);

  return output; // done
  
}

int MixtureModel::getOrderedDataIndex(int index) {
  if(rep_specs_ != NULL && use_adj_probs_ && ordered_[index].charge_ >= 0 && ordered_[index].charge_ < MAX_CHARGE) {
    SpectHash::const_iterator z = rep_specs_->find((*(*spectra_)[ordered_[index].charge_])[ordered_[index].index_]);
    int i;
    if(z != rep_specs_->end() &&  use_adj_probs_ && ordered_[index].charge_ >= 0 &&  ordered_[index].charge_ < MAX_CHARGE) {
      
      for (i=0; i < (*(*z).second).size(); i++) {
	if (ordered_[index].charge_ == (*(*(*z).second)[i]).charge_ ) return (*(*(*z).second)[i]).data_index_;
      }
      
    }
  }
  return ordered_[index].index_;
}



Array<Tag*>* MixtureModel::getOrderedProbScoreTags(int charge, int index) {
  Array<Tag*>* output = new Array<Tag*>;
  Boolean allchg_idx_incr = False;
  Tag* nexttag = new Tag("search_score_summary", True, False);
  output->insertAtEnd(nexttag);
  for(int k = 0; k < (*mixtureDistrs_)[charge]->size(); k++) 
    if (!RT_ && strcmp((*(*mixtureDistrs_)[charge])[k]->getTag(), "RT")==0) {
      continue;
    }
    else if(index < (*(*mixtureDistrs_)[charge])[k]->getNumVals()) {
      nexttag = new Tag("parameter", True, True);
      nexttag->setAttributeValue("name", (*(*mixtureDistrs_)[charge])[k]->getTag());
      if (strcmp((*(*mixtureDistrs_)[charge])[k]->getTag(), "pI")==0) {
	char *valstr;
	nexttag->setAttributeValue("value", valstr=((KernelDensityPIMixtureDistr*)((*(*mixtureDistrs_)[charge])[k]))->getStringpIValue(allchg_index_));
	delete[] valstr;
	output->insertAtEnd(nexttag);
	nexttag = new Tag("parameter", True, True);
	nexttag->setAttributeValue("name", "pI_zscore");
	nexttag->setAttributeValue("value", valstr=(*(*mixtureDistrs_)[charge])[k]->getStringValue(allchg_index_));
	delete[] valstr;
	output->insertAtEnd(nexttag);
	allchg_idx_incr = True;
      }
      else if (RT_ && strcmp((*(*mixtureDistrs_)[charge])[k]->getTag(), "RT")==0) {
	nexttag->setAttributeValue("value", ((KernelDensityRTMixtureDistr*)((*(*mixtureDistrs_)[charge])[k]))->getStringRTValue(allchg_index_));
	output->insertAtEnd(nexttag);
	nexttag = new Tag("parameter", True, True);
	nexttag->setAttributeValue("name", "RT_score");
	nexttag->setAttributeValue("value", (*(*mixtureDistrs_)[charge])[k]->getStringValue(allchg_index_));
	output->insertAtEnd(nexttag);
	allchg_idx_incr = True;
      }
      else if (accMass_ && strcmp((*(*mixtureDistrs_)[charge])[k]->getName(), "accurate mass diff [massd]")==0) {
	char* value = (*(*mixtureDistrs_)[charge])[k]->getStringValue(allchg_index_);
	nexttag->setAttributeValue("value", value);
	delete[] value; // setAttributeValue creates a copy of the value string -- delete the one we created
	output->insertAtEnd(nexttag);  
      }
      else {
	char* value = (*(*mixtureDistrs_)[charge])[k]->getStringValue(index);
	nexttag->setAttributeValue("value", value);
	delete[] value; // setAttributeValue creates a copy of the value string -- delete the one we created
	output->insertAtEnd(nexttag);  
      }
      
    }
    else {
      cout << "charge " << (charge+1) << " index " << index << " vs " << (*(*mixtureDistrs_)[charge])[k]->getNumVals() << endl;
    }
  
  output->insertAtEnd(new Tag("search_score_summary", False, True));
  
  allchg_index_++;
  
  return output;
}

void MixtureModel::initResultsInOrder() {
  int ordered_ind = 0;
  num_ordered_ = 0;
  for (int charge=MAX_CHARGE;charge--;) {
    num_ordered_ += (*spectra_)[charge]->size();
  }
  ordered_ = new OrderedResult[num_ordered_];
  for(int ch = 0; ch < MAX_CHARGE; ch++) {
      for(int k = 0; k < (*spectra_)[ch]->size(); k++) {
	if (use_decoy_) {
	  ordered_[ordered_ind++].init(ch, k, (*(*inds_)[ch])[k], (*(*isdecoy_inds_)[ch])[k]);
	}
	else {
	  ordered_[ordered_ind++].init(ch, k, (*(*inds_)[ch])[k]);
	}
      }
  }

  // now order them
  qsort(ordered_, num_ordered_, sizeof(OrderedResult), (int(*)(const void*, const void*)) comp_ords);


}

void MixtureModel::setResultsInOrder() {
  if(use_adj_probs_ && rep_specs_ == NULL) {
    //    computeAdjDoubleTriplySpectraProbs();
    computeAdjMultiChargeSpectraProbs();
  }

  int ordered_ind = 0;
  int charge;
  int total = 0;
  for (charge=MAX_CHARGE;charge--;) {
    total += (*spectra_)[charge]->size();
  }
  //total = rep_specs_->size();
  if ((num_ordered_ != total) || !ordered_) {
	num_ordered_ = total;
	delete[] ordered_; // don't leak - may have been set by initResultsInOrder
	ordered_ = new OrderedResult[total]; 
  }
  // first the 1+
  // for(int k = 0; k < (*spectra_)[0]->size(); k++) {
  // ordered_[ordered_ind++].init(0, k, (*(*inds_)[0])[k]); 
  //}
  //if(use_adj_probs_) {
  //  for(int k = 0; k < num_pairs_; k++) {
  //    ordered_[ordered_ind++].init(pairedspecs_[k].charge_, k, pairedspecs_[k].ind_);
  //  }
  //  for(int ch = 3; ch < MAX_CHARGE; ch++) {
  //    for(int k = 0; k < (*spectra_)[ch]->size(); k++) {
  //      ordered_[ordered_ind++].init(ch, k, (*(*inds_)[ch])[k]);
  //    }
  //  }
  // }
  //else {

  for(int ch = 0; ch < MAX_CHARGE; ch++) {
    for(int k = 0; k < (*spectra_)[ch]->size(); k++) {
      if (use_decoy_) {
	ordered_[ordered_ind++].init(ch, k, (*(*inds_)[ch])[k], (*(*isdecoy_inds_)[ch])[k]);
      }
      else {
	ordered_[ordered_ind++].init(ch, k, (*(*inds_)[ch])[k]);
      }
    }
  }
  //  for(SpectHash::const_iterator z = rep_specs_->begin(); z != rep_specs_->end(); z++) {
  //  int ch = (*z->second)[0]->charge_;	
  //  int data_idx = (*z->second)[0]->data_index_;	
  //  int idx = (*z->second)[0]->ind_;	
  //  ordered_[ordered_ind++].init(ch, idx, data_idx);
  // }

  //}
  // now order them
  qsort(ordered_, total, sizeof(OrderedResult), (int(*)(const void*, const void*)) comp_ords);

}

void MixtureModel::writeResultsInOrder(char* filename) {
  if(use_adj_probs_ && rep_specs_ == NULL) {
    //    computeAdjDoubleTriplySpectraProbs();
    computeAdjMultiChargeSpectraProbs();
  }

  int ordered_ind = 0;
  int total = 0;
  for (int ch=MAX_CHARGE;ch--;) {
	  total += (*spectra_)[ch]->size();
  }
  OrderedResult* ordered = new OrderedResult[total];
  // first the 1+
  //int k;
  //for(k = 0; k < (*spectra_)[0]->size(); k++) {
  //  ordered[ordered_ind++].init(0, k, (*(*inds_)[0])[k]); 
  //}
  // and the 4+, 5+, ...
  //for(int ch = 3; ch < MAX_CHARGE; ch++) {
  //  for(int k = 0; k < (*spectra_)[ch]->size(); k++) {
  //   ordered[ordered_ind++].init(ch, k, (*(*inds_)[ch])[k]);
  // }
  //}
  //// and the 2+, 3+
  //if(use_adj_probs_) {
  //  for(int k = 0; k < num_pairs_; k++) {
  //    ordered[ordered_ind++].init(pairedspecs_[k].charge_, k, pairedspecs_[k].ind_); 
  //  }
  //}
  //else {
  // for(int ch = 0; ch < MAX_CHARGE; ch++) {  // BSP was "ch<MAX_CHARGE"
  //    for(int k = 0; k < (*spectra_)[ch]->size(); k++) {
  //      ordered[ordered_ind++].init(ch, k, (*(*inds_)[ch])[k]);
  //    }
  //  }
    //  }

  for(SpectHash::const_iterator z = rep_specs_->begin(); z != rep_specs_->end(); z++) {
    int ch = (*z->second)[0]->charge_;	
    int data_idx = (*z->second)[0]->data_index_;	
    int idx = (*z->second)[0]->ind_;	
    if (use_decoy_) {
      ordered_[ordered_ind++].init(ch, idx, data_idx, (*(*isdecoy_inds_)[ch])[idx]);
    }
    else {
      ordered_[ordered_ind++].init(ch, idx, data_idx);
    }
  
  }

  // now order them
  qsort(ordered, total, sizeof(OrderedResult), (int(*)(const void*, const void*)) comp_ords);

  ofstream fout(filename);
  if(! fout) {
    cerr << "could not open filename" << endl;
    exit(1);
  }

  int k;
  for(k = 0; k < total; k++) {


      int ch = ordered[k].charge_;
      fout << (*(*spectra_)[ch])[ordered[k].index_];
      if(! maldi_)
	fout << "." << 1;
      fout << "\t";
      if(! negOnly_[ch] && priors_[ch] == 0.0) { // definite 0
	fout << "0\tnodata";
      }
      else {
	fout << (*(*probs_)[ch])[ordered[k].index_];
      }
      fout <<  "\t(";
      for(int n = 0; n < 3; n++) {
	if (no_ntt_) {
	  fout << (*(*probs_)[ch])[ordered[k].index_];
	}
	else {
	  fout << computeProbWithNTT(0, ((NTTMixtureDistr*)(getMixtureDistr(ntt_name_, 0)))->getNTTValue(ordered[k].index_), (*(*probs_)[ch])[ordered[k].index_], n, ordered[k].index_);
	}
	if(n < 2) {
	  fout << ",";
	}
      }
      fout << ")";
      if(ignore_[ch] || (negOnly_[ch] && !forcedistr_)) {
	fout << "\tnegonly";
      }
      fout << "\n";
  }


  // next result to write
  fout.close();
}

// write out probabilities to output file (input to mixture_aft.pl)
void MixtureModel::writeResults(char* filename) {
  if(use_adj_probs_ && rep_specs_ == NULL) {
    //    computeAdjDoubleTriplySpectraProbs();
    computeAdjMultiChargeSpectraProbs();
  }

  ofstream fout(filename);
  if(! fout) {
    cerr << "could not open filename" << endl;
    exit(1);
  }

  // first the 1+
  for(int k = 0; k < (*spectra_)[0]->size(); k++) {
    fout << (*(*spectra_)[0])[k] << "." << 1 << "\t";
    if(! negOnly_[0] && priors_[0] == 0.0) { // definite 0
      fout << "0\tnodata";
    }
    else {
      fout << (*(*probs_)[0])[k];
    }
    fout <<  "\t(";
    for(int n = 0; n < 3; n++) {
      if (no_ntt_) {
	fout << (*(*probs_)[0])[k];
      }
      else {
	fout << computeProbWithNTT(0, ((NTTMixtureDistr*)(getMixtureDistr(ntt_name_, 0)))->getNTTValue(k), (*(*probs_)[0])[k], n, k);
      }
      if(n < 2) {
	fout << ",";
      }
    }
    fout << ")";
    if(ignore_[0] || (negOnly_[0] && !forcedistr_)) {
      fout << "\tnegonly";
    }
    fout << "\n";
  }

  if(use_adj_probs_) {
    for(int k = 0; k < num_pairs_; k++) {
      fout << pairedspecs_[k].name_ << "\t" << pairedspecs_[k].prob_;
      fout << "\t(";
      for(int n = 0; n < 3; n++) {
	 if (no_ntt_) {
	   fout << pairedspecs_[k].prob_;
	 }
	 else {
	   fout << computeProbWithNTT(
				      pairedspecs_[k].charge_, 
				      pairedspecs_[k].ntt_,
				      pairedspecs_[k].prob_, n, pairedspecs_[k].ind_);
	 }
	if(n < 2) {
	  fout << ",";
	}
      }
      fout << ")";
      if(isAdjusted(k)) {
	fout << "\tadj";
      }
      fout << endl;
    }
  
    for(int charge = 3; charge < MAX_CHARGE; charge++) {
      for(int k = 0; k < (*spectra_)[charge]->size(); k++) {
	fout << (*(*spectra_)[charge])[k] << "." << (charge+1) << "\t";
	if(! negOnly_[charge] && priors_[charge] == 0.0) { // definite 0
	  fout << "0\tnodata";
	}
	else {
	  fout << (*(*probs_)[charge])[k];
	}
	//}
	fout <<  "\t(";
	  for(int n = 0; n < 3; n++) {
	    if (no_ntt_) {
	      fout << (*(*probs_)[charge])[k];
	    }
	    else {
	      fout << computeProbWithNTT(charge, ((NTTMixtureDistr*)(getMixtureDistr(ntt_name_, charge)))->getNTTValue(k), (*(*probs_)[charge])[k], n, k);
	    }
	    if(n < 2) {
	      fout << ",";
	    }
	  }
	  fout << ")";
	  if(ignore_[charge] || (negOnly_[charge] && !forcedistr_)) {
	    fout << "\tnegonly";
	  }
	  fout << "\n";
      }
    } 
  }
  else {
    for(int charge = 1; charge < MAX_CHARGE; charge++) {
      for(int k = 0; k < (*spectra_)[charge]->size(); k++) {
	  fout << (*(*spectra_)[charge])[k] << "." << (charge+1) << "\t";
	  if(! negOnly_[charge] && priors_[charge] == 0.0) { // definite 0
	    fout << "0\tnodata";
	  }
	  else {
	    fout << (*(*probs_)[charge])[k];
	  }
	  //}
	  fout <<  "\t(";
	  for(int n = 0; n < 3; n++) {
	    if (no_ntt_) {
	      fout << (*(*probs_)[charge])[k];
	    }
	    else {
	      fout << computeProbWithNTT(charge, ((NTTMixtureDistr*)(getMixtureDistr(ntt_name_, charge)))->getNTTValue(k), (*(*probs_)[charge])[k], n, k);
	    }
	    if(n < 2) {
	      fout << ",";
	    }
	  }
	  fout << ")";
	  if(ignore_[charge] || (negOnly_[charge] && !forcedistr_)) {
	    fout << "\tnegonly";
	  }
	  fout << "\n";
      }
    } // next charge
  } // else
  fout.close();
}

/*
Boolean MixtureModel::isAdjusted(int index) {
  if(adjusted_ == NULL) {
    return False;
  }
  for(int k = 0; k < adjusted_->size(); k++) {
    if((*adjusted_)[k] == index) {
      return True;
    }
    else if((*adjusted_)[k] > index) {
      return False;
    }
  }
  return False;
}
*/

Boolean MixtureModel::isAdjusted(int index) {
  set<int>::const_iterator itr = adj_set_.find(index);
  
  if (itr != adj_set_.end()) {
    return True;
  }

  return False;
}

void MixtureModel::writeModelfile(char* modelfile, char* interact_command) {
  writeDistr(modelfile, interact_command);
  computeEstimatedSensAndError(modelfile);
  writeDiscrimValDistr(modelfile);
}


Array<Tag*>* MixtureModel::getDiscrimValDistrTags() {
  Array<Tag*>* output = new Array<Tag*>;
  Tag* tag = NULL;
  char text[500];
  char attr[100];

  // HENRY: get minval, maxval and window from the Fvalue distribution, instead of hard-coding it
  DiscrimValMixtureDistr* mixtureDistr = (DiscrimValMixtureDistr*)(getMixtureDistr(discrim_name_, 1)); // just pick any charge
  double minval = mixtureDistr->getMinVal();
  double maxval = mixtureDistr->getMaxVal();
  double window = mixtureDistr->getWindow();

  //  double minval = -5.0;
  //double maxval = 10.0;
  //double window = 0.2;

  // END HENRY

  int numwins = lrint((maxval - minval)/window);
  for(int k = 0; k < numwins; k++) {
    tag = new Tag("distribution_point", True, True);
    sprintf(text, "%0.2f", (k+0.5)*window + minval);
    tag->setAttributeValue("fvalue", text);

    for(int charge = 0; charge < MAX_CHARGE; charge++) {
      sprintf(text, "%d", ((DiscrimValMixtureDistr*)(getMixtureDistr(discrim_name_, charge)))->slice(k*window + minval, (k+1)*window + minval));
      if (strcmp(text, "nan") == 0)
        strcpy(text, "0.0");
      sprintf(attr, "obs_%d_distr", charge+1);
      tag->setAttributeValue(attr, text);
      
      sprintf(text, "%0.2f", ((DiscrimValMixtureDistr*)(getMixtureDistr(discrim_name_, charge)))->posSlice(k*window + minval, (k+1)*window + minval));
      if (strcmp(text, "nan") == 0)
        strcpy(text, "0.0");
      sprintf(attr, "model_%d_pos_distr", charge+1);
      tag->setAttributeValue(attr, text);

      sprintf(text, "%0.2f", ((DiscrimValMixtureDistr*)(getMixtureDistr(discrim_name_, charge)))->negSlice(k*window + minval, (k+1)*window + minval));
      if (strcmp(text, "nan") == 0)
        strcpy(text, "0.0");
      sprintf(attr, "model_%d_neg_distr", charge+1);
      tag->setAttributeValue(attr, text);

      if(charge < 2) {
      }
    } // next charge
    output->insertAtEnd(tag);
  }
  return output;

}

// model learned distribution summaries
void MixtureModel::writeDiscrimValDistr(char* filename) {
  double minval = -5.0;
  double maxval = 10.0;
  double window = 0.2;
  FILE* fout;
  if((fout = fopen(filename, "a")) == NULL) {
    cerr << "could not open " << filename << endl;
    exit(1);
  }
  fprintf(fout, "\n\nOBSERVED AND MODEL DISTRIBUTIONS\n");
  fprintf(fout, "#Fval\t1+ distr\t1+ pos\t1+neg\t2+ distr\t2+ pos\t2+ neg\t3+ distr\t3+ pos\t3+ neg\n");
  int numwins = lrint ((maxval - minval)/window);
  for(int k = 0; k < numwins; k++) {
    fprintf(fout, "%0.2f\t", (k+0.5)*window + minval);
    for(int charge = 0; charge < MAX_CHARGE; charge++) {
      fprintf(fout, "%d\t%0.2f\t%0.2f", ((DiscrimValMixtureDistr*)(getMixtureDistr(discrim_name_, charge)))->slice(k*window + minval, (k+1)*window + minval),((DiscrimValMixtureDistr*)(getMixtureDistr(discrim_name_, charge)))->posSlice(k*window + minval, (k+1)*window + minval), ((DiscrimValMixtureDistr*)(getMixtureDistr(discrim_name_, charge)))->negSlice(k*window + minval, (k+1)*window + minval));
      if(charge < 2) {
	fprintf(fout, "\t");
      }
    } // next charge
    fprintf(fout, "\n");
  }

  if(!no_ntt_ && ( maldi_ || qtof_ ) ) {
    fprintf(fout, "\n\nMODEL DISTRIBUTIONS BY NTT VALUE\n");
    if(maldi_)
    fprintf(fout, "#Fval\tntt0 1+ pos\tntt1 1+ pos\tntt2 1+ pos\n");
    else 
      fprintf(fout, "#Fval\tntt0 1+ pos\tntt0 2+ pos\tntt0 3+ pos\tntt1 1+ pos\tntt1 2+ pos\tntt1 3+ pos\tntt2 1+ pos\tntt2 2+ pos\tntt2 3+ pos\n");
    for(int k = 0; k < numwins; k++) {
      fprintf(fout, "%0.2f", (k+0.5)*window + minval);
      for(int ntt = 0; ntt < 3; ntt++) {
	for(int charge = 0; charge < MAX_CHARGE; charge++) {
	  if(qtof_ || (maldi_ && charge == 0))
	    fprintf(fout, "\t%0.2f", ((DiscrimValMixtureDistr*)(getMixtureDistr(discrim_name_, charge)))->posSliceWithNTT(k*window + minval, (k+1)*window + minval, ntt));
	} // next charge
      } // next ntt
      fprintf(fout, "\n");
    } // next win

  } // if maldi or qtof


  fclose(fout);
}

void MixtureModel::printDistr() {
  for(int charge = 0; charge < MAX_CHARGE; charge++) {
    printf("charge %d after %d iterations:\n", charge+1, done_[charge]);
    printf("\tprior: %0.3f\n", priors_[charge]);
    for(int k = 0; k < (*mixtureDistrs_)[charge]->size(); k++) {
      (*(*mixtureDistrs_)[charge])[k]->printDistr();
    }
    printf("\n\n");
  }
}

void MixtureModel::writeDistr(char* filename, char* interact_command) {
  FILE* fout;
  if((fout = fopen(filename, "w")) == NULL) {
    cerr << "cannot open " << filename << endl;
    exit(1);
  }
  // put date and time here
  time_t now;
  time(&now);
  fprintf(fout, "%s\n", ctime(&now));

  // here have min and max column width
  const int min_width = 75;
  int pos = 0;
  char* options = NULL;
  int start_options = -1;
  for(int k = 0; k < (int) strlen(interact_command); k++) {
    if(k < (int) strlen(interact_command)-3 && interact_command[k] == ' ' && 
       interact_command[k+1] == '-' && interact_command[k+2] == 'O')
      start_options = k+3;
    if(start_options > -1 && (k == (int) strlen(interact_command)-1 || interact_command[k+1] == ' ')) {
      options = new char[k+2 - start_options];
      strncpy(options, interact_command + start_options, k + 1 - start_options);
      options[k + 1 - start_options] = 0;
      start_options = -1; // done
    }

    pos++;
    if(pos >= min_width && interact_command[k] == ' ') {
      fprintf(fout, "\n\t"); // newline
      pos = 0; // reset
    }
    else {
      fprintf(fout, "%c", interact_command[k]);
    }
	    }
  fprintf(fout, "\n\n");
  // put explicit options here
  if(options != NULL) {
    PeptideProphetOptions* pepproph_options = new PeptideProphetOptions();
    fprintf(fout, "OPTIONS: ");
    for(int k = 0; k < (int) strlen(options); k++)
      fprintf(fout, "%s", pepproph_options->getOption(options[k]));
    if(pepproph_options != NULL)
      delete pepproph_options;
  }
  fprintf(fout, "\n\n\n");

  fprintf(fout, "%s\n\n\n", PEPTIDEPROPHET_VERSION);

  // here add options for run.....


  for(int charge = 0; charge < MAX_CHARGE; charge++) {
    fprintf(fout, "FINAL %d+ MODEL after %d iterations:\n", charge+1, done_[charge]);
    fprintf(fout, "number of spectra: %d\n", (*spectra_)[charge]->size());
    if(ignore_[charge] || (negOnly_[charge] && !forcedistr_) ) {
      if(getNegOnlyCharge(charge) < 0)
	fprintf(fout, "using training data positive distributions to identify candidates ('-%d') above background ('0')\n", charge+1);
      else 
	fprintf(fout, "using %d+ positive distributions to identify candidates ('-%d') above background ('0')\n", getNegOnlyCharge(charge)+1, charge+1);
    }
    else if(pseudonegs_[charge]) {
      fprintf(fout, "using %s 0 data as pseudonegatives\n", ntt_name_);
    }
    else {
      fprintf(fout, "using training data negative distributions\n");
    }

    fprintf(fout, "\tprior: %0.3f, est. total no. correct: %0.1f\n", priors_[charge], getTotalAdjProb(charge));
    for(int k = 0; k < (*mixtureDistrs_)[charge]->size(); k++) {
      (*(*mixtureDistrs_)[charge])[k]->writeDistr(fout);
    }
   fprintf(fout, "\n");
  }
  fclose(fout);
}


// add constraint that final probs for 2+ and 3+ precursor ion interpretations
// of same spectrum cannot total more than unity
void MixtureModel::computeAdjDoubleTriplySpectraProbs() {

  if(negOnly_[1] || negOnly_[2]) {
    return;
  }
  num_pairs_ = (*spectra_)[1]->size()+(*spectra_)[2]->size(); // 2+ and 3+
  pairedspecs_ = new Spectrum[num_pairs_];
  adjusted_ = new Array<int>;
  int total = 0;
  for(int charge = 1; charge < 3; charge++) {
      for(int k = 0; k < (*probs_)[charge]->size(); k++) {
		  int len=(int)strlen((*(*spectra_)[charge])[k]);
	char* next = new char[len+ 3];
	strcpy(next, (*(*spectra_)[charge])[k]);
	next[len] = '.';
	next[len + 1] = charge + '1';
	next[len + 2] = 0;
	pairedspecs_[total++].init(next, 
				      ((NTTMixtureDistr*)(getMixtureDistr(ntt_name_, charge)))->getNTTValue(k), (*(*inds_)[charge])[k], k);
	delete [] next;
      }
  }
  // now order it
  qsort(pairedspecs_, num_pairs_, sizeof(Spectrum), (int(*)(const void*, const void*)) comp_specs);
  // now walk down list and make 2/3 adjustments....
  for(int k = 0; k < num_pairs_-1; k++) {
    if(strncmp(pairedspecs_[k].name_, pairedspecs_[k+1].name_, strlen(pairedspecs_[k].name_)-2) == 0) {
      // make adjustment

      double prob1 = getAdjDoublyTriplyProb(pairedspecs_[k].prob_, pairedspecs_[k+1].prob_);
      double prob2 = getAdjDoublyTriplyProb(pairedspecs_[k+1].prob_, pairedspecs_[k].prob_);
      if(pairedspecs_[k].prob_ >= 0.5 && pairedspecs_[k+1].prob_ >= 0.5) {
	adjusted_->insertAtEnd(k);
	adjusted_->insertAtEnd(k+1);
      }
      pairedspecs_[k].prob_ = prob1;
      pairedspecs_[k+1].prob_ = prob2;
      k++;
    }
  }

}

void MixtureModel::findRepSpecs() {
  int total = 0;
  int charge;
  int k;
  SpectHash::iterator itr;
   rep_specs_ = new SpectHash();
    for(charge = 0; charge < MAX_CHARGE; charge++) {
      for(k = 0; k < (*probs_)[charge]->size(); k++) {
	
	int len=(int)strlen((*(*spectra_)[charge])[k]);
	char* next = new char[len+3];
	strcpy(next, (*(*spectra_)[charge])[k]);
	
	next[len] = '.';
	//DDS: TODO WILL NOT WORK when charge is multi character (e.g. 10)?
	next[len + 1] = charge + '1';
	next[len + 2] = 0;
	
	itr = rep_specs_->find( (*(*spectra_)[charge])[k] );
	

	if ( itr == rep_specs_->end() ) {
	  total++;
	  rep_specs_->insert( std::make_pair( (*(*spectra_)[charge])[k], new Array<Spectrum*>() ) );
	  itr = rep_specs_->find( (*(*spectra_)[charge])[k] );
	}
	double tprob =  (*(*probs_)[charge])[k];
	//	if ( (*isdecoy_inds_)[(*(*inds_)[charge])[k]] ) {
	//  tprob = 0;
	//	}
	if ( use_decoy_ && (*(*isdecoy_inds_)[charge])[k]) {
	  tprob = 0;
	}
	Spectrum* s = new Spectrum(next, 
				   ((NTTMixtureDistr*)(getMixtureDistr(ntt_name_, charge)))->getNTTValue(k), (*(*inds_)[charge])[k], k);

	//	if (strstr((*itr).first.c_str(), "022008_F8_ETD_1.03159.03159") != NULL) {
	//	  cerr << "DDS: DEBUG" << endl;
	//	}

	if (itr->second->size() <= charge)
	  itr->second->insertAtEnd( s );
	else 
	  delete s;
	
	delete [] next;
	
	
      }
    }
}
// add constraint that final probs for 2+ and 3+ precursor ion interpretations
// of same spectrum cannot total more than unity
void MixtureModel::computeAdjMultiChargeSpectraProbs() {
  //  if(negOnly_[1] || negOnly_[2]) {
  //  return;
  //}

  num_reps_ = 0;
  for(int z = 0; z < MAX_CHARGE; z++) {
    num_reps_ = (*spectra_)[z]->size(); // 2+ and 3+
  }

  

  if (rep_specs_ == NULL) {
    findRepSpecs();
  }
  SpectHash::const_iterator z;
  for(z = rep_specs_->begin(); z != rep_specs_->end(); z++) {
    double denom=0;
    double MAXPROB = 0.99999999;
    double allneg=0;
    double probsum=0;
    double common=0;

    bool prob1=false;
    bool allneg0=false;


    if ((*z->second).size() > 1) {

      for (int i=0; i< (*z->second).size(); i++) {
	double tprob = (*(*probs_)[(*z->second)[i]->charge_])[(*z->second)[i]->data_index_];

	if (tprob <= 1-MAXPROB ) {
	  continue;
        }

	if (tprob >= MAXPROB) {
	  tprob = MAXPROB ;
	}

	probsum += tprob;

	allneg += log(1-tprob);
	
      }

      double denom = exp(allneg);

      for (int i=0; i< (*z->second).size(); i++) {
	double tprob = (*(*probs_)[(*z->second)[i]->charge_])[(*z->second)[i]->data_index_];

	if (tprob <= 1-MAXPROB ) {
	  continue;
        }

	if (tprob >= MAXPROB) {
	  tprob = MAXPROB ;
	}
	denom += exp(allneg - log(1-tprob) + log(tprob));

      }
      
      
 
      double tnumer=0;
      for (int i=0; i< (*z->second).size(); i++) {


	bool allneg0 = false;
	double old_prob = (*(*probs_)[(*z->second)[i]->charge_])[(*z->second)[i]->data_index_];
	double new_prob;

	if (old_prob <= 1 - MAXPROB) {
	  continue;
	}

	if (old_prob >= MAXPROB) {
	  old_prob = MAXPROB;
	}

	tnumer = allneg - log(1-old_prob) + log(old_prob);
	tnumer = exp(tnumer);	
	

	new_prob = tnumer / denom;
	
	
	//if (probsum > 1) {
	//   new_prob = old_prob / probsum;
	//}
	//else {
	//  new_prob = old_prob;
	//}

	
	if (fabs(new_prob - old_prob) >= 0.05 && new_prob > 0.05) {
	  (*(*probs_)[(*z->second)[i]->charge_])[(*z->second)[i]->data_index_] = new_prob;
	  adj_set_.insert((*z->second)[i]->ind_);
	}
      }
    }
  }
}

//adds roc curve and error data to xml file for one or all charges - Richie Stauffer, August 2010
//charge of 0 returns data from all charges that aren't ignored
//charge of 1 through 7 (MAX_CHARGE) returns data from that charge state alone (if charge is not ignored)
//other charge values return null
Array<Tag*>* MixtureModel::getRocDataPointTags(int charge) //takes charge value, not index
{
	if ((charge < 0 || charge > MAX_CHARGE) || (charge && (ignore_[charge - 1] || (negOnly_[charge - 1] && !forcedistr_)))) //make sure charge is in range
		return NULL;
		
	bool noData = true;
		
	Array<Tag*>* output = new Array<Tag*>; //array returned
	Tag* tag;
	
	char text[500]; //helps convert numbers into text so they can be passed into methods

	int total = 0;

	if(charge)
	{
		if(!ignore_[charge - 1] && (!negOnly_[charge - 1] || forcedistr_))
			total += (*probs_)[charge - 1]->size();
	}
	else
	{
		for(int c = 0; c < MAX_CHARGE; c++)
			if(!ignore_[c] && (!negOnly_[c] || forcedistr_))
				total += (*probs_)[c]->size();
	}

	double* combinedprobs = new double[total];
	total = 0; //index

	if(charge)
	{
		if(!ignore_[charge - 1] && (! negOnly_[charge - 1] || forcedistr_))
			for(int k = 0; k < (*probs_)[charge - 1]->size(); k++)
				combinedprobs[total++] = (*(*probs_)[charge - 1])[k];
	}
	else
	{
		for(int c = 0; c < MAX_CHARGE; c++)
			if(!ignore_[c] && (!negOnly_[c] || forcedistr_))
				for(int k = 0; k != (*probs_)[c]->size(); k++)
					combinedprobs[total++] = (*(*probs_)[c])[k];
	}

	qsort(combinedprobs, total, sizeof(double), (int(*)(const void*, const void*)) comp_nums);

	//now sens and error as walk down list
	double thresh[] = {0.9999, 0.999, 0.99, 0.98, 0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6, 0.55, 0.5, 0.45, 0.4, 0.35, 0.3, 0.25, 0.2, 0.15, 0.1, 0.05, 0.0};
	int threshind = 0;
	double correct = 0.0;
	double incorrect = 0.0;
	double totcorrect = 0.0;
	
	for(int k = 0; k < total; k++)
		totcorrect += combinedprobs[k];

	double grandTotal = totcorrect;
	
	tag = new Tag("roc_error_data", true, false);
	
	if(charge)
	{
		sprintf(text, "%d", charge);
		tag->setAttributeValue("charge", text);
		sprintf(text, "%0.1f", getTotalAdjProb(charge - 1));
		tag->setAttributeValue("charge_est_correct", text);
	}
	else //charge is 0
		tag->setAttributeValue("charge", "all");
		
	output->insertAtEnd(tag);
	
	if(grandTotal > 0.0)
	{
		if(noData)
			noData = false;
	
		for(int k = 0; k < total; k++)
		{
			double rnd;
			sprintf(text, "%0.4f", combinedprobs[k]);
			rnd = atof(text);
			if(rnd >= thresh[threshind])
			{
				correct += combinedprobs[k];
				incorrect += 1 - combinedprobs[k];
			}
			else if (rnd >= 0)
			{
				tag = new Tag("roc_data_point", true, true); //creates point
				sprintf(text, "%0.4f", thresh[threshind]);
				tag->setAttributeValue("min_prob", text);
				sprintf(text, "%0.4f", correct / totcorrect);
				tag->setAttributeValue("sensitivity", text);

				if(correct + incorrect > 0.0)
				{
					sprintf(text, "%0.4f", incorrect/(correct + incorrect));
					tag->setAttributeValue("error", text);
				}
				else
				{
					sprintf(text, "%0.0f", 0.0);
					tag->setAttributeValue("error", text);
				}

				sprintf(text, "%0.0f", correct);
				tag->setAttributeValue("num_corr", text);
				sprintf(text, "%0.0f", incorrect);
				tag->setAttributeValue("num_incorr", text);
				output->insertAtEnd(tag);
				threshind++;
				k--; //assay again using next threshold
			}
		} //next member

		tag = new Tag("roc_data_point", true, true); //creates point
		sprintf(text, "%0.4f", thresh[threshind]);
		tag->setAttributeValue("min_prob", text);
		sprintf(text, "%0.4f", correct/totcorrect);
		tag->setAttributeValue("sensitivity", text);
		sprintf(text, "%0.4f", incorrect/(correct + incorrect));
		tag->setAttributeValue("error", text);
		sprintf(text, "%0.0f", correct);
		tag->setAttributeValue("num_corr", text);
		sprintf(text, "%0.0f", incorrect);
		tag->setAttributeValue("num_incorr", text);
		output->insertAtEnd(tag);
	}
	else
	{
		tag = new Tag("roc_data_point", true, true); //creates point
		sprintf(text, "%0.4f", 0.0);
		tag->setAttributeValue("min_prob", text);
		sprintf(text, "%0.4f", 0.0);
		tag->setAttributeValue("sensitivity", text);
		sprintf(text, "%0.4f", 0.0);
		tag->setAttributeValue("error", text);
		sprintf(text, "0.0");
		tag->setAttributeValue("num_corr", text);
		sprintf(text, "0.0");
		tag->setAttributeValue("num_incorr", text);
		output->insertAtEnd(tag);
	}
	
	if (noData) //if after looping through everything no real data is found return null
		return NULL;
	
	//set error rates
	
	if(grandTotal > 0.0)
	{
		double error_rates[] = {0.0, 0.0001, 0.0002, 0.0003, 0.0004, 0.0005, 0.0006, 0.0007, 0.0008, 0.0009, 0.001, 0.0015, 0.002, 0.0025, 0.003, 0.004, 0.005, 0.006, 0.007, 0.008, 0.009, 0.01, 0.015, 0.02, 0.025, 0.03, 0.04, 0.05, 0.075, 0.1, 0.15};
  		correct = 0.0;
		incorrect = 0.0;
		threshind = 0;
		for(int k = 0; k < total; k++)
		{
			if(incorrect / (correct + incorrect) > error_rates[threshind])
			{
				tag = new Tag("error_point", true, true);
				if(k == 0)
				{
					sprintf(text, "%0.4f", error_rates[threshind]);
					tag->setAttributeValue("error", text);
					sprintf(text, "%0.4f", combinedprobs[k]);
					tag->setAttributeValue("min_prob", text);
					sprintf(text, "%0.0f", correct + combinedprobs[k]);
					tag->setAttributeValue("num_corr", text);
					sprintf(text, "%0.0f", incorrect + 1.0 - combinedprobs[k]);
					tag->setAttributeValue("num_incorr", text);
				}
				else
				{
					sprintf(text, "%0.4f", error_rates[threshind]);
					tag->setAttributeValue("error", text);
					sprintf(text, "%0.4f", combinedprobs[k - 1]);
					tag->setAttributeValue("min_prob", text);
					sprintf(text, "%0.0f", correct + combinedprobs[k]);
					tag->setAttributeValue("num_corr", text);
					sprintf(text, "%0.0f", incorrect + 1.0 - combinedprobs[k]);
					tag->setAttributeValue("num_incorr", text);
				}

				output->insertAtEnd(tag);
				threshind++;
				k--;
			}
			else
			{
				correct += combinedprobs[k];
				incorrect += 1 - combinedprobs[k];
			}

			if(threshind == (sizeof(error_rates) / sizeof(double)) - 1)
				k = total; //done
		} //next ordered prob
	} //if have data
	
	//finally insert end of roc_error_data tag
	output->insertAtEnd(new Tag("roc_error_data", false, true));
	
	delete[] combinedprobs;
	return output;
}

// writes out sensitivity and error for various minimum probs
void MixtureModel::computeEstimatedSensAndError(char* filename) {
  if(use_adj_probs_ && rep_specs_ == NULL) {
    //    computeAdjDoubleTriplySpectraProbs();
    computeAdjMultiChargeSpectraProbs();
  }
  int total = 0;
  for(int charge = 0; charge < MAX_CHARGE; charge++) {
    if(!ignore_[charge] && (! negOnly_[charge] || forcedistr_)) {
      total += (*probs_)[charge]->size();
    }
  }
  
  double* combinedprobs = new double[total];
  total = 0; // index
  int k;
  for(k = 0; k < (*probs_)[0]->size(); k++) {
    if(!ignore_[0] && (! negOnly_[0] || forcedistr_)) {
      combinedprobs[total++] = (*(*probs_)[0])[k];
    }
  }
  if(use_adj_probs_) {
    for(int k = 0; k < num_pairs_; k++) {
      if(!ignore_[pairedspecs_[k].charge_] && (! negOnly_[pairedspecs_[k].charge_] || forcedistr_)) {
	combinedprobs[total++] = pairedspecs_[k].prob_;
      }
    }
    for(int charge = 1; charge < MAX_CHARGE; charge++) {
      for(int k = 0; k < (*probs_)[charge]->size(); k++) {
	if(!ignore_[charge] && (! negOnly_[charge] || forcedistr_)) {
	  combinedprobs[total++] = (*(*probs_)[charge])[k];
	}
      }
    } // next 
  }
  else {
    for(int charge = 0; charge < MAX_CHARGE; charge++) {
      for(int k = 0; k < (*probs_)[charge]->size(); k++) {
	if(!ignore_[charge] && (! negOnly_[charge] || forcedistr_)) {
	  combinedprobs[total++] = (*(*probs_)[charge])[k];
	}
      }
    } // next 
  } // next charge
  
  qsort(combinedprobs, total, sizeof(double), (int(*)(const void*, const void*)) comp_nums);

     // now sens and error as walk down list
  double thresh[] = {0.9999, 0.999, 0.99, 0.98, 0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6, 0.55, 0.5, 0.45, 0.4, 0.35, 0.3, 0.25, 0.2, 0.15, 0.1, 0.05, 0.0};
  int threshind = 0;
  double correct = 0.0;
  double incorrect = 0.0;
  double totcorrect = 0.0;
  FILE* fout;
  for(k = 0; k < total; k++) {
    totcorrect += combinedprobs[k];
  }
  if( (fout = fopen(filename, "a")) == NULL) {
    cerr << "cannot open " << filename << endl;
    exit(1);
  }
  double grandTotal = totcorrect;
  fprintf(fout, "\n\nJOINT 1+/2+/3+ MODEL SENSITIVITY/ERROR PREDICTIONS (%d tot correct)\n", (int)(grandTotal));
  fprintf(fout, "#min_prob\tsens\terr\t\test # corr\test # incorr\n");
  if(grandTotal > 0.0) {
    for(int k = 0; k < total; k++) {
      if(combinedprobs[k] >= thresh[threshind]) {
	correct += combinedprobs[k];
	incorrect += 1 - combinedprobs[k];
      }
      else {
	if(correct + incorrect > 0.0)
	  fprintf(fout, "%0.4f\t\t%0.4f\t%0.4f\t\t%0.0f\t\t%0.0f\n", thresh[threshind], correct/totcorrect, incorrect/(correct + incorrect), correct, incorrect);
	else
	  fprintf(fout, "%0.4f\t\t%0.4f\t%0.4f\t\t%0.0f\t\t%0.0f\n", thresh[threshind], correct/totcorrect, 0.0, correct, incorrect);
	threshind++;
	k--; // assay again using next threshold
      }
    } // next member
    fprintf(fout, "%0.4f\t\t%0.4f\t%0.4f\t\t%0.0f\t\t%0.0f\n", thresh[threshind], correct/totcorrect, incorrect/(correct + incorrect), correct, incorrect);
  } // if have data
  else 
    fprintf(fout, "%0.4f\t\t%0.4f\t%0.4f\t\t%0.0f\t\t%0.0f\n", 0.0, 0.0, 0.0, 0.0, 0.0);
  fprintf(fout, "\n\n");

  fprintf(fout, "\n\nJOINT 1+/2+/3+ MIN PROB THRESHOLDS FOR SPECIFIED ERROR RATES\n");
  fprintf(fout, "#err\t\tmin_prob\test # corr\test # incorr\n");
  if(grandTotal > 0.0) { 
    double error_rates[] = {0.0, 0.0001, 0.0002, 0.0003, 0.0004, 0.0005, 0.0006, 0.0007, 0.0008, 0.0009, 0.001, 0.0015, 0.002, 0.0025, 0.003, 0.004, 0.005, 0.006, 0.007, 0.008, 0.009, 0.01, 0.015, 0.02, 0.025, 0.03, 0.04, 0.05, 0.075, 0.1, 0.15};
    correct = 0.0;
    incorrect = 0.0;
    threshind = 0;
    for(int k = 0; k < total; k++) {
      if(incorrect / (correct + incorrect) > error_rates[threshind]) {
	if(k == 0) {
	  fprintf(fout, "%0.4f\t\t%0.4f\t\t%0.0f\t\t%0.0f\n", error_rates[threshind], combinedprobs[k], correct + combinedprobs[k], incorrect + 1.0 - combinedprobs[k]);
	}
	else {
	  fprintf(fout, "%0.4f\t\t%0.4f\t\t%0.0f\t\t%0.0f\n", error_rates[threshind], combinedprobs[k-1], correct + combinedprobs[k], incorrect + 1.0 - combinedprobs[k]);
	}
	threshind++;
	k--;
      }
      else {
	correct += combinedprobs[k];
	incorrect += 1 - combinedprobs[k];
      }
      if(threshind == (sizeof(error_rates)/sizeof(double)) - 1) {
	k = total; // done
      }
    } // next ordered prob
  } // if have data
  fprintf(fout, "\n\n");

  fclose(fout);
}

int comp_nums(const void* num1, const void* num2) {
  if(*(double*)num1 < *(double*)num2) return 1;
  if(*(double*)num1 == *(double*)num2) return 0;
  return -1;
}

int comp_specs(const void* num1, const void* num2) {
  return strcmp(((Spectrum*)num1)->name_, ((Spectrum*)num2)->name_);
}

int comp_ords(const void* num1, const void* num2) {
  if(((OrderedResult*)num1)->input_index_ < ((OrderedResult*)num2)->input_index_) return -1;
  if(((OrderedResult*)num1)->input_index_ > ((OrderedResult*)num2)->input_index_) return 1;
  return 0;
}


// adjustment imposing constraint that final probs for 2+ and 3+ precursor ion interpretations
// of same spectrum do not total more than unity
// DDS: This implementation is not Mathematically correct
//double MixtureModel::getAdjDoublyTriplyProb(double prob_2_adj, double prob_of_partner) {
//  if(prob_2_adj == 0 || prob_2_adj + prob_of_partner == 0) {
//    return 0.0;
//  }
//  return (prob_2_adj * (prob_2_adj + prob_of_partner - prob_2_adj * prob_of_partner) / (prob_2_adj + prob_of_partner));
//}

double MixtureModel::getAdjDoublyTriplyProb(double prob_2_adj, double prob_of_partner) {
  if(prob_2_adj == 0 || prob_2_adj + prob_of_partner == 0) {
    return 0.0;
  }
  if(prob_2_adj == 1 && prob_of_partner == 1) {
    return 0.5;
  }
  return (prob_2_adj * (1-prob_of_partner) / (1-prob_2_adj*prob_of_partner));
}
  
void MixtureModel::multiplyBySpectraSTLibProbs() {

  for (int charge = 0; charge < 3; charge++) {
    for (int index = 0; index < (*probs_)[charge]->size(); index++) {
      (*(*probs_)[charge])[index] *= (*(*libprobs_)[charge])[index];
    }
  }

  /*
  for (int charge = 0; charge < 3; charge++) {

    // update everything except discriminant function (hence k starts from 1)    
    for(int k = 1; k < (*mixtureDistrs_)[charge]->size(); k++) {
      (*(*mixtureDistrs_)[charge])[k]->update(probs_, charge);
    }

    updateProbs(charge, true);
  }
  */
}
