#include "MascotDiscrimValMixtureDistr.h"

/*

Program       : MascotDiscrimValMixtureDistr for PeptideProphet                                                       
Author        : Andrew Keller <akeller@systemsbiology.org>                                                       
Date          : 11.27.02 

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

MascotDiscrimValMixtureDistr::MascotDiscrimValMixtureDistr(int charge, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean nonparam) {  
  initializeDistr(charge, name, tag);
  all_negs_ = False;
  maldi_ = maldi;
  maxdiff_ = 0.002;
  gamma_ = False;
  qtof_ = qtof;
  gammapos_ = False;
  xmlfile_ = NULL;
  nonparam_= nonparam;
  //if(charge > 0)
  //gammapos_ = True;


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
    
    negdistr_ = new ExtremeValueDistribution(maxdiff_);
  }
  // evd with discriminant score (Mascot + delta)
  //double singlyposprior[] = {2.0, 0.4};
  //double doublyposprior[] = {3.754, 2.3};
  //double triplyposprior[] = {4.117, 2.9};
  //double singlynegprior[] = {-2.0, 0.9};
  //double doublynegprior[] = {-0.377, 0.64};
  //double triplynegprior[] = {-0.224, 0.67};

  double singlyposmaldiprior[] = {5.0, 0.6};
  double singlynegmaldiprior[] = {1.0, 1.5};

  // single score
  double singlyposprior[] = {2.0, 1.0};
  //  double singlyposprior[] = {-0.191, 0.833};
  double doublyposprior[] = {4.213, 2.00}; //{1.109, 1.767};
  double triplyposprior[] = {4.135, 2.75}; //{0.411, 1.545};
  double quadposprior[] = {4.135, 2.75}; //{0.411, 1.545};
  double pentposprior[] = {4.135, 2.75}; //{0.411, 1.545};
  double hexposprior[] = {4.135, 2.75}; //{0.411, 1.545};
  double septposprior[] = {4.135, 2.75}; //{0.411, 1.545};

  double singlynegprior[] = {-1.748, 0.539};
  double doublynegprior[] = {-0.366, 0.86}; //{-2.468, 0.374};
  double triplynegprior[] = {-0.235, 0.79}; //{-2.179, 0.363};
  double quadnegprior[] = {-0.235, 0.79}; //{-2.179, 0.363};
  double pentnegprior[] = {-0.235, 0.79}; //{-2.179, 0.363};
  double hexnegprior[] = {-0.235, 0.79}; //{-2.179, 0.363};
  double septnegprior[] = {-0.235, 0.79}; //{-2.179, 0.363};
 
  double doublyposgamma[] = {4.87, 26.0, -3.34 };
  double triplyposgamma[] = {3.9, 16.8, -2.95 };
  double quadposgamma[] = {3.9, 16.8, -2.95 };
  double pentposgamma[] = {3.9, 16.8, -2.95 };
  double hexposgamma[] = {3.9, 16.8, -2.95 };
  double septposgamma[] = {3.9, 16.8, -2.95 };

  negmean_ = 0.0;
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
    else {
      neginit_ = copy(singlynegprior, 2);
    }

  }
  else if(charge == 1) {
    if(gammapos_)
      posinit_ = copy(doublyposgamma, 3);
    else
      posinit_ = copy(doublyposprior, 2);
    neginit_ = copy(doublynegprior, 2);
  }
  else if(charge == 2) {
    if(gammapos_)
      posinit_ = copy(triplyposgamma, 3);
    else
      posinit_ = copy(triplyposprior, 2);
    neginit_ = copy(triplynegprior, 2);
  }
  else if(charge == 3) {
    if(gammapos_)
      posinit_ = copy(quadposgamma, 3);
    else
      posinit_ = copy(quadposprior, 2);
    neginit_ = copy(quadnegprior, 2);
  }
  else if(charge == 4) {
    if(gammapos_)
      posinit_ = copy(pentposgamma, 3);
    else
      posinit_ = copy(pentposprior, 2);
    neginit_ = copy(pentnegprior, 2);
  }
  else if(charge == 5) {
    if(gammapos_)
      posinit_ = copy(hexposgamma, 3);
    else
      posinit_ = copy(hexposprior, 2);
    neginit_ = copy(hexnegprior, 2);
  }
  else if(charge == 6) {
    if(gammapos_)
      posinit_ = copy(septposgamma, 3);
    else
      posinit_ = copy(septposprior, 2);
    neginit_ = copy(septnegprior, 2);
  }
  reset();
  
}

MascotDiscrimValMixtureDistr::MascotDiscrimValMixtureDistr(int charge, const char* name, const char* tag, Boolean maldi, Boolean qtof, ScoreOptions& opts, Boolean nonparam) {  
  initializeDistr(charge, name, tag);
  all_negs_ = False;
  maldi_ = maldi;
  maxdiff_ = 0.002;
  gamma_ = False;
  qtof_ = qtof;
  gammapos_ = False;
  nonparam_ =  nonparam;
  xmlfile_ = new char[strlen(opts.inputfile_)+1];
  star_ = opts.mascotstar_;
  strcpy(xmlfile_, opts.inputfile_);

  //if(charge > 0)
  //gammapos_ = True;

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
    
    negdistr_ = new ExtremeValueDistribution(maxdiff_);
  }
  // evd with discriminant score (Mascot + delta)
  //double singlyposprior[] = {2.0, 0.4};
  //double doublyposprior[] = {3.754, 2.3};
  //double triplyposprior[] = {4.117, 2.9};
  //double singlynegprior[] = {-2.0, 0.9};
  //double doublynegprior[] = {-0.377, 0.64};
  //double triplynegprior[] = {-0.224, 0.67};

  double singlyposmaldiprior[] = {5.0, 0.6};
  double singlynegmaldiprior[] = {1.0, 1.5};

  // single score
  //  double singlyposprior[] = {-0.191, 0.833};
  double singlyposprior[] = {2.0, 1.0};
  double doublyposprior[] = {4.213, 2.00}; //{1.109, 1.767};
  double triplyposprior[] = {4.135, 2.75}; //{0.411, 1.545};
  double quadposprior[] = {4.135, 2.75}; //{0.411, 1.545};
  double pentposprior[] = {4.135, 2.75}; //{0.411, 1.545};
  double hexposprior[] = {4.135, 2.75}; //{0.411, 1.545};
  double septposprior[] = {4.135, 2.75}; //{0.411, 1.545};

  double singlynegprior[] = {-1.748, 0.539};
  double doublynegprior[] = {-0.366, 0.86}; //{-2.468, 0.374};
  double triplynegprior[] = {-0.235, 0.79}; //{-2.179, 0.363};
  double quadnegprior[] = {-0.235, 0.79}; //{-2.179, 0.363};
  double pentnegprior[] = {-0.235, 0.79}; //{-2.179, 0.363};
  double hexnegprior[] = {-0.235, 0.79}; //{-2.179, 0.363};
  double septnegprior[] = {-0.235, 0.79}; //{-2.179, 0.363};
 
  double doublyposgamma[] = {4.87, 26.0, -3.34 };
  double triplyposgamma[] = {3.9, 16.8, -2.95 };
  double quadposgamma[] = {3.9, 16.8, -2.95 };
  double pentposgamma[] = {3.9, 16.8, -2.95 };
  double hexposgamma[] = {3.9, 16.8, -2.95 };
  double septposgamma[] = {3.9, 16.8, -2.95 };

  negmean_ = 0.0;
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
    else {
      neginit_ = copy(singlynegprior, 2);
    }

  }
  else if(charge == 1) {
    if(gammapos_)
      posinit_ = copy(doublyposgamma, 3);
    else
      posinit_ = copy(doublyposprior, 2);
    neginit_ = copy(doublynegprior, 2);
  }
  else if(charge == 2) {
    if(gammapos_)
      posinit_ = copy(triplyposgamma, 3);
    else
      posinit_ = copy(triplyposprior, 2);
    neginit_ = copy(triplynegprior, 2);
  }  
  else if(charge == 3) {
    if(gammapos_)
      posinit_ = copy(quadposgamma, 3);
    else
      posinit_ = copy(quadposprior, 2);
    neginit_ = copy(quadnegprior, 2);
  }
  else if(charge == 4) {
    if(gammapos_)
      posinit_ = copy(pentposgamma, 3);
    else
      posinit_ = copy(pentposprior, 2);
    neginit_ = copy(pentnegprior, 2);
  }
  else if(charge == 5) {
    if(gammapos_)
      posinit_ = copy(hexposgamma, 3);
    else
      posinit_ = copy(hexposprior, 2);
    neginit_ = copy(hexnegprior, 2);
  }
  else if(charge == 6) {
    if(gammapos_)
      posinit_ = copy(septposgamma, 3);
    else
      posinit_ = copy(septposprior, 2);
    neginit_ = copy(septnegprior, 2);
  }
  reset();
}

Boolean MascotDiscrimValMixtureDistr::update(Array<double>* probs) {
  Boolean rtn = DiscrimValMixtureDistr::update(probs);
  negmean_ = ((ContinuousDistribution*)negdistr_)->getMean()+consStdev_*((ContinuousDistribution*)negdistr_)->getStdev();
  return rtn;
}

void MascotDiscrimValMixtureDistr::setDiscrimFunction(const char* mass_spec_type) {
  //  cout << "setting new mascot discrim function...for input file: " << xmlfile_ << endl;
  //  discrim_func_ = new MascotDiscrimFunction(charge_, xmlfile_);
  discrim_func_ = new MascotDiscrimFunction(charge_, xmlfile_, star_);
}

// roll into parent....
Array<Tag*>* MascotDiscrimValMixtureDistr::getMixtureDistrTags(const char* name) {
  char next[500];
  char* fvalinfo = ((MascotDiscrimFunction*)discrim_func_)->getMascotScoreParserDescription();
  if(fvalinfo != NULL) {
    sprintf(next, "%s\t%s\tnegmean: %0.2f", getName(), fvalinfo, negmean_);
    delete fvalinfo;
  }
  else 
    sprintf(next, "%s\tnegmean: %0.2f", getName(), negmean_);
  return MixtureDistr::getMixtureDistrTags(next);
}


double MascotDiscrimValMixtureDistr::getPosProb(int index) { 
  
  if(all_negs_ || (*doublevals_)[index] < negmean_) {
    return 0.0;
  }
  
  if (nonparam_) {
    return MixtureDistr::getPosProb(index);
  }
  
  if(! ((ExtremeValueDistribution*)(negdistr_))->aboveMin((*doublevals_)[index]))
    return 0.0;
  
  return MixtureDistr::getPosProb(index);
}

double MascotDiscrimValMixtureDistr::getNegProb(int index) {
  
  if(all_negs_ || (*doublevals_)[index] < negmean_) {
    return 1.0;
  }
  
  if (nonparam_) {
    return MixtureDistr::getNegProb(index);
  }
  
  if(! ((ExtremeValueDistribution*)(negdistr_))->aboveMin((*doublevals_)[index]))
    return 1.0;
  return MixtureDistr::getNegProb(index);
}

Boolean MascotDiscrimValMixtureDistr::initializeNegDistribution(NTTMixtureDistr* nttdistr) {
  assert(nttdistr->getNumVals() == getNumVals());
  double mean = 0.0;
  double stdev = 0.0;
  int tot = 0;
  double totsq = 0.0;
  //double zero;


  // evd for discriminatn score
  //double posmean[] = { 2.0, 4.102, 4.563 };
  //double posstdev[] = { 0.4, 1.64, 1.84 };

  // for single score guassian
  double posmean[] = { -.191, 1.109, 0.411, 0.411, 0.411, 0.411, 0.411};
  double posstdev[] = { 0.833, 1.77, 1.55, 1.55, 1.55, 1.55, 1.55 };

  // values for truncating negative distribution
  //double minvals[] = { -1.3, -1.5, -1.1 };
  double minvals[] = { -4.0, -2.6, -2.0, -2.0, -2.0, -2.0 , -2.0  };

  if(charge_ == 0)
    ((ExtremeValueDistribution*)(negdistr_))->setDistrMinval(minvals[charge_]);


  double MAX_SINGLY_NEGMEAN = 1.0;

  //double negmean_num_stds[] = {-1.0, 0.5, 0.5}; // by charge
  //double negmean_num_stds[] = {-0.1, 1.0, 1.0}; // by charge

  //double negmean_num_stds[] = {-0.3, 0.0, 0.0}; // by charge
  double negmean_num_stds[] = {5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // by charge

  double min_singly_fval = -2;
  int k;
  for(k = 0; k < getNumVals(); k++) {
    if(nttdistr->isValue(k, 0) && ((ExtremeValueDistribution*)(negdistr_))->aboveMin((*doublevals_)[k])) {
      mean += (*doublevals_)[k];
      totsq += (*doublevals_)[k] * (*doublevals_)[k];
      tot++;
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

    double* newsettings = new double[2];
    newsettings[0] = mean;
    newsettings[1] = stdev;
    newsettings[1] = sqrt(newsettings[1]);
    negdistr_->init(newsettings);
    delete[] newsettings;


    negmean_ = mean - negmean_num_stds[charge_] * stdev;
    if(negmean_ > MAX_SINGLY_NEGMEAN) {
      negmean_ = MAX_SINGLY_NEGMEAN;
    }

    USE_TR_NEG_DISTR_ = True;
    return False; // done
  } // if not enough pseudos
    
  mean /= tot;
  stdev = (totsq / tot) - mean * mean;
  stdev = sqrt(stdev);  // delete this later ?

  negmean_ = mean - negmean_num_stds[charge_] * stdev;

  // now recompute real mean and stdev
  mean = 0.0;
  totsq = 0.0;
  tot = 0;
    
  for(k = 0; k < getNumVals(); k++) {
    if(nttdistr->isValue(k, 0) && ((ExtremeValueDistribution*)(negdistr_))->aboveMin((*doublevals_)[k])) {
      mean += (*doublevals_)[k];
      totsq += ((*doublevals_)[k]) * ((*doublevals_)[k]);
      tot++;
    }
  } // next
  
  if(tot > 0) {
    mean /= tot;
    stdev = (totsq / tot) - mean * mean;
    stdev = sqrt(stdev);
  }
  Boolean reset = False;
  double newposmean = posmean[charge_];
  double newposstdev = posstdev[charge_];;
  while(noDistr()) {
    newposmean += 0.5;
    newposstdev += 0.95;
    reset = True;
  }
  if(reset) {
    setPositiveDistr(newposmean, newposstdev);
  }

  setNegativeDistr(mean, stdev);
  return True;
}

Boolean MascotDiscrimValMixtureDistr::initializeNegDistribution(Array<Boolean>* isdecoy) {
  if (nonparam_) {
    isdecoy_ = isdecoy;
    ((NonParametricDistribution*)(posdistr_))->initWithDecoy(isdecoy, doublevals_);
    ((NonParametricDistribution*)(negdistr_))->initWithDecoy(isdecoy, doublevals_);
    return True;
  }
  // assert(nttdistr->getNumVals() == getNumVals());
  double mean = 0.0;
  double stdev = 0.0;
  int tot = 0;
  double totsq = 0.0;
  //double zero;


  // evd for discriminatn score
  //double posmean[] = { 2.0, 4.102, 4.563 };
  //double posstdev[] = { 0.4, 1.64, 1.84 };

  // for single score guassian
  double posmean[] = { -.191, 1.109, 0.411, 0.411, 0.411, 0.411, 0.411};
  double posstdev[] = { 0.833, 1.77, 1.55, 1.55, 1.55, 1.55, 1.55 };

  // values for truncating negative distribution
  //double minvals[] = { -1.3, -1.5, -1.1 };
  double minvals[] = { -4.0, -2.6, -2.0, -2.0, -2.0, -2.0, -2.0 };

  if(charge_ == 0)
    ((ExtremeValueDistribution*)(negdistr_))->setDistrMinval(minvals[charge_]);


  double MAX_SINGLY_NEGMEAN = 1.0;

  //double negmean_num_stds[] = {-1.0, 0.5, 0.5}; // by charge
  //double negmean_num_stds[] = {-0.1, 1.0, 1.0}; // by charge

  //double negmean_num_stds[] = {-0.3, 0.0, 0.0}; // by charge
  double negmean_num_stds[] = {5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // by charge

  double min_singly_fval = -2;
  int k;
  for (k = 0; k < getNumVals(); k++) {
    if ((*isdecoy)[k]) { 
      mean += (*doublevals_)[k];
      totsq += (*doublevals_)[k] * (*doublevals_)[k];
      tot++;
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

    double* newsettings = new double[2];
    newsettings[0] = mean;
    newsettings[1] = stdev;
    newsettings[1] = sqrt(newsettings[1]);
    negdistr_->init(newsettings);
    delete[] newsettings;


    negmean_ = mean - negmean_num_stds[charge_] * stdev;
    if(negmean_ > MAX_SINGLY_NEGMEAN) {
      negmean_ = MAX_SINGLY_NEGMEAN;
    }

    USE_TR_NEG_DISTR_ = True;
    return False; // done
  } // if not enough pseudos
    
  mean /= tot;
  stdev = (totsq / tot) - mean * mean;
  stdev = sqrt(stdev);  // delete this later ?

  negmean_ = mean - negmean_num_stds[charge_] * stdev;

  // now recompute real mean and stdev
  mean = 0.0;
  totsq = 0.0;
  tot = 0;
    
  for(k = 0; k < getNumVals(); k++) {
    if((*isdecoy)[k]) {
      mean += (*doublevals_)[k];
      totsq += ((*doublevals_)[k]) * ((*doublevals_)[k]);
      tot++;
    }
  } // next
  
  if(tot > 0) {
    mean /= tot;
    stdev = (totsq / tot) - mean * mean;
    stdev = sqrt(stdev);
  }
  Boolean reset = False;
  double newposmean = posmean[charge_];
  double newposstdev = posstdev[charge_];;
  while(noDistr()) {
    newposmean += 0.5;
    newposstdev += 0.95;
    reset = True;
  }
  if(reset) {
    setPositiveDistr(newposmean, newposstdev);
  }

  setNegativeDistr(mean, stdev);
  return True;
}


// Boolean MascotDiscrimValMixtureDistr::noDistr() {
//   if (charge_ == 0 &&
//       ((ContinuousDistribution*)(posdistr_))->getMean() + ((ContinuousDistribution*)(posdistr_))->getStdev() <
//       ((ContinuousDistribution*)(negdistr_))->getMean() + 3.0 * ((ContinuousDistribution*)(negdistr_))->getStdev())
//     return True;
//   else if (charge_ == 0)
//     return False;
//   else 
//     return 
//       ((ContinuousDistribution*)(posdistr_))->getMean() + ((ContinuousDistribution*)(posdistr_))->getStdev() <
//       ((ContinuousDistribution*)(negdistr_))->getMean() + 4.0 * ((ContinuousDistribution*)(negdistr_))->getStdev();

// }


void MascotDiscrimValMixtureDistr::setNegativeDistr(double mean, double stdev) {
  double* next = new double[2];
  next[0] = mean;
  next[1] = stdev;

  negdistr_->init(next);
  delete[] next;
}
