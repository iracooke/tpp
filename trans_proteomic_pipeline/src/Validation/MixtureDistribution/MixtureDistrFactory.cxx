/*

Program       : MixtureDist                                                       
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

#include "MixtureDistrFactory.h"
MixtureDistrFactory::MixtureDistrFactory() {}
//MixtureDistrFactory::MixtureDistrFactory(char* engine, char* enzyme, char* massspec, ScoreOptions options) { 
MixtureDistrFactory::MixtureDistrFactory(ModelOptions& modelopts, ScoreOptions& options) { 

  scoreOpts_ = options;
  modelOpts_ = modelopts;
  // HENRY: add SpectraST
  const char* engines[] = {"SEQUEST", "MASCOT", "COMET", "PROBID", "X! TANDEM", "SPECTRAST", "PHENYX", "OMSSA", "MYRIMATCH", "INSPECT", "YABSE", "CRUX", "MS-GFDB", "MS-GF+",  "HYDRA"};
  int num_engines = sizeof(engines)/sizeof(char*);
  Boolean found = False;

  char enginePrefix[256];
  //  char* engine = getSearchEngine(modelopts);
  int k;
  for(k = 0; k < num_engines; k++) {
    // X! Tandem/Hydra supports multiple scoring algorithms which are
    // appended as " (algorithm)"
    int len = (int)strlen(engines[k]);
    strncpy(enginePrefix, modelOpts_.engine_, len);
    enginePrefix[len] = '\0';

    if(! strcasecmp(enginePrefix, engines[k]))
      found = True;
  }
  if(! found) {
    // error
     cout << "error: engine " << modelOpts_.engine_ << " not recognized" << std::endl;
    exit(1);
  }

  search_engine_ = modelOpts_.engine_;

  //  search_engine_ = new char[strlen(engine)+1];
  //  strcpy(search_engine_, engine);

  //  char* enzyme = getSampleEnzyme(modelopts);

  //  enzyme_ = new char[strlen(enzyme)+1];
  //  strcpy(enzyme_, enzyme);

  const char* massspecs[] = {"LCQ"};
  int num_specs = sizeof(massspecs)/sizeof(char*);
  found = False;

  const char* massspec = ""; // to be replaced with getMassSpecInfo....

  for(k = 0; k < num_specs; k++)
    if(! strcasecmp(massspec, massspecs[k]))
      found = True;
  if(! found) {
    // error
  }
  mass_spec_type_ = massspec;

  // now options
  //options_ = new Array<char*>;

  strcpy(discr_mixture_distr_name_, modelOpts_.engine_);
  strcat(discr_mixture_distr_name_, " discrim score [fval]");

  getNTTMixtureDistrName(modelOpts_.enzyme_);
  /*
  strcpy(ntt_mixture_distr_name_, "no. tolerable ");
  if(enzyme_ == NULL)
    strcat(ntt_mixture_distr_name_, "tryptic");
  else strcat(ntt_mixture_distr_name_, enzyme_);
  strcat(ntt_mixture_distr_name_, " term. [ntt]");
  */
}


