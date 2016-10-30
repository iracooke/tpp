#include "ProbIDDiscrimValMixtureDistr.h"

/*

Program       : CometDiscrimValMixtureDistr for PeptideProphet                                                       
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
ProbIDDiscrimValMixtureDistr::ProbIDDiscrimValMixtureDistr(int charge, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean nonparam) {  
  initializeDistr(charge, name, tag);
  all_negs_ = False;
  maldi_ = maldi;
  maxdiff_ = 0.002;
  gamma_ = False; //True;
  qtof_ = qtof;
  gammapos_ = False;
  nonparam_ = nonparam;

  //if(charge > 0)
  //gammapos_ = True;


  if(charge_ == 0)
    gamma_ = True;

  doublevals_ = new Array<double>;
  if (nonparam_) {
    negdistr_ = new NonParametricDistribution(0.081, True);
    posdistr_ = new NonParametricDistribution(0.081, False);
  }
  else { 
    if(qtof_ || (maldi_ && charge_ == 0)) {
      //cerr << "instantiating decaycontmultimix distr" << endl;
      posdistr_ = new DecayContinuousMultimixtureDistr(maxdiff_ * 2.0, qtof_);
    }
    else {
      
      if(gammapos_) 
	posdistr_ = new GammaDistribution(maxdiff_);
      else
	posdistr_ = new GaussianDistribution(maxdiff_);
      //posdistr_ = new ExtremeValueDistribution(maxdiff_);
    }

    if(gamma_)
      negdistr_ = new GammaDistribution(maxdiff_);
    else
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
  double singlynegmaldiprior[] = {1.0, 1.5, -5.0};

  // single score
  //  double singlyposprior[] = {-0.7, 1.0};

  double singlyposprior[] = {1.448, 1.542}; 

  double doublyposprior[] = {1.448, 1.542}; //{1.109, 1.767};
  double triplyposprior[] = {1.54, 1.74}; //{0.411, 1.545};
  double quadposprior[] = {1.54, 1.74}; //{0.411, 1.545};
  double pentposprior[] = {1.54, 1.74}; //{0.411, 1.545};
  double hexposprior[] = {1.54, 1.74}; //{0.411, 1.545};
  double septposprior[] = {1.54, 1.74}; //{0.411, 1.545};
  //  double triplyposprior[] = {2.54, 1.74}; //{0.411, 1.545};

  double singlynegprior[] = {-1.126, 2.045, -5.0};

  if(! gamma_)
    singlynegprior[1] = 0.88;

  //double singlynegprior[] = {-1.126, 0.88};



  //  double doublynegprior[] = {-0.67, 0.996, -5.0}; //{-2.468, 0.374};
  double doublynegprior[] = {-0.613, 0.7142}; //{-2.468, 0.374};

  //  double triplynegprior[] = {-0.613, 0.885, -5.0}; //{-2.179, 0.363};
  double triplynegprior[] = {-0.67, 0.74}; //{-2.179, 0.363};
  double quadnegprior[] = {-0.67, 0.74}; //{-2.179, 0.363};
  double pentnegprior[] = {-0.67, 0.74}; //{-2.179, 0.363};
  double hexnegprior[] = {-0.67, 0.74}; //{-2.179, 0.363};
  double septnegprior[] = {-0.67, 0.74}; //{-2.179, 0.363};
 
  double doublyposgamma[] = {4.87, 26.0, -3.34 };
  double triplyposgamma[] = {3.9, 16.8, -2.95 };
  double quadposgamma[] = {3.9, 16.8, -2.95 };
  double pentposgamma[] = {3.9, 16.8, -2.95 };
  double hexposgamma[] = {3.9, 16.8, -2.95 };
  double septposgamma[] = {3.9, 16.8, -2.95 };


  double minvals[] = {-2.0, -5.0, -5.0 };

//double minvals[] = {-2.0, -1.0, -1.0};

  negmean_ = 0.0;

  MIN_NUM_PSEUDOS_ = 50;
  ZERO_SET_ = 100; // ?
  NUM_DEVS_ = 6;
  USE_TR_NEG_DISTR_ = False;
  posinit_ = NULL;
  neginit_ = NULL;

  int num_params = gamma_ ? 3 : 2;

  if(charge == 0) {

    if(maldi_) {
      posinit_ = copy(singlyposmaldiprior, 2);
    }
    else {
      posinit_ = copy(singlyposprior, 2);
    }

    if(maldi_) {
	neginit_ = copy(singlynegmaldiprior, num_params);
    }
    else {
      neginit_ = copy(singlynegprior, num_params);
    }

  }
  else if(charge == 1) {
    if(gammapos_)
      posinit_ = copy(doublyposgamma, 3);
    else
      posinit_ = copy(doublyposprior, 2);
    neginit_ = copy(doublynegprior, num_params);
  }
  else if(charge == 2) {
    if(gammapos_)
      posinit_ = copy(triplyposgamma, 3);
    else
      posinit_ = copy(triplyposprior, 2);
    neginit_ = copy(triplynegprior, num_params);
  }
  else if(charge == 3) {
    if(gammapos_)
      posinit_ = copy(quadposgamma, 3);
    else
      posinit_ = copy(quadposprior, 2);
    neginit_ = copy(quadnegprior, num_params);
  }
  else if(charge == 4) {
    if(gammapos_)
      posinit_ = copy(pentposgamma, 3);
    else
      posinit_ = copy(pentposprior, 2);
    neginit_ = copy(pentnegprior, num_params);
  }
  else if(charge == 5) {
    if(gammapos_)
      posinit_ = copy(hexposgamma, 3);
    else
      posinit_ = copy(hexposprior, 2);
    neginit_ = copy(hexnegprior, num_params);
  }
  else if(charge == 6) {
    if(gammapos_)
      posinit_ = copy(septposgamma, 3);
    else
      posinit_ = copy(septposprior, 2);
    neginit_ = copy(septnegprior, num_params);
  }
  reset();

  min_dataval_ = 999;
  minval_ = minvals[charge];

  //  cout << "min val: " << minval_ << endl;

  // negdistr_->setMinVal(minval_);

  
  double min_fval[] = {-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0};
  if(charge > 0 && gamma_ && !nonparam_)
    ((GammaDistribution*)(negdistr_))->setDistrMinval(min_fval[charge_-1]);
  
}

Boolean ProbIDDiscrimValMixtureDistr::initializeNegGammaDistribution(NTTMixtureDistr* nttdistr) {
  // cout << "init neg..." << endl;

  assert(nttdistr->getNumVals() == getNumVals());
  double mean = 0.0;
  double stdev = 0.0;
  int tot = 0;
  double totsq = 0.0;
  double zero;
  double posmean[] = { 1.5, 4.102, 4.563, 4.563, 4.563, 4.563, 4.563 };
  double posstdev[] = { 1.5, 1.64, 1.84 , 1.84, 1.84, 1.84, 1.84 };
  double MAX_SINGLY_NEGMEAN = 1.0;

  //  double negmean_num_stds[] = {-1.0, 0.5, 0.5}; // by charge
  double negmean_num_stds[] = {-1.0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}; // by charge
  double maldi_negmean_num_stds = 0.5; //-0.1; //-1.0; //0.5; //-0.1; //0.25; //0.1;
  double qtof_negmean_num_stds = 0.5; //-0.1; //0.5;

  double min_singly_fval = -2;
  if(maldi_)
    min_singly_fval = -6;


  if(gamma_ && charge_ == 0) {
    ((GammaDistribution*)(negdistr_))->setDistrMinval(min_singly_fval);
  }
  int k;
  for(k = 0; k < getNumVals(); k++) {
    if(nttdistr->isValue(k, 0) && (! gamma_ || ((GammaDistribution*)(negdistr_))->aboveMin((*doublevals_)[k]))) {
      mean += (*doublevals_)[k];
      totsq += (*doublevals_)[k] * (*doublevals_)[k];
      tot++;
    }
  }

  //cout << "tot: " << tot << " vs " << MIN_NUM_PSEUDOS_ << endl;
  if(tot < MIN_NUM_PSEUDOS_) {
 
    if(! gamma_) {
      zero = 0.0; // don't need this
    }
    else {
      if(charge_ == 0) {
	zero = min_dataval_ - 3.0;
	if(zero < min_singly_fval) {
	  zero = min_singly_fval;
	}
      }
      else {
	zero = min_dataval_ - 0.1;
      }
      if(zero < minval_) {    
	zero = minval_;
      }
    } // gamma


      //  cout << "zero: " << zero << ", min data val: " << minval_ << endl;

    tot = 0; // restart
    mean = 0.0;
    totsq = 0.0;
    for(int k = 0; k < getNumVals(); k++) {
      if((*doublevals_)[k] > zero && (*doublevals_)[k] < posmean[charge_] - posstdev[charge_]) {
	mean += (*doublevals_)[k] - zero;
	totsq += ((*doublevals_)[k] - zero) * ((*doublevals_)[k] - zero);
	tot++;
      }
    } // next

    //    cout << "tot: " << tot << endl;

    if(tot > 0) {
      mean /= tot;
      stdev = totsq / tot - mean * mean;
      
    }
    else
      stdev = 10.0;

    stdev = sqrt(stdev);
    double* newsettings = new double[3];
    newsettings[0] = mean;
    newsettings[1] = stdev;
    newsettings[2] = zero;

    
    if(! gamma_) {
      negmean_ = mean - negmean_num_stds[charge_] * stdev;
      negdistr_->init(newsettings);
    }
    else {
      negmean_ = zero + mean - negmean_num_stds[charge_] * stdev;
      negdistr_->init(newsettings, False);
    }

    delete [] newsettings;
    
    if(negmean_ > MAX_SINGLY_NEGMEAN) {
      negmean_ = MAX_SINGLY_NEGMEAN;
    }
    //    cout << "setting negmean to " << negmean_ << " for charge " << (charge_+1) << " with zero " << zero << endl;

    USE_TR_NEG_DISTR_ = True;

    return False; // done
  } // if not enough pseudos
    
  mean /= tot;
  stdev = (totsq / tot) - mean * mean;
  stdev = sqrt(stdev);  // delete this later ?

  if(gamma_) {
    zero = mean - NUM_DEVS_ * stdev;
    //    cout << "zero: " << zero << " mean: " << mean << " numdevs: " << NUM_DEVS_ << " stdev: " << stdev << endl;
    if(min_dataval_ - ZERO_SET_ * stdev > zero) {
      zero = min_dataval_ - ZERO_SET_ * stdev;
    }
  }
  else {
    zero = 0.0;
  }
  // cout << "zero: " << zero << " minval " << min_dataval_ << endl;
  //  cout << "mean: " << mean << " and stdev: " << stdev << endl;

  if(maldi_) {
    negmean_ = mean - maldi_negmean_num_stds * stdev;
    //cerr << "mean: " << mean << ", negmeanstdevs: " << negmean_num_stds[0] << ", stdev: " << stdev << endl;
    //negmean_ = -1.4;
  }
  else if(qtof_) {
    negmean_ = mean - qtof_negmean_num_stds * stdev;
  }
  else
    negmean_ = mean - negmean_num_stds[charge_] * stdev;

  //  cout << "negmean: " << negmean_ << endl;

  // now recompute real mean and stdev
  mean = 0.0;
  totsq = 0.0;
  tot = 0;
    
  for(k = 0; k < getNumVals(); k++) {
    if(nttdistr->isValue(k, 0) && (*doublevals_)[k] > zero &&
       (! gamma_ || ((GammaDistribution*)(negdistr_))->aboveMin((*doublevals_)[k]))) {
      mean += (*doublevals_)[k] - zero;
      totsq += ((*doublevals_)[k] - zero) * ((*doublevals_)[k] - zero);
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
  while(! maldi_ && noDistr()) {
    newposmean += 0.5;
    newposstdev += 0.95;
    setPositiveDistr(newposmean, newposstdev);
    //reset = True;
  }
  //if(reset) {
  //setPositiveDistr(newposmean, newposstdev);
  //}

  setNegativeDistr(mean, stdev, zero);
  return True;
}


Boolean ProbIDDiscrimValMixtureDistr::initializeNegGammaDistribution(Array<Boolean>* isdecoy) {
  // cout << "init neg..." << endl;
  if (nonparam_) {
    isdecoy_ = isdecoy;
    ((NonParametricDistribution*)(posdistr_))->initWithDecoy(isdecoy, doublevals_);
    ((NonParametricDistribution*)(negdistr_))->initWithDecoy(isdecoy, doublevals_);
    return True;
  }
  //assert(nttdistr->getNumVals() == getNumVals());
  double mean = 0.0;
  double stdev = 0.0;
  int tot = 0;
  double totsq = 0.0;
  double zero;
  double posmean[] = { 1.5, 4.102, 4.563, 4.563, 4.563 };
  double posstdev[] = { 1.5, 1.64, 1.84 , 1.84, 1.84  };
  double MAX_SINGLY_NEGMEAN = 1.0;

  //  double negmean_num_stds[] = {-1.0, 0.5, 0.5}; // by charge
  double negmean_num_stds[] = {-1.0, 0.5, 0.5, 0.5, 0.5}; // by charge
  double maldi_negmean_num_stds = 0.5; //-0.1; //-1.0; //0.5; //-0.1; //0.25; //0.1;
  double qtof_negmean_num_stds = 0.5; //-0.1; //0.5;

  double min_singly_fval = -2;
  if(maldi_)
    min_singly_fval = -6;


  if(gamma_ && charge_ == 0) {
    ((GammaDistribution*)(negdistr_))->setDistrMinval(min_singly_fval);
  }
  int k;
  for(k = 0; k < getNumVals(); k++) {
    if ((*isdecoy)[k]) {
      mean += (*doublevals_)[k];
      totsq += (*doublevals_)[k] * (*doublevals_)[k];
      tot++;
    }
  }

  //cout << "tot: " << tot << " vs " << MIN_NUM_PSEUDOS_ << endl;
  if(tot < MIN_NUM_PSEUDOS_) {
 
    if(! gamma_) {
      zero = 0.0; // don't need this
    }
    else {
      if(charge_ == 0) {
	zero = min_dataval_ - 3.0;
	if(zero < min_singly_fval) {
	  zero = min_singly_fval;
	}
      }
      else {
	zero = min_dataval_ - 0.1;
      }
      if(zero < minval_) {    
	zero = minval_;
      }
    } // gamma


      //  cout << "zero: " << zero << ", min data val: " << minval_ << endl;

    tot = 0; // restart
    mean = 0.0;
    totsq = 0.0;
    for(int k = 0; k < getNumVals(); k++) {
      if((*doublevals_)[k] > zero && (*doublevals_)[k] < posmean[charge_] - posstdev[charge_]) {
	mean += (*doublevals_)[k] - zero;
	totsq += ((*doublevals_)[k] - zero) * ((*doublevals_)[k] - zero);
	tot++;
      }
    } // next

    //    cout << "tot: " << tot << endl;

    if(tot > 0) {
      mean /= tot;
      stdev = totsq / tot - mean * mean;
    }
    else
      stdev = 10.0;

    stdev = sqrt(stdev);
    double* newsettings = new double[3];
    newsettings[0] = mean;
    newsettings[1] = stdev;
    newsettings[2] = zero;

    if(! gamma_) {
      negmean_ = mean - negmean_num_stds[charge_] * stdev;
      negdistr_->init(newsettings);
    }
    else {
      negmean_ = zero + mean - negmean_num_stds[charge_] * stdev;
      negdistr_->init(newsettings, False);
    }
    delete[] newsettings;

    if(negmean_ > MAX_SINGLY_NEGMEAN) {
      negmean_ = MAX_SINGLY_NEGMEAN;
    }
    //    cout << "setting negmean to " << negmean_ << " for charge " << (charge_+1) << " with zero " << zero << endl;

    USE_TR_NEG_DISTR_ = True;

    return False; // done
  } // if not enough pseudos
    
  mean /= tot;
  stdev = (totsq / tot) - mean * mean;
  stdev = sqrt(stdev);  // delete this later ?

  if(gamma_) {
    zero = mean - NUM_DEVS_ * stdev;
    //    cout << "zero: " << zero << " mean: " << mean << " numdevs: " << NUM_DEVS_ << " stdev: " << stdev << endl;
    if(min_dataval_ - ZERO_SET_ * stdev > zero) {
      zero = min_dataval_ - ZERO_SET_ * stdev;
    }
  }
  else {
    zero = 0.0;
  }
  // cout << "zero: " << zero << " minval " << min_dataval_ << endl;
  //  cout << "mean: " << mean << " and stdev: " << stdev << endl;

  if(maldi_) {
    negmean_ = mean - maldi_negmean_num_stds * stdev;
    //cerr << "mean: " << mean << ", negmeanstdevs: " << negmean_num_stds[0] << ", stdev: " << stdev << endl;
    //negmean_ = -1.4;
  }
  else if(qtof_) {
    negmean_ = mean - qtof_negmean_num_stds * stdev;
  }
  else
    negmean_ = mean - negmean_num_stds[charge_] * stdev;

  //  cout << "negmean: " << negmean_ << endl;

  // now recompute real mean and stdev
  mean = 0.0;
  totsq = 0.0;
  tot = 0;
    
  for(k = 0; k < getNumVals(); k++) {
    if((*isdecoy)[k]) {
      mean += (*doublevals_)[k] - zero;
      totsq += ((*doublevals_)[k] - zero) * ((*doublevals_)[k] - zero);
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
  while(! maldi_ && noDistr()) {
    newposmean += 0.5;
    newposstdev += 0.95;
    setPositiveDistr(newposmean, newposstdev);
    //reset = True;
  }
  //if(reset) {
  //setPositiveDistr(newposmean, newposstdev);
  //}

  setNegativeDistr(mean, stdev, zero);
  return True;
}



Boolean ProbIDDiscrimValMixtureDistr::initializeNegDistribution(NTTMixtureDistr* nttdistr) {

  if(gamma_)
    return initializeNegGammaDistribution(nttdistr);
  else
    return initializeNegExtremeValueDistribution(nttdistr);
}

Boolean ProbIDDiscrimValMixtureDistr::initializeNegDistribution(Array<Boolean>* isdecoy) {
  if (nonparam_) {
    isdecoy_ = isdecoy;
    ((NonParametricDistribution*)(posdistr_))->initWithDecoy(isdecoy, doublevals_);
    ((NonParametricDistribution*)(negdistr_))->initWithDecoy(isdecoy, doublevals_);
    return True;
  }
  if(gamma_)
    return initializeNegGammaDistribution(isdecoy);
  else
    return initializeNegExtremeValueDistribution(isdecoy);
}

Boolean ProbIDDiscrimValMixtureDistr::initializeNegExtremeValueDistribution(NTTMixtureDistr* nttdistr) {



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
  double posmean[] = { -.191, 1.109, 0.411, 0.411, 0.411};
  double posstdev[] = { 0.833, 1.77, 1.55, 1.55, 1.55 };
  //  double posmean[] = { 1.5, 1.109, 0.411};
  // double posstdev[] = { 1.5, 1.77, 1.55 };


  // values for truncating negative distribution
  //double minvals[] = { -1.3, -1.5, -1.1 };
  double minvals[] = { -1.5, -2.6, -2.0, -2.0, -2.0 };

  if(charge_ == 0)
      ((ExtremeValueDistribution*)(negdistr_))->setDistrMinval(minvals[charge_]);


  double MAX_SINGLY_NEGMEAN = 1.0;

  //double negmean_num_stds[] = {-1.0, 0.5, 0.5}; // by charge
  //double negmean_num_stds[] = {-0.1, 1.0, 1.0}; // by charge

  //double negmean_num_stds[] = {-0.3, 0.0, 0.0}; // by charge
  double negmean_num_stds[] = {5.0, 0.0, 0.0, 0.0, 0.0}; // by charge

  double min_singly_fval = -2;
  int k;
  for(k = 0; k < getNumVals(); k++) {
    if(! gamma_ && nttdistr->isValue(k, 0) && ((ExtremeValueDistribution*)(negdistr_))->aboveMin((*doublevals_)[k])) {
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
    if(! gamma_ && nttdistr->isValue(k, 0) && ((ExtremeValueDistribution*)(negdistr_))->aboveMin((*doublevals_)[k])) {
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

  setNegativeProbidDistr(mean, stdev);
  return True;
}

Boolean ProbIDDiscrimValMixtureDistr::initializeNegExtremeValueDistribution(Array<Boolean>* isdecoy) {
  if (nonparam_) {
    return True;
  }
  double mean = 0.0;
  double stdev = 0.0;
  int tot = 0;
  double totsq = 0.0;
  //double zero;


  // evd for discriminatn score
  //double posmean[] = { 2.0, 4.102, 4.563 };
  //double posstdev[] = { 0.4, 1.64, 1.84 };

  // for single score guassian
  double posmean[] = { -.191, 1.109, 0.411, 0.411, 0.411};
  double posstdev[] = { 0.833, 1.77, 1.55, 1.55, 1.55 };
  //  double posmean[] = { 1.5, 1.109, 0.411};
  // double posstdev[] = { 1.5, 1.77, 1.55 };


  // values for truncating negative distribution
  //double minvals[] = { -1.3, -1.5, -1.1 };
  double minvals[] = { -1.5, -2.6, -2.0, -2.0, -2.0 };

  if(charge_ == 0)
      ((ExtremeValueDistribution*)(negdistr_))->setDistrMinval(minvals[charge_]);


  double MAX_SINGLY_NEGMEAN = 1.0;

  //double negmean_num_stds[] = {-1.0, 0.5, 0.5}; // by charge
  //double negmean_num_stds[] = {-0.1, 1.0, 1.0}; // by charge

  //double negmean_num_stds[] = {-0.3, 0.0, 0.0}; // by charge
  double negmean_num_stds[] = {5.0, 0.0, 0.0, 0.0, 0.0}; // by charge

  double min_singly_fval = -2;
  int k;
  for(k = 0; k < getNumVals(); k++) {
    if((*isdecoy)[k]) {
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
    delete [] newsettings;


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

  setNegativeProbidDistr(mean, stdev);
  return True;
}


Boolean ProbIDDiscrimValMixtureDistr::noDistrGamma() {
  //double num_stdevs = 0.45; //0.15;
  //  double num_stdevs[] = {0.5, 0.45, 0.7}; // for each charge
   double num_stdevs[] = {0.5, 0.6, 0.8, 0.8, 0.8}; // for each charge
 double min_pos_means[] = {-0.2, 0.0, 0.5, 0.5, 0.5};
  Boolean first;
  Boolean second;
  Boolean third = False;
  if(qtof_ || (charge_ == 0 && maldi_)) {
    return False;
    first = gamma_ && ((DecayContinuousMultimixtureDistr*)(posdistr_))->getMean() + ((DecayContinuousMultimixtureDistr*)(posdistr_))->getStdev() < ((ContinuousDistribution*)(negdistr_))->getMean() + ((GammaDistribution*)(negdistr_))->getZero() + num_stdevs[charge_] * sqrt(((ContinuousDistribution*)(negdistr_))->getStdev());
  Boolean second = !gamma_ && ((DecayContinuousMultimixtureDistr*)(posdistr_))->getMean() + ((DecayContinuousMultimixtureDistr*)(posdistr_))->getStdev() < ((ContinuousDistribution*)(negdistr_))->getMean() + ((ContinuousDistribution*)(negdistr_))->getStdev();
  }
  else {
    first = gamma_ && ((ContinuousDistribution*)(posdistr_))->getMean() + ((ContinuousDistribution*)(posdistr_))->getStdev() < ((ContinuousDistribution*)(negdistr_))->getMean() + ((GammaDistribution*)(negdistr_))->getZero() + num_stdevs[charge_] * sqrt(((ContinuousDistribution*)(negdistr_))->getStdev());
    second = !gamma_ && ((ContinuousDistribution*)(posdistr_))->getMean() + ((ContinuousDistribution*)(posdistr_))->getStdev() < ((ContinuousDistribution*)(negdistr_))->getMean() + ((ContinuousDistribution*)(negdistr_))->getStdev();
    third = gamma_ && ((ContinuousDistribution*)(posdistr_))->getMean() < min_pos_means[charge_] && 
      ((ContinuousDistribution*)(posdistr_))->getMean() + ((ContinuousDistribution*)(posdistr_))->getStdev() < ((ContinuousDistribution*)(negdistr_))->getMean() + ((GammaDistribution*)(negdistr_))->getZero() + sqrt(((ContinuousDistribution*)(negdistr_))->getStdev());
 }


  return (first || second || third);


}


Boolean ProbIDDiscrimValMixtureDistr::noDistr() {
  return DiscrimValMixtureDistr::noDistr();
  //Not used anymore
  if(gamma_)
    return noDistrGamma();
  return noDistrEVD();
}



Boolean ProbIDDiscrimValMixtureDistr::noDistrEVD() {
  if(gammapos_)
    return False;
  return ((ContinuousDistribution*)(posdistr_))->getMean() + ((ContinuousDistribution*)(posdistr_))->getStdev() < ((ContinuousDistribution*)(negdistr_))->getMean() + ((ContinuousDistribution*)(negdistr_))->getStdev(); 

}


void ProbIDDiscrimValMixtureDistr::setNegativeProbidDistr(double mean, double stdev) {
  assert(!gamma_);
  double* next = new double[2];
  next[0] = mean;
  next[1] = stdev;

  negdistr_->init(next);
  delete [] next;
}

void ProbIDDiscrimValMixtureDistr::setDiscrimFunction(const char* mass_spec_type) {
   //cout << "setting new probid discrim function...for input file: " << mass_spec_type << endl;
   discrim_func_ = new ProbIDDiscrimFunction(charge_);

}
