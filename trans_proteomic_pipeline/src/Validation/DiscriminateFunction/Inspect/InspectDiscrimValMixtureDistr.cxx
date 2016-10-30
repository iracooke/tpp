#include "InspectDiscrimValMixtureDistr.h"

/*
 * WARNING!! This discriminant function is not yet complete.  It is presented
 *           here to help facilitate trial and discussion.  Reliance on this
 *           code for publishable scientific results is not recommended.
 */

/*

Program       : InspectDiscrimValMixtureDistr for PeptideProphet                                                       
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
static bool bFirstCall=true;

// Using class static was causing problems for VC++ 6.0 build.
static map<string, InspectDiscrimFunctionFactory*>* pFactoryMap_ = NULL;

void InspectDiscrimValMixtureDistr::registerDiscrimFunctionFactory(InspectDiscrimFunctionFactory* factory) {
  if (pFactoryMap_ == NULL)
    pFactoryMap_ = new map<string, InspectDiscrimFunctionFactory*>();
  string name = factory->getName();
  pFactoryMap_->insert(make_pair(name, factory));
}

// TODO: Figure out a better way to make sure these get linked in.
//extern void linkInspectPscore();
//extern void linkInspectMQscore();
extern void linkInspectFscore();

InspectDiscrimValMixtureDistr::InspectDiscrimValMixtureDistr(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof) {
  init(charge, engine, name, tag, maldi, qtof, False, True);													   
}																												   
																												   
InspectDiscrimValMixtureDistr::InspectDiscrimValMixtureDistr(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean gamma) { 
  init(charge, engine, name, tag, maldi, qtof, gamma, True);
} 

InspectDiscrimValMixtureDistr::InspectDiscrimValMixtureDistr(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean gamma, Boolean nonparam) { 
  init(charge, engine, name, tag, maldi, qtof, gamma, nonparam);
} 
void InspectDiscrimValMixtureDistr::init(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean gamma) {
  init(charge, engine, name, tag, maldi, qtof, gamma, True);
}
void InspectDiscrimValMixtureDistr::init(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean gamma, Boolean nonparam) {
  if (bFirstCall) {
	bFirstCall = false;
	// put the text of the "README" file where users can actually be aware of it - BSP
	std::cout << "WARNING!! The discriminant function for Inspect is not yet complete.  It is presented "
			"here to help facilitate trial and discussion.  Reliance on this "
			"code for publishable scientific results is not recommended." << std::endl;
  }
  initializeDistr(charge, name, tag);
  engine_ = engine;
  all_negs_ = False;
  maldi_ = maldi;
  maxdiff_ = 0.002;

  min_dataval_ = 999.0;

  gamma_ = False;
  //gamma_ = gamma;

  qtof_ = qtof;
  gammapos_ = False;
  
  if (!nonparam) {
    cerr << "WARNING: Inspect only support semi-parametric PeptideProphet modelling, which relies on a DECOY search." << endl;
  }
  nonparam_ = True; //TODO: clean up code from parameteric stuff that doesn't apply to Inspect
  //  nonparam_ = nonparam;

  //  linkInspectPscore();
  //  linkInspectMQscore();
  linkInspectFscore();

  string algorithmName = "zscore";
  const char* start = strchr(engine, '(');
  if (start != NULL) {
      algorithmName = start + 1;
      algorithmName.assign(start + 1, strlen(start) - 2);  // Remove '(' and ')'
  }
  algorithmName_ = algorithmName;

  map<string, InspectDiscrimFunctionFactory*>::iterator it = pFactoryMap_->find(algorithmName);

  if (it == pFactoryMap_->end()) {
    cerr << "Error: Inspect scoring algorithm '" << algorithmName << "' is not supported.";
    exit(1);
  }

  discrim_func_ = it->second->createDiscrimFunction(charge);

  doublevals_ = new Array<double>;
  
 
  if (nonparam_) {
    negdistr_ = new NonParametricDistribution(1, True, False);
    posdistr_ = new NonParametricDistribution(1, False, False);
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

Boolean InspectDiscrimValMixtureDistr::update(Array<double>* probs) {
  Boolean rtn = DiscrimValMixtureDistr::update(probs);
  negmean_ = ((ContinuousDistribution*)negdistr_)->getMean()+consStdev_*((ContinuousDistribution*)negdistr_)->getStdev();
  return rtn;
}

void InspectDiscrimValMixtureDistr::setDiscrimFunction(const char* mass_spec_type) {
    ((InspectDiscrimFunction*)discrim_func_)->setMassSpecType(mass_spec_type);
}


double InspectDiscrimValMixtureDistr::getPosProb(int index) { 
 

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

double InspectDiscrimValMixtureDistr::getNegProb(int index) {

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


Boolean InspectDiscrimValMixtureDistr::initializeNegDistribution(Array<Boolean>* isdecoy) {
  if (nonparam_) {
    isdecoy_ = isdecoy;
    ((NonParametricDistribution*)(posdistr_))->initWithDecoy(isdecoy, doublevals_);
    ((NonParametricDistribution*)(negdistr_))->initWithDecoy(isdecoy, doublevals_);
    return True;
  }
  return false;
}

double InspectDiscrimValMixtureDistr::getMinVal() {
  return (DiscrimValMixtureDistr::getMinVal());
  
}

double InspectDiscrimValMixtureDistr::getMaxVal() {
  return (DiscrimValMixtureDistr::getMaxVal());
}