SearchResult* MixtureDistrFactory::getSearchResult(Array<Tag*>* tags) {
  return getSearchResult(tags, modelOpts_.engine_);

}
SearchResult* MixtureDistrFactory::getSearchResult(Array<Tag*>* tags, char* engine) {
  //  cout << "model: " << engine << endl;
  if(! strcasecmp(engine, "SEQUEST")) {
    return new SequestResult(tags);
  }
  else if(! strcasecmp(engine, "MASCOT")) {
    //cout << "here" << endl;

    //for(int k = 0; k < tags->length(); k++)
    //   (*tags)[k]->write(cout);


    return new MascotResult(tags);
    //cout << "there!" << endl;
  }
  else if(! strcasecmp(engine, "COMET")) {
    return new CometResult(tags);
  }
  else if(! strcasecmp(engine, "PROBID")) {
    return new ProbIDResult(tags);
  }
  else if(! strncmp(engine, "X! Tandem", 9)) {
    return new TandemResult(tags);
  }
  //  else if(! strncmp(engine, "Hydra(k-score)", 14) ||
  //	  ! strncmp(engine, "Hydra(KScore)", 13)  ||
  //	  ! strncmp(engine, "Hydra(ProbId)", 13)      ) { 
  // return new HydraResult(tags);
  //}
  // HENRY: add SpectraST
  else if(! strcasecmp(engine, "SPECTRAST")) {
    return new SpectraSTResult(tags);
  }
  else if(! strcasecmp(engine, "PHENYX")) {
    return new PhenyxResult(tags);
  }
  else if(! strcasecmp(engine, "OMSSA")) {
    return new OMSSAResult(tags);
  }
  else if(! strcasecmp(engine, "YABSE")) {
    return new YABSEResult(tags);
  }
  else if(! strcasecmp(engine, "MYRIMATCH")) {
    return new MyrimatchResult(tags);
  }
  else if(! strcasecmp(engine, "MS-GFDB")) {
    return new MSGFDBResult(tags);
  }
  else if(! strcasecmp(engine, "MS-GF+")) {
    return new MSGFPLResult(tags);
  }
  else if(! strcasecmp(engine, "INSPECT")) {
    return new InspectResult(tags);
  }
 else if(! strcasecmp(engine, "CRUX")) {
    return new CruxResult(tags);
  }
  else {
    // error
    //    cout << "NULL" << endl;
    return NULL;
  }

  
}

SearchResult* MixtureDistrFactory::getSearchResultWithAppliedOpts(Array<Tag*>* tags) {
  SearchResult* result = getSearchResult(tags);

  if(result == NULL)
    return result;
  if(applyOptions(result))
    return result;

  // doesn't pass, set to null
  delete result;
  return NULL;
}

Boolean MixtureDistrFactory::applyOptions(SearchResult* result) {
  if(! result->isProcessed())
    return False;
  if(result->charge_ < 1 || result->charge_ > MAX_CHARGE)
    return False;
  if(result->num_tol_term_ < 0) 
    return False;

  if(! strcasecmp(search_engine_.c_str(), "SEQUEST")) {
    SequestResult* seq = (SequestResult*)result;
    // delta star
    if(scoreOpts_.deltastar_ != DELTACN_LEAVE && seq->deltastar_){ 
      if(scoreOpts_.deltastar_ == DELTACN_ZERO){
	seq->delta_ = 0.0;
      } else {
	//cout << "returning false for " << seq->spectrum_ << endl;
	//if(seq->deltastar_)
	//  cout << "deltastar" << endl;
	//else
	//  cout << "NO deletastar" << endl;
	return False; // doesn't pass
      }
    }
    if(scoreOpts_.deltastar_ == DELTACN_LEAVE && seq->delta_ == 1.0)
      seq->delta_ = 0.0;
    return True;
  }
  if(! strcasecmp(search_engine_.c_str(), "MASCOT")) {
    double PENALTY = 0.5;
    MascotResult* mas = (MascotResult*)result;
    if(scoreOpts_.mascotstar_ == MASCOTSTAR_PENALIZE && mas->star_) {
      //if(mas->ionscore_ > mas->identity_)
      mas->ionscore_ *= PENALTY; // * (mas->ionscore_ + mas->identity_);
      return True;
    }
    if(scoreOpts_.mascotstar_ != MASCOTSTAR_EXCLUDE || ! mas->star_) 
      return True;

    // still here 
    return False;
  }

  if(! strcasecmp(search_engine_.c_str(), "COMET")) {
    CometResult* comet = (CometResult*)result;
    if(scoreOpts_.cometstar_ == COMETSTAR_ZERO && comet->deltastar_) {
      comet->delta_ = 0.0;
      return True;
    }
    if(scoreOpts_.cometstar_ != COMETSTAR_EXCLUDE || ! comet->deltastar_) 
      return True;

    // still here 
    return False;
  } // comet



  return True;
}


const char* MixtureDistrFactory::getDiscrMixtureDistrName() {
  return discr_mixture_distr_name_;
}

