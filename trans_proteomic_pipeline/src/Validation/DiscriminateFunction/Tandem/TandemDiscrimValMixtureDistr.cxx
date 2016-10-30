#include "TandemDiscrimValMixtureDistr.h"

/*

Program       : TandemDiscrimValMixtureDistr for PeptideProphet                                                       
Author        : Brendan MacLean <bmaclean%at%fhcrc.org>                                                       
Date          : 06.20.06 

Copyright (C) 2006 Brendan MacLean and Andrew Keller

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

#include <map>
#include <string>

using namespace std;

// Using class static was causing problems for VC++ 6.0 build.
static map<string, TandemDiscrimFunctionFactory*>* pFactoryMap_ = NULL;

static void cleanup() { // keep memleak detectors happy
   if (pFactoryMap_) {
      delete pFactoryMap_;
      pFactoryMap_ = NULL;
   }
}

void TandemDiscrimValMixtureDistr::registerDiscrimFunctionFactory(TandemDiscrimFunctionFactory* factory) {
   if (pFactoryMap_ == NULL) {
    pFactoryMap_ = new map<string, TandemDiscrimFunctionFactory*>();
    atexit(cleanup);
   }
  string name = factory->getName();
  pFactoryMap_->insert(make_pair(name, factory));
}

// TODO: Figure out a better way to make sure these get linked in.
extern void linkTandemNative();
extern void linkTandemKscore();
extern void linkTandemHRKscore();
//extern void linkTandemHscore();

TandemDiscrimValMixtureDistr::TandemDiscrimValMixtureDistr(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof) {
  init(charge, engine, name, tag, maldi, qtof, False, False);
}

TandemDiscrimValMixtureDistr::TandemDiscrimValMixtureDistr(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean gamma) { 
  init(charge, engine, name, tag, maldi, qtof, gamma, False);
}

TandemDiscrimValMixtureDistr::TandemDiscrimValMixtureDistr(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean gamma, Boolean nonparam) { 
  init(charge, engine, name, tag, maldi, qtof, gamma, nonparam);
} 
TandemDiscrimValMixtureDistr::TandemDiscrimValMixtureDistr(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean gamma, Boolean nonparam, Boolean use_expect) { 
  init(charge, engine, name, tag, maldi, qtof, gamma, nonparam, use_expect);
} 

void TandemDiscrimValMixtureDistr::init(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean gamma) {
  init(charge, engine, name, tag, maldi, qtof, gamma, False);
}

void TandemDiscrimValMixtureDistr::init(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean gamma, Boolean nonparam) {
  init(charge, engine, name, tag, maldi, qtof, gamma, False, False);
}
void TandemDiscrimValMixtureDistr::init(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean gamma, Boolean nonparam, Boolean use_expect) {

  initializeDistr(charge, name, tag);
  engine_ = engine;
  all_negs_ = False;
  maldi_ = maldi;
  maxdiff_ = 0.002;
  nonparam_ = nonparam;
  use_expect_ = use_expect;
  
  gamma_ = gamma;
  qtof_ = qtof;
  gammapos_ = False;

  linkTandemNative();
  linkTandemKscore();
  linkTandemHRKscore();
  //  linkTandemHscore();

  string algorithmName = "native";
  const char* start = strchr(engine, '(');
  if (start != NULL) {
      algorithmName = start + 1;
      algorithmName.assign(start + 1, strlen(start) - 2);  // Remove '(' and ')'
  }
  algorithmName_ = algorithmName;

  map<string, TandemDiscrimFunctionFactory*>::iterator it = pFactoryMap_->find(algorithmName);

  if (it == pFactoryMap_->end()) {
    cerr << "Error: The X! Tandem scoring algorithm '" << algorithmName << "' is not supported.";
    exit(1);
  }

  discrim_func_ = it->second->createDiscrimFunction(charge, use_expect);

  doublevals_ = new Array<double>;
 
  if (nonparam_) {
    negdistr_ = new NonParametricDistribution(0.5, True, False);
    posdistr_ = new NonParametricDistribution(0.5, False, False);
  }
  else {
    if(gammapos_) 
      posdistr_ = new GammaDistribution(maxdiff_);
    else
      posdistr_ = new GaussianDistribution(maxdiff_);
    //posdistr_ = new ExtremeValueDistribution(maxdiff_);
    
    if (gamma_) {
      negdistr_ = new GammaDistribution(maxdiff_);
    } else {
      negdistr_ = new ExtremeValueDistribution(maxdiff_);
    }
  }

  double singlyposmaldiprior[] = {5.0, 0.6};
  double singlynegmaldiprior[] = {1.0, 1.5, -5.0};

  // single score
  double singlyposprior[] = {1.87, 1.146};//{0.87, 1.146};
  double doublyposprior[] = {4.46, 2.08}; //{1.109, 1.767};
  double triplyposprior[] = {4.01, 2.53}; //{0.411, 1.545};
  double quadposprior[] = {4.01, 2.53}; //{0.411, 1.545};
  double pentposprior[] = {4.01, 2.53}; //{0.411, 1.545};
  double hexposprior[] = {4.01, 2.53}; //{0.411, 1.545};
  double septposprior[] = {4.01, 2.53}; //{0.411, 1.545};

  double singlynegprior[] = {-0.24, 0.956};
  double doublynegprior[] = {-0.36, 0.86}; //{-2.468, 0.374};
  double triplynegprior[] = {-0.2, 0.86}; //{-2.179, 0.363};
  double quadnegprior[] = {-0.2, 0.86}; //{-2.179, 0.363};
  double pentnegprior[] = {-0.2, 0.86}; //{-2.179, 0.363};
  double hexnegprior[] = {-0.2, 0.86}; //{-2.179, 0.363};
  double septnegprior[] = {-0.2, 0.86}; //{-2.179, 0.363};
 
  double singlyneggammaprior[] = {4.76, 1.0, -5.0};
  double doublyneggammaprior[] = {4.64, 0.8, -5.0};
  double triplyneggammaprior[] = {4.8, 0.8, -5.0};
  double quadneggammaprior[] = {4.8, 0.8, -5.0};
  double pentneggammaprior[] = {4.8, 0.8, -5.0};
  double hexneggammaprior[] = {4.8, 0.8, -5.0};
  double septneggammaprior[] = {4.8, 0.8, -5.0};



  double doublyposgamma[] = {4.87, 26.0, -3.34 };
  double triplyposgamma[] = {3.9, 16.8, -2.95 };
  double quadposgamma[] = {3.9, 16.8, -2.95 };
  double pentposgamma[] = {3.9, 16.8, -2.95 };
  double hexposgamma[] = {3.9, 16.8, -2.95 };
  double septposgamma[] = {3.9, 16.8, -2.95 };
  

  negmean_ = -1.0;
  MIN_NUM_PSEUDOS_ = 50;
  ZERO_SET_ = 100; // ?
  NUM_DEVS_ = 6;
  USE_TR_NEG_DISTR_ = False;
  posinit_ = NULL;
  neginit_ = NULL;

  if(charge == 0) {

    if(maldi_) {
      posinit_ = copy(singlyposmaldiprior, 2);
    }
    else {
      posinit_ = copy(singlyposprior, 2);
    }

    if(maldi_) {
      neginit_ = copy(singlynegmaldiprior, 2);
    }
    else if (gamma_) {
      neginit_ = copy(singlyneggammaprior, 3);
    }
    else {
      neginit_ = copy(singlynegprior, 2);
    }

  }
  else if(charge == 1) {
    if(gammapos_)
      posinit_ = copy(doublyposgamma, 3);
    else
      posinit_ = copy(doublyposprior, 2);
    if (gamma_)  
      neginit_ = copy(doublyneggammaprior, 3);
    else   
      neginit_ = copy(doublynegprior, 2);
  }
  else if(charge == 2) {
    if(gammapos_)
      posinit_ = copy(triplyposgamma, 3);
    else
      posinit_ = copy(triplyposprior, 2);
    if (gamma_)
      neginit_ = copy(triplyneggammaprior, 3);
    else     
      neginit_ = copy(triplynegprior, 2);
  }
  else if(charge == 3) {
    if(gammapos_)
      posinit_ = copy(quadposgamma, 3);
    else
      posinit_ = copy(quadposprior, 2);
    if (gamma_)
      neginit_ = copy(quadneggammaprior, 3);
    else
      neginit_ = copy(quadnegprior, 2);
  }
  else if(charge == 4) {
    if(gammapos_)
      posinit_ = copy(pentposgamma, 3);
    else
      posinit_ = copy(pentposprior, 2);
    if (gamma_) 
      neginit_ = copy(pentneggammaprior, 3);
    else 
      neginit_ = copy(pentnegprior, 2);
  }
  else if(charge == 5) {
    if(gammapos_)
      posinit_ = copy(hexposgamma, 3);
    else
      posinit_ = copy(hexposprior, 2);
    if (gamma_) 
      neginit_ = copy(hexneggammaprior, 3);
    else 
      neginit_ = copy(hexnegprior, 2);
  }
  else if(charge == 6) {
    if(gammapos_)
      posinit_ = copy(septposgamma, 3);
    else
      posinit_ = copy(septposprior, 2);
    if (gamma_) 
      neginit_ = copy(septneggammaprior, 3);
    else 
      neginit_ = copy(septnegprior, 2);
  }

  reset();

}


void TandemDiscrimValMixtureDistr::setDiscrimFunction(const char* mass_spec_type) {
    ((TandemDiscrimFunction*)discrim_func_)->setMassSpecType(mass_spec_type);
}

Boolean TandemDiscrimValMixtureDistr::update(Array<double>* probs) {
  Boolean rtn = DiscrimValMixtureDistr::update(probs);
  negmean_ = ((ContinuousDistribution*)negdistr_)->getMean()+consStdev_*((ContinuousDistribution*)negdistr_)->getStdev();
  return rtn;
}

double TandemDiscrimValMixtureDistr::getPosProb(int index) { 
  if(all_negs_ || (*doublevals_)[index] < negmean_) {
    return 0.0;
  }
  if (nonparam_) {
    return MixtureDistr::getPosProb(index);
  }
  if (gamma_) {
    if (!((GammaDistribution*)(negdistr_))->aboveMin((*doublevals_)[index])) return (0.0);
  } else {
    if(! ((ExtremeValueDistribution*)(negdistr_))->aboveMin((*doublevals_)[index])) return (0.0);
  }

  return MixtureDistr::getPosProb(index);
}

double TandemDiscrimValMixtureDistr::getNegProb(int index) {
  if(all_negs_ || (*doublevals_)[index] < negmean_) {
    return 1.0;
  }
  
  if (nonparam_) {
    return MixtureDistr::getNegProb(index);
  }

  if (gamma_) {
    if (!((GammaDistribution*)(negdistr_))->aboveMin((*doublevals_)[index])) return (1.0);
  } else {
    if(! ((ExtremeValueDistribution*)(negdistr_))->aboveMin((*doublevals_)[index])) return (1.0);
  }

  return MixtureDistr::getNegProb(index);
}

Boolean TandemDiscrimValMixtureDistr::initializeNegDistribution(NTTMixtureDistr* nttdistr) {

  if (gamma_) {
    return (initializeNegGammaDistribution(nttdistr));
  } else {
    return (initializeNegExtremeValueDistribution(nttdistr));
  }
}
Boolean TandemDiscrimValMixtureDistr::initializeNegDistribution(Array<Boolean>* isdecoy) {
  if (nonparam_) {
    isdecoy_ = isdecoy;
    ((NonParametricDistribution*)(posdistr_))->initWithDecoy(isdecoy, doublevals_);
    ((NonParametricDistribution*)(negdistr_))->initWithDecoy(isdecoy, doublevals_);
    return True;
  }
  if (gamma_) {
    return (initializeNegGammaDistribution(isdecoy));
  } else {
    return (initializeNegExtremeValueDistribution(isdecoy));
  }
}

Boolean TandemDiscrimValMixtureDistr::initializeNegGammaDistribution(NTTMixtureDistr* nttdistr) {

  // never use NTT=0 to initialize negative distribution

  // cerr << "HERE" << std::endl;

  double mean = 0.0;
  double stdev = 0.0;
  int tot = 0;
  double totsq = 0.0;

  // for single score guassian
  double posmean[] = { 1.87, 4.46, 4.01 , 4.01, 4.01, 4.01, 4.01};
  double posstdev[] = { 1.146, 2.08, 2.53, 2.53, 2.53, 2.53, 2.53 };

  double negmean[] = {4.76, 4.64, 4.8, 4.8, 4.8, 4.8, 4.8};
  double negstdev[] = {0.956, 0.86, 0.86, 0.86, 0.86, 0.86, 0.86};

  // values for truncating negative distribution
  if (charge_ != 0) {
    ((GammaDistribution*)(negdistr_))->
    setDistrMinval(((TandemDiscrimFunction*)discrim_func_)->getMinVal());    
  }

  double MAX_SINGLY_NEGMEAN = 1.0;

  double negmean_num_stds[] = {-0.3, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // by charge

  int k;

  double zero = -5.0;

  for(k = 0; k < getNumVals(); k++) {
    if((*doublevals_)[k] > zero && (*doublevals_)[k] < posmean[charge_] - posstdev[charge_]) {
      mean += (*doublevals_)[k] - zero;
      totsq += ((*doublevals_)[k] - zero) * ((*doublevals_)[k] - zero);
      tot++;
    }
  } // next

  if(tot > 0) {
    mean /= tot;
    stdev = totsq / tot - mean * mean;
  }
  else {

    mean = negmean[charge_];
    stdev = negstdev[charge_];

  }

  double* newsettings = new double[3];
  newsettings[0] = mean;
  newsettings[1] = stdev;
  newsettings[2] = zero;
  
  negdistr_->init(newsettings, False);
  delete [] newsettings;
  
  negmean_ = zero + mean - negmean_num_stds[charge_] * sqrt(stdev);
  if(negmean_ > MAX_SINGLY_NEGMEAN) {
    negmean_ = MAX_SINGLY_NEGMEAN;
  }
  
  USE_TR_NEG_DISTR_ = True;

  //  cerr << "negmean = " << negmean_ << std::endl;
  return False; // done


}


Boolean TandemDiscrimValMixtureDistr::initializeNegExtremeValueDistribution(NTTMixtureDistr* nttdistr) {

  // never use NTT=0 to initialize negative distribution

  // cerr << "HERE" << std::endl;

  double mean = 0.0;
  double stdev = 0.0;
  int tot = 0;
  double totsq = 0.0;

  // for single score guassian
  double posmean[] = { 1.87, 4.46, 4.01 , 4.01, 4.01, 4.01, 4.01};
  double posstdev[] = { 1.146, 2.08, 2.53, 2.53, 2.53, 2.53, 2.53 };

  //  double negmean[] = {4.76, 4.64, 4.8, 4.8, 4.8};
  //double negstdev[] = {23, 23, 23, 23, 23};

  double negmean[] = {-0.24, -0.36, -0.2, -0.2, -0.2, -0.2, -0.2};
  double negstdev[] = {0.956, 0.86, 0.86, 0.86, 0.86, 0.86, 0.86};



  // values for truncating negative distribution
  if (charge_ != 0) {
    ((ExtremeValueDistribution*)(negdistr_))->
    setDistrMinval(((TandemDiscrimFunction*)discrim_func_)->getMinVal());    
  }

  double MAX_SINGLY_NEGMEAN = 1.0;

  double negmean_num_stds[] = {-0.3, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // by charge

  int k;

  for(k = 0; k < getNumVals(); k++) {
    if((*doublevals_)[k] < posmean[charge_] - posstdev[charge_]) {
      mean += (*doublevals_)[k];
      totsq += ((*doublevals_)[k]) * ((*doublevals_)[k]);
      tot++;
    }
  } // next

  if(tot > 0) {
    mean /= tot;
    stdev = totsq / tot - mean * mean;
  }
  else {

    mean = negmean[charge_];
    stdev = negstdev[charge_];

  }

  double* newsettings = new double[3];
  newsettings[0] = mean;
  newsettings[1] = stdev;
  
  negdistr_->init(newsettings);
  delete [] newsettings;
  
  negmean_ =  mean - negmean_num_stds[charge_] * sqrt(stdev);
  if(negmean_ > MAX_SINGLY_NEGMEAN) {
    negmean_ = MAX_SINGLY_NEGMEAN;
  }
  
  USE_TR_NEG_DISTR_ = True;

  cerr << "negmean = " << negmean_ << std::endl;
  return False; // done
}

Boolean TandemDiscrimValMixtureDistr::initializeNegGammaDistribution(Array<Boolean>* isdecoy) {
  if (nonparam_) {
    return True;
  }

  //cerr << "HERE" << endl;

  double mean = 0.0;
  double stdev = 0.0;
  int tot = 0;
  double totsq = 0.0;

  // for single score guassian
  double posmean[] = { 1.87, 4.46, 4.01 , 4.01, 4.01, 4.01, 4.01};
  double posstdev[] = { 1.146, 2.08, 2.53, 2.53, 2.53, 2.53, 2.53 };

  double negmean[] = {4.76, 4.64, 4.8, 4.8, 4.8, 4.8, 4.8};
  double negstdev[] = {0.956, 0.86, 0.86, 0.86, 0.86, 0.86, 0.86};

  // values for truncating negative distribution
  if (charge_ != 0) {
    ((GammaDistribution*)(negdistr_))->
    setDistrMinval(((TandemDiscrimFunction*)discrim_func_)->getMinVal());    
  }

  double MAX_SINGLY_NEGMEAN = 1.0;

  double negmean_num_stds[] = {-0.3, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // by charge

  int k;

  double zero = -5.0;
  double pmean = 0.0;
  double pstdev = 0.0;
  double ptotsq = 0.0;
  int ptot = 0; 
  for(k = 0; k < getNumVals(); k++) {
    if((*isdecoy)[k]) {
      mean += (*doublevals_)[k]-zero;
      totsq += ((*doublevals_)[k]-zero) * ((*doublevals_)[k]-zero);
      tot++;
    }   
    else {
      pmean += (*doublevals_)[k];
      ptotsq += (*doublevals_)[k]  * (*doublevals_)[k];
      ptot++;
    }
  } // next

  if(tot > 0) {
    mean /= tot;
    stdev = totsq / tot - mean * mean;
    stdev = sqrt(stdev);
  }
  else {
    mean = negmean[charge_];
    stdev = negstdev[charge_];
    USE_TR_NEG_DISTR_ = True;

  }

  if(ptot > 0) {
    pmean /= ptot;
    pstdev = ptotsq / ptot - pmean * pmean;
    pstdev = sqrt(pstdev);
  }
  else {
    pmean = posmean[charge_];
    pstdev = posstdev[charge_];
  }
  //  cerr << "pmean: " << pmean << " pstdev:" <<  pstdev << endl;
  
  double* newsettings = new double[3];
  newsettings[0] = mean;
  newsettings[1] = stdev;
  newsettings[2] = zero;
  
  negdistr_->init(newsettings, False);
  delete [] newsettings;
  
  negmean_ = zero + mean - negmean_num_stds[charge_] * stdev;
  if(negmean_ > MAX_SINGLY_NEGMEAN) {
    negmean_ = MAX_SINGLY_NEGMEAN;
  }
  

  setPositiveDistr(pmean, pstdev, 1);
  ((ContinuousDistribution*)negdistr_)->setTot(1);
  //((ContinuousDistribution*)posdistr_)->setTot(1);
  while(noDistr()) {
    pmean += 0.5;
    setPositiveDistr(pmean, pstdev, 1);
  }


  //cerr << "negmean = " << negmean_ << endl;
  return False; // done


}


Boolean TandemDiscrimValMixtureDistr::initializeNegExtremeValueDistribution(Array<Boolean>* isdecoy) {
  if (nonparam_) {
    return True;
  } 
  //assert(nttdistr->getNumVals() == getNumVals());
  double mean = 0.0;
  double stdev = 0.0;
  int tot = 0;
  double totsq = 0.0;

  // evd for discriminatn score
  //double posmean[] = { 2.0, 4.102, 4.563 };
  //double posstdev[] = { 0.4, 1.64, 1.84 };

  // for single score guassian
  double posmean[] = { 1.87, 4.46, 4.01 , 4.01, 4.01, 4.01, 4.01};
  double posstdev[] = { 1.146, 2.08, 2.53, 2.53, 2.53, 2.53, 2.53 };

  double negmean[] = {-0.24, -0.36, -0.2, -0.2, -0.2, -0.2, -0.2};
  double negstdev[] = {0.956, 0.86, 0.86, 0.86, 0.86, 0.86, 0.86};

 

  // values for truncating negative distribution
  if (charge_ != 0) {
    ((ExtremeValueDistribution*)(negdistr_))->setDistrMinval(((TandemDiscrimFunction*)discrim_func_)->getMinVal());
  }
  double zero = -5.0;
  double pmean = 0.0;
  double pstdev = 0.0;
  double ptotsq = 0.0;
  int ptot = 0; 

  double MAX_SINGLY_NEGMEAN = 1.0;

  //double negmean_num_stds[] = {-1.0, 0.5, 0.5}; // by charge
  //double negmean_num_stds[] = {-0.1, 1.0, 1.0}; // by charge
  double negmean_num_stds[] = {-0.3, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // by charge

  int k;

  for(k = 0; k < getNumVals(); k++) {
    //Use only decoy values to init the negative, 
    //the values have to be greater than the minimum value of the distribution
    if((*isdecoy)[k]) {
      mean += (*doublevals_)[k];
      totsq += (*doublevals_)[k] * (*doublevals_)[k];
      tot++;
    }
    else {
      pmean += (*doublevals_)[k];
      ptotsq += (*doublevals_)[k]  * (*doublevals_)[k];
      ptot++;
    }
  }
    
  if(tot < MIN_NUM_PSEUDOS_) {
    tot = 0; // restart
    mean = 0.0;
    totsq = 0.0;
    for(int k = 0; k < getNumVals(); k++) {
      if((*doublevals_)[k] < posmean[charge_] - posstdev[charge_]) {
	mean += (*doublevals_)[k];
	totsq += ((*doublevals_)[k]) * ((*doublevals_)[k]);
	tot++;
      }
    } // next

    if(tot > 0) {
      mean /= tot;
      stdev = totsq / tot - mean * mean;
    }
    else {
      mean = negmean[charge_];
      stdev = negstdev[charge_];
    }


  
    double* newsettings = new double[2];
    newsettings[0] = mean;
    newsettings[1] = stdev;
    newsettings[1] = sqrt(newsettings[1]);
    negdistr_->init(newsettings);
    delete [] newsettings;


    negmean_ = mean - negmean_num_stds[charge_] * stdev;
    if(negmean_ > MAX_SINGLY_NEGMEAN) {
      negmean_ = MAX_SINGLY_NEGMEAN;
    }

    USE_TR_NEG_DISTR_ = True;
    return False; // done
  } // if not enough pseudos
  
  if(ptot > 0) {
    pmean /= ptot;
    pstdev = ptotsq / ptot - pmean * pmean;
    pstdev = sqrt(pstdev);
  }
  else {
    pmean = posmean[charge_];
    pstdev = posstdev[charge_];
  }
  
  mean /= tot;
  stdev = (totsq / tot) - mean * mean;
  stdev = sqrt(stdev); 

  negmean_ = mean - negmean_num_stds[charge_] * stdev;
  
  double newposmean = pmean;
  double newposstdev = pstdev;
  
  setNegativeDistr(negmean_, stdev);
  setPositiveDistr(newposmean, newposstdev,1);
  ((ContinuousDistribution*)negdistr_)->setTot(1);
  //  ((ContinuousDistribution*)posdistr_)->setTot(1);
  if (charge_ == 0 && getNumVals() < 500) {
    return True;
  }
  else {
    while(negmean_ + stdev > newposmean - newposstdev) {
      newposmean += 0.5;
      setPositiveDistr(newposmean, newposstdev, 1);
    }
  }

  return True;
}






void TandemDiscrimValMixtureDistr::setNegativeDistr(double mean, double stdev, double tot) {
  assert(!gamma_);
  double* next = new double[2];
  next[0] = mean;
  next[1] = stdev;
  negmean_ = mean;
  negdistr_->init(next);
  ((ContinuousDistribution*)negdistr_)->setTot(tot);
  delete[] next;
}

void TandemDiscrimValMixtureDistr::setNegativeDistr(double mean, double stdev) {
  assert(!gamma_);
  double* next = new double[2];
  next[0] = mean;
  next[1] = stdev;
  negmean_ = mean;
  negdistr_->init(next);
  delete[] next;
}

double TandemDiscrimValMixtureDistr::getMinVal() {

  if (algorithmName_ == "h-score") {
    return (-8.0);
  } else {
    return (DiscrimValMixtureDistr::getMinVal());
  }
  
}

double TandemDiscrimValMixtureDistr::getMaxVal() {

  if (algorithmName_ == "h-score") {
    return (12.0);
  } else {
    return (DiscrimValMixtureDistr::getMaxVal());
  }

}