const char* MixtureDistrFactory::getNTTMixtureDistrName() {
  return ntt_mixture_distr_name_;
}

const char* MixtureDistrFactory::getNTTMixtureDistrName(const char* enzyme) {
  strcpy(ntt_mixture_distr_name_, "no. tolerable ");
  if(enzyme == NULL)
    strcat(ntt_mixture_distr_name_, "tryptic");
  else strcat(ntt_mixture_distr_name_, enzyme);

  strcat(ntt_mixture_distr_name_, " term. [ntt]");
  return ntt_mixture_distr_name_;
}

DiscrimValMixtureDistr* MixtureDistrFactory::getDiscrimValMixtureDistr(int charge) {
  return getDiscrimValMixtureDistr(charge, True);
}
DiscrimValMixtureDistr* MixtureDistrFactory::getDiscrimValMixtureDistr(int charge, Boolean gamma) {
  return getDiscrimValMixtureDistr(charge, gamma, False) ;
}

DiscrimValMixtureDistr* MixtureDistrFactory::getDiscrimValMixtureDistr(int charge, Boolean gamma, Boolean nonparam) {
  return getDiscrimValMixtureDistr(charge, gamma, nonparam, False) ;
}
DiscrimValMixtureDistr* MixtureDistrFactory::getDiscrimValMixtureDistr(int charge, Boolean gamma, Boolean nonparam, Boolean use) {

  DiscrimValMixtureDistr* distr = NULL;
  //Boolean gamma = True;
  Boolean maldi = False;
  Boolean qtof = False;

  if(! strcasecmp(search_engine_.c_str(), "SEQUEST")) {
    //cout << "making SQ f distr" << endl;
    //distr = new DiscrimValMixtureDistr(charge, getDiscrMixtureDistrName(), "fval", gamma, maldi, qtof);
    distr = new DiscrimValMixtureDistr(charge, getDiscrMixtureDistrName(), "fval", True, maldi, qtof, nonparam, use);

  }
  else if(! strcasecmp(search_engine_.c_str(), "CRUX")) {
    //cout << "making SQ f distr" << endl;
    //distr = new DiscrimValMixtureDistr(charge, getDiscrMixtureDistrName(), "fval", gamma, maldi, qtof);
    distr = new CruxDiscrimValMixtureDistr(charge, search_engine_.c_str(), getDiscrMixtureDistrName(), "fval", modelOpts_.maldi_, qtof, gamma, nonparam, use);
    //    distr = new CruxDiscrimValMixtureDistr(charge, getDiscrMixtureDistrName(), "fval", True, maldi, qtof, nonparam);

  }
  else if(! strcasecmp(search_engine_.c_str(), "MASCOT")) {
    //cout << "making mascot f distr" << endl;
    //distr = new MascotDiscrimValMixtureDistr(charge, getDiscrMixtureDistrName(), "fval", maldi, qtof, scoreOpts_.inputfile_, nonparam);
    distr = new MascotDiscrimValMixtureDistr(charge, getDiscrMixtureDistrName(), "fval", maldi, qtof, scoreOpts_, nonparam);

  }
  else if(! strcasecmp(search_engine_.c_str(), "COMET")) {
    //cout << "making mascot f distr" << endl;
    distr = new CometDiscrimValMixtureDistr(charge, getDiscrMixtureDistrName(), "fval", gamma, modelOpts_.maldi_, qtof, nonparam, use);

  }
  else if(! strcasecmp(search_engine_.c_str(), "PROBID")) {
    //cout << "making mascot f distr" << endl;
    distr = new ProbIDDiscrimValMixtureDistr(charge, getDiscrMixtureDistrName(), "fval", modelOpts_.maldi_, qtof, nonparam);

  }
  else if(! strncmp(search_engine_.c_str(), "X! Tandem", 9)) {
    //cout << "making mascot f distr" << endl;
    distr = new TandemDiscrimValMixtureDistr(charge, search_engine_.c_str(), getDiscrMixtureDistrName(), "fval", modelOpts_.maldi_, qtof, gamma, nonparam, use);

  }
  //  else if(! strncmp(search_engine_.c_str(), "Hydra(KScore)", 13)  || ! strncmp(search_engine_.c_str(), "Hydra(k-score)", 14) || ! strncmp(search_engine_.c_str(), "Hydra(ProbId)", 13)) {
  //  //cout << "making mascot f distr" << endl;
  //  distr = new HydraDiscrimValMixtureDistr(charge, search_engine_.c_str(), getDiscrMixtureDistrName(), "fval", modelOpts_.maldi_, qtof, gamma, nonparam, use);
  //
  //}
  // HENRY: add SpectraST
  else if(! strcasecmp(search_engine_.c_str(), "SPECTRAST")) {
    //cout << "making spectrast f distr" << endl;
    distr = new SpectraSTDiscrimValMixtureDistr(charge, getDiscrMixtureDistrName(), "fval", modelOpts_.maldi_, qtof, nonparam, modelOpts_.optimize_fval_);

  }
  else if(! strcasecmp(search_engine_.c_str(), "PHENYX")) {
    //cout << "making spectrast f distr" << endl;
    distr = new PhenyxDiscrimValMixtureDistr(charge, search_engine_.c_str(), getDiscrMixtureDistrName(), "fval", modelOpts_.maldi_, qtof, gamma, nonparam);

  }
  else if(! strcasecmp(search_engine_.c_str(), "OMSSA")) {
    //cout << "making spectrast f distr" << endl;
    distr = new OMSSADiscrimValMixtureDistr(charge, search_engine_.c_str(), getDiscrMixtureDistrName(), "fval", modelOpts_.maldi_, qtof, gamma, nonparam);
  }
  else if(! strcasecmp(search_engine_.c_str(), "YABSE")) {
    //cout << "making spectrast f distr" << endl;
    distr = new YABSEDiscrimValMixtureDistr(charge, search_engine_.c_str(), getDiscrMixtureDistrName(), "fval", modelOpts_.maldi_, qtof, gamma, nonparam);
  }
 else if(! strcasecmp(search_engine_.c_str(), "MYRIMATCH")) {
    //cout << "making spectrast f distr" << endl;
    distr = new MyrimatchDiscrimValMixtureDistr(charge, search_engine_.c_str(), getDiscrMixtureDistrName(), "fval", modelOpts_.maldi_, qtof, gamma, nonparam);
  }
   else if(! strcasecmp(search_engine_.c_str(), "MS-GFDB")) {
    //cout << "making spectrast f distr" << endl;
    distr = new MSGFDBDiscrimValMixtureDistr(charge, search_engine_.c_str(), getDiscrMixtureDistrName(), "fval", modelOpts_.maldi_, qtof, gamma, nonparam);
  }
   else if(! strcasecmp(search_engine_.c_str(), "MS-GF+")) {
    //cout << "making spectrast f distr" << endl;
    distr = new MSGFPLDiscrimValMixtureDistr(charge, search_engine_.c_str(), getDiscrMixtureDistrName(), "fval", modelOpts_.maldi_, qtof, gamma, nonparam);
  }
 else if(! strcasecmp(search_engine_.c_str(), "INSPECT")) {
    //cout << "making spectrast f distr" << endl;
    distr = new InspectDiscrimValMixtureDistr(charge, search_engine_.c_str(), getDiscrMixtureDistrName(), "fval", modelOpts_.maldi_, qtof, gamma, nonparam);
  }


  if(distr != NULL)
    distr->setDiscrimFunction(mass_spec_type_.c_str());
  else {
    cout << "NULL distri, search engine=" << search_engine_ << endl;
    exit(1);
  }
  //cout << "DONE" << endl;
  return distr;
}

NTTMixtureDistr* MixtureDistrFactory::getNTTMixtureDistr(int charge) {
  return new NTTMixtureDistr(charge, getNTTMixtureDistrName(), "ntt");

}

IsoMassDiffDiscrMixtureDistr* MixtureDistrFactory::getIsoMassDiffDiscrMixtureDistr(int charge) {
  char distr_name[500];
  strcpy(distr_name, "isotopic peak mass difference [IsoMassDiff]");
  return new IsoMassDiffDiscrMixtureDistr(charge, distr_name, "isomassd");

}

NMCMixtureDistr* MixtureDistrFactory::getNMCMixtureDistr(int charge) {
  //  if(! strcmp(search_engine_, "MASCOT"))
  //   return NULL; // don't use
  

  char distr_name[500];
  strcpy(distr_name, "no. missed enz. cleavages [nmc]");
  return new NMCMixtureDistr(charge, distr_name, "nmc");
}

MassDifferenceDiscrMixtureDistr* MixtureDistrFactory::getMassDifferenceDiscrMixtureDistr(int charge) {
  char distr_name[500];
  strcpy(distr_name, "accurate mass diff [massd]");
  if (modelOpts_.accMass_) {
    return new AccurateMassDiffDiscrMixtureDistr(charge, distr_name, "massd", 0.5, 0.001, 0.0, modelOpts_.ppm_);
    //return new VariableOffsetMassDiffDiscrMixtureDistr(charge, distr_name, "massd", 0.5, 0.001, 0.0);
  }
  else {
    return new VariableOffsetMassDiffDiscrMixtureDistr(charge, distr_name, "massd", 5.0, 1.0, 0.0);
  }
}

// HENRY: Precursor m/z distribution for SpectraST
MzDifferenceDiscrMixtureDistr* MixtureDistrFactory::getMzDifferenceDiscrMixtureDistr(int charge) {

  return new MzDifferenceDiscrMixtureDistr(charge, "m/z diff [mzd]", "mzd", 5.0, 1.0);

}

// HENRY: Modify prototypes for icatType specification
ICATMixtureDistr* MixtureDistrFactory::getICATMixtureDistr(int charge, int icatType) {
  char distr_name[500];
  strcpy(distr_name, "icat cys [icat]");
  // HENRY: add icatType optional argument
  return new ICATMixtureDistr(charge, distr_name, "icat", icatType);

}
  
GlycMixtureDistr* MixtureDistrFactory::getGlycMixtureDistr(int charge) {
  return new GlycMixtureDistr(charge, "N glyc motif [glyc]", "glyc");
}

PhosphoMixtureDistr* MixtureDistrFactory::getPhosphoMixtureDistr(int charge) {
  return new PhosphoMixtureDistr(charge, "Phospho motif [phospho]", "phospho");
}

pIMixtureDistr* MixtureDistrFactory::getPIMixtureDistr(int charge) {
  Boolean variable = True;
  char distr_name[500];
  if(variable) {
    //    strcpy(distr_name, "var offset calc pI [pI]");
    //    return new VariableOffsetpIMixtureDistr(charge, distr_name, "pI", 14.0, 1.0, 0.0);
    strcpy(distr_name, "kernel density calc pI [pI]");
    return new KernelDensityPIMixtureDistr(charge, distr_name, "pI", 14.0, 1.0, 0.0);
  }
  strcpy(distr_name, "calc pI [pI]");
  return new pIMixtureDistr(charge, distr_name, "pI");

}

//RTMixtureDistr* MixtureDistrFactory::getRTMixtureDistr(int charge) {
RTMixtureDistr* MixtureDistrFactory::getRTMixtureDistr(int charge) {
  // Boolean variable = True;
  
  char distr_name[500];
  strcpy(distr_name, "kernel density SSRCalc RT [RT]");
  return new KernelDensityRTMixtureDistr(charge, distr_name, "RT");


   //if(variable) {
   //  strcpy(distr_name, "var offset calc RT [RT]");
   //  return new VariableOffsetRTMixtureDistr(charge, distr_name, "RT", 14.0, 1.0, 0.0);
   //}
   //strcpy(distr_name, "calc RT [RT]");
   //return new RTMixtureDistr(charge, distr_name, "RT");
   
   
   //char distr_name[500];
     
}

