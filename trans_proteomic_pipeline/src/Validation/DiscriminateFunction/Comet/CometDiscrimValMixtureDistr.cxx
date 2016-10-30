#include "CometDiscrimValMixtureDistr.h"

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
CometDiscrimValMixtureDistr::CometDiscrimValMixtureDistr(int charge, const char* name, const char* tag, Boolean gamma, Boolean maldi, Boolean qtof, Boolean nonparam, Boolean use_expect )
  : DiscrimValMixtureDistr( charge,   name,  tag,  gamma,  maldi,  qtof,  nonparam, use_expect) {  

  use_expect_ = use_expect;
  all_negs_ = False;
  gamma_ = gamma;
  maldi_ = maldi;
  maxdiff_ = 0.002;

  qtof_ = qtof;
  reinit_ = 0;
  discrim_func_ = NULL;
  min_dataval_ = 999.0;
  nonparam_ = nonparam;
  doublevals_ = new Array<double>;
 
  if (nonparam_) {
    negdistr_ = new NonParametricDistribution(0.5, True, False);
    posdistr_ = new NonParametricDistribution(0.5, False, False);
  }
  else {
    if(qtof_ || (maldi_ && charge_ == 0)) {
      //cerr << "instantiating decaycontmultimix distr" << endl;
      posdistr_ = new DecayContinuousMultimixtureDistr(maxdiff_ * 2.0, qtof_);
    }
    else
      posdistr_ = new GaussianDistribution(maxdiff_);
    
    
    if(gamma_) {
      negdistr_ = new GammaDistribution(maxdiff_);
    }
    else {
      negdistr_ = new GaussianDistribution(maxdiff_);
    }
  }


  double singlyposprior[] = {2.0, 0.4};
  double doublyposprior[] = {4.563, 1.24};
  double triplyposprior[] = {4.563, 1.24};
  double quadposprior[] = {4.563, 1.24};
  double pentposprior[] = {4.563, 1.24};
  double hexposprior[] = {4.563, 1.24};
  double septposprior[] = {4.563, 1.24};
  double singlynegprior[] = {-2.0, 0.9};
  double doublynegprior[] = {-1.945, 0.87};
  double triplynegprior[] = {-1.563, 0.78};
  double quadnegprior[] = {-1.563, 0.78};
  double pentnegprior[] = {-1.563, 0.78};
  double hexnegprior[] = {-1.563, 0.78};
  double septnegprior[] = {-1.563, 0.78};
  double singlyneggammprior[] = {5.0, 26.0, -5.0};
  double doublyneggammprior[] = {6.06, 37, -5.0};
  double triplyneggammprior[] = {6.06, 37, -5.0};
  double quadneggammprior[] = {6.06, 37, -5.0};
  double pentneggammprior[] = {6.06, 37, -5.0};
  double hexneggammprior[] = {6.06, 37, -5.0};
  double septneggammprior[] = {6.06, 37, -5.0};
  double singlyposmaldiprior[] = {5.0, 0.6};
  double singlynegmaldiprior[] = {1.0, 1.5};
  double singlyneggammmaldiprior[] = {6.5, 25.6, -5.52};
  double minvals[] = {-2.0, -5.0, -5.0 , -5.0, -5.0, -5.0, -5.0};
  negmean_ = 0.0;
  MIN_NUM_PSEUDOS_ = 50;
  ZERO_SET_ = 1; // ?
  NUM_DEVS_ = 6;
  USE_TR_NEG_DISTR_ = False;
  posinit_ = NULL;
  neginit_ = NULL;

  if(charge == 0) {
    if(maldi_ || qtof_) {
      ; //posinit_ = copy(singlyposmaldiprior, 2);
    }
    else {
      posinit_ = copy(singlyposprior, 2);
    }
    if(gamma) {
      if(maldi_) {
	neginit_ = copy(singlyneggammmaldiprior, 3);
      }
      else {
	neginit_ = copy(singlyneggammprior, 3);
      }
    }
    else { // not gamma
      if(maldi_) {
	neginit_ = copy(singlynegmaldiprior, 2);
      }
      else {
	neginit_ = copy(singlynegprior, 2);
      }
    }
  }
  else if(charge == 1) {
    if(! qtof_)
      posinit_ = copy(doublyposprior, 2);
    if(gamma) {
      neginit_ = copy(doublyneggammprior, 3);
    }
    else {
      neginit_ = copy(doublynegprior, 2);
    }
  }
  else if(charge == 2) {
    if(! qtof_)
      posinit_ = copy(triplyposprior, 2);
    if(gamma) {
      neginit_ = copy(triplyneggammprior, 3);
    }
    else {
      neginit_ = copy(triplynegprior, 2);
    }
  }
  else if(charge == 3) {
    if(! qtof_)
      posinit_ = copy(quadposprior, 2);
    if(gamma) {
      neginit_ = copy(quadneggammprior, 3);
    }
    else {
      neginit_ = copy(quadnegprior, 2);
    }
  }
  else if(charge == 4) {
    if(! qtof_)
      posinit_ = copy(pentposprior, 2);
    if(gamma) {
      neginit_ = copy(pentneggammprior, 3);
    }
    else {
      neginit_ = copy(pentnegprior, 2);
    }
  }
  else if(charge == 5) {
    if(! qtof_)
      posinit_ = copy(hexposprior, 2);
    if(gamma) {
      neginit_ = copy(hexneggammprior, 3);
    }
    else {
      neginit_ = copy(hexnegprior, 2);
    }
  }
  else if(charge == 6) {
    if(! qtof_)
      posinit_ = copy(septposprior, 2);
    if(gamma) {
      neginit_ = copy(septneggammprior, 3);
    }
    else {
      neginit_ = copy(septnegprior, 2);
    }
  }


  reset();

  if(gamma_) {
    minval_ = minvals[charge];
    negdistr_->setMinVal(minval_);
  }

}

CometDiscrimValMixtureDistr::~CometDiscrimValMixtureDistr() {
  // delete[] posinit_;
  // delete[] neginit_;
  //delete discrim_func_;
}

void CometDiscrimValMixtureDistr::setConservative(float stdev) {
  consStdev_ = stdev;
}

void CometDiscrimValMixtureDistr::setDiscrimFunction(const char* mass_spec_type) {
  discrim_func_ = new CometDiscrimFunction(charge_, use_expect_);
}

bool CometDiscrimValMixtureDistr::valid(SearchResult* result) {
 
  if(discrim_func_ != NULL) {
   double nextval = discrim_func_->getDiscriminantScore(result);
   if ( isnan(nextval) || isinf(nextval) ) {
     cerr << "ERROR: CometDiscriminant value computed as " << nextval << ", for spectrum " << result->spectrum_ 
	  << ". This spectrum will be excluded from the model." << endl;
     return false;
   }
  }
  else {
    return false;
  }

  return true;
}

void CometDiscrimValMixtureDistr::enter(SearchResult* result) {
  if(discrim_func_ != NULL) {
    double nextval = discrim_func_->getDiscriminantScore(result);
    //    if(result->charge_ == 2 && nextval < -3.0) {
    //      cout << result->spectrum_ << " had score: " << nextval << endl;
    //    }
    
    if ( isnan(nextval) || isinf(nextval) ) {
      cerr << "ERROR: CometDiscriminant value computed as " << nextval << ", for spectrum " << result->spectrum_ 
	   << ". This spectrum will be excluded from the model." << endl;
      return;
    }
    MixtureDistr::enter(0, nextval);
    if(nextval < min_dataval_) 
      min_dataval_ = nextval;
  }
  else {
    cout << "NULL discrim function..." << endl;
    exit(1);
  }

}


double* CometDiscrimValMixtureDistr::copy(double* init, int num) {
  double* output = new double[num];
  for(int k = 0; k < num; k++) {
    output[k] = init[k];
  }
  return output;
}

void CometDiscrimValMixtureDistr::reset() {
  if (nonparam_) return;
  if(qtof_ || (maldi_ && charge_ == 0)) {
    posdistr_->init(posinit_); //((DecayContinuousMultimixtureDistr*)(posdistr_))->reset();
  }
  else {
    posdistr_->init(posinit_);
  }
  if (!gamma_) {
    negdistr_->init(neginit_);
  } 
  else {
    negdistr_->init(neginit_, True);
  }
 }

void CometDiscrimValMixtureDistr::resetTot() {
  ((ContinuousDistribution*)(posdistr_))->resetTot();
  ((ContinuousDistribution*)(negdistr_))->resetTot();
} 

Boolean CometDiscrimValMixtureDistr::noDistr() {
  //if (nonparam_) {
  //  return False;
  //}
  // if (charge_ == 0) {
  //  if (getNumVals() < 500) return (True);
  //}

  double posMean = ((ContinuousDistribution*)(posdistr_))->getMean();
  double posStdev = ((ContinuousDistribution*)(posdistr_))->getStdev();
  double negMean = ((ContinuousDistribution*)(negdistr_))->getMean();
  double negStdev = ((ContinuousDistribution*)(negdistr_))->getStdev();

  double negZero = 0;

  if (gamma_) {
    negZero = ((GammaDistribution*)(negdistr_))->getZero();
    negMean += negZero;
  }

  if (negMean > posMean || posStdev < 0.0001) {
    //cerr << "noDistr: negMean = " << negMean << " > posMean = " << posMean << endl;
    return True;
  }

  double posSlice =((ContinuousDistribution*)(posdistr_))->slice(posMean-3*posStdev, posMean+3*posStdev);
  double negSlice = ((ContinuousDistribution*)(negdistr_))->slice(negMean-3*negStdev, negMean+3*negStdev);
  double ratio = posSlice / negSlice;

  if (ratio > 10) {
    //double posSlice =((ContinuousDistribution*)(posdistr_))->slice(posMean-3*posStdev, posMean+3*posStdev);
    //double negSlice = ((ContinuousDistribution*)(negdistr_))->slice(negMean-3*negStdev, negMean+3*negStdev);
    //cerr << "Positive distribution is too large compared to the Negative: posSlice " << posSlice << " > negSlice = " << negSlice << endl; 
    return True;
  }

  if (ratio > 1.1 && posMean < negMean + 2*negStdev ) {
    //cerr << "Positive distribution is too large compared to the Negative: posSlice " << posSlice << " > negSlice = " << negSlice ; 
    //cerr << "AND Too much overlap between negative and positive models; posMean (" << posMean <<  ") is less than negMean + 2 * negStdev (" 
    //	 << negMean + 2*negStdev << ")" << endl;

    return True;
  }
  posSlice = ((ContinuousDistribution*)(posdistr_))->slice(posMean, posMean+2*posStdev);
  negSlice = ((ContinuousDistribution*)(negdistr_))->slice(posMean, posMean+2*posStdev);
  ratio = posSlice / negSlice;
  if (ratio < 1.1 ) {
    //cerr << "Too much overlap detected at high f-vals: negSlice = " << negSlice << " > posSlice = " << posSlice << endl; 
    return True;
  }
   
 

  
  return False; 

}


double CometDiscrimValMixtureDistr::getPosProb(int index) {
  if(all_negs_ || (*doublevals_)[index] < negmean_) {
    return 0.0;
  }
  else if (nonparam_) {
    if(all_negs_ || (*doublevals_)[index] < negmean_) {
      return 0.0;
    }
    return MixtureDistr::getPosProb(index);
  }
  if(qtof_ || (maldi_ && charge_ == 0)) {
    //cerr << "pos prob: " << ((DecayContinuousMultimixtureDistr*)(posdistr_))->getMixtureProb(index, (*doublevals_)[index]) << endl;
    //if(((DecayContinuousMultimixtureDistr*)(posdistr_))->oneProb((*doublevals_)[index])) {
    // return 1.0;
    //}

    return ((DecayContinuousMultimixtureDistr*)(posdistr_))->getMixtureProb(index, (*doublevals_)[index]);

   
    //cerr << "MALDI" << endl;
    //cerr << "number of distrs in DISCRIMVAL: " << ((DecayContinuousMultimixtureDistr*)(posdistr_))->getNumDistributions() << endl;
  }


  if(! maldi_ && ((GaussianDistribution*)(posdistr_))->oneProb((*doublevals_)[index])) {
    if(charge_ == 0) {
      ;//cout << "one probs for " << (*doublevals_)[index] << endl;
    }
    return 1.0;
  }
  else if(gamma_ && ((GammaDistribution*)(negdistr_))->zeroProb((*doublevals_)[index])) {
    return 0.0;
  }
  return MixtureDistr::getPosProb(index);
}



double CometDiscrimValMixtureDistr::getNegProb(int index) {
  if(all_negs_ || (*doublevals_)[index] < negmean_) {
    return 1.0;
  }
  else if (nonparam_) {
    if(all_negs_ || (*doublevals_)[index] < negmean_) {
      return 1.0;
    }
    return MixtureDistr::getNegProb(index);
  }

  if(! maldi_ && ! qtof_ && ((GaussianDistribution*)(posdistr_))->oneProb((*doublevals_)[index])) {
    return 0.0;
  }
  //else if(((maldi_ && charge_ == 0) || qtof_) && ((DecayContinuousMultimixtureDistr*)(posdistr_))->oneProb((*doublevals_)[index])) {
  //return 0.0;
  //}
  else if(gamma_ && ((GammaDistribution*)(negdistr_))->zeroProb((*doublevals_)[index])) {
    return 1.0;
  }
  //cerr << "neg prob: " << MixtureDistr::getNegProb(index) << endl;

  return MixtureDistr::getNegProb(index);
}

void CometDiscrimValMixtureDistr::enter(int index, double val) {
  MixtureDistr::enter(index, val);
  if(index == 0 || val < min_dataval_) {
    min_dataval_ = val;
  }
}


double CometDiscrimValMixtureDistr::getRightCumulativeNegProb(int index, double right_val) {
  return negSlice((*doublevals_)[index], right_val);
}

double CometDiscrimValMixtureDistr::getRightCumulativeNegProb(double total, int index, double right_val) {
  return ((ContinuousDistribution*)(negdistr_))->slice(total, (*doublevals_)[index], right_val);
}

int CometDiscrimValMixtureDistr::slice(double left_val, double right_val) {
  int tot = 0;
  for(int k = 0; k < doublevals_->length(); k++) {
    if((*doublevals_)[k] > left_val && (*doublevals_)[k] <= right_val) {
      tot++;
    }
  }
  return tot;
}


Boolean CometDiscrimValMixtureDistr::decayMultimixture() { return qtof_ || (charge_ == 0 && maldi_); }

double CometDiscrimValMixtureDistr::posSliceWithNTT(double left_val, double right_val, int ntt) {
  if(! decayMultimixture()) {
    cerr << "error in posSliceWithNTT" << endl;
    exit(1);
  }
  return ((DecayContinuousMultimixtureDistr*)(posdistr_))->sliceWithNTT(left_val, right_val, ntt);
}

double CometDiscrimValMixtureDistr::posSlice(double left_val, double right_val) {
  if(decayMultimixture())
    return ((DecayContinuousMultimixtureDistr*)(posdistr_))->slice(left_val, right_val);
  return ((ContinuousDistribution*)(posdistr_))->slice(left_val, right_val);
}

double CometDiscrimValMixtureDistr::negSlice(double left_val, double right_val) {
  return ((ContinuousDistribution*)(negdistr_))->slice(left_val, right_val);
}

void CometDiscrimValMixtureDistr::setNegativeDistr(double mean, double stdev, double zero) {
  double* next = new double[3];
  next[0] = mean;
  next[1] = stdev;
  next[2] = zero;
  if (!gamma_) {
    negdistr_->init(next);
  }
  else {
    next[0] = mean - zero;
    negdistr_->init(next, False);
  }
  delete [] next;
}

void CometDiscrimValMixtureDistr::setPositiveDistr(double mean, double stdev) {
  double* next = new double[2];
  next[0] = mean;
  next[1] = stdev;
  posdistr_->init(next);
  delete [] next;
}
void CometDiscrimValMixtureDistr::setPositiveDistr(double mean, double stdev, double tot) {
  double* next = new double[2];
  next[0] = mean;
  next[1] = stdev;
  posdistr_->init(next);
  ((ContinuousDistribution*)posdistr_)->setTot(tot);
  delete [] next;
}
void CometDiscrimValMixtureDistr::printDistr() {
  MixtureDistr::printDistr();
  printf("\tnegmean: %0.2f\n", negmean_);
}

void CometDiscrimValMixtureDistr::writeDistr(FILE* fout) {
  MixtureDistr::writeDistr(fout);
  fprintf(fout, "\tnegmean: %0.2f\n", negmean_);
}

// roll into parent....
Array<Tag*>* CometDiscrimValMixtureDistr::getMixtureDistrTags(const char* name) {
  char next[500];
  sprintf(next, "%s\tnegmean: %0.2f", getName(), negmean_);
  return MixtureDistr::getMixtureDistrTags(next);
}


Boolean CometDiscrimValMixtureDistr::update(Array<double>* probs) {
  Boolean result;
  if (nonparam_) {
    result = MixtureDistr::update(probs, isdecoy_, False);
    //    negmean_ = ((ContinuousDistribution*)negdistr_)->getMean()-((ContinuousDistribution*)negdistr_)->getStdev();
    negmean_ = ((ContinuousDistribution*)negdistr_)->getMean()+consStdev_*((ContinuousDistribution*)negdistr_)->getStdev();
    return result;
  }
  result = MixtureDistr::update(probs);
  if (result) {
    if (gamma_) {
      negmean_ = ((GammaDistribution*)negdistr_)->getZero() + ((ContinuousDistribution*)negdistr_)->getMean()-((ContinuousDistribution*)negdistr_)->getStdev();
    }
    else {
      negmean_ = ((ContinuousDistribution*)negdistr_)->getMean()-((ContinuousDistribution*)negdistr_)->getStdev();
    }
  }
  return result;
}

Boolean CometDiscrimValMixtureDistr::finalupdate(Array<double>* probs) {
  if (nonparam_) {
    MixtureDistr::update(probs, isdecoy_, True);
  }
  else {
    MixtureDistr::update(probs);
  }
  Boolean result = False;
  if(noDistr()) {
    cerr << "WARNING: Mixture model quality test failed for charge (" << charge_+1 << "+)." << endl;
    all_negs_ = True;
    result = True;
  }
  return result;
}

Boolean CometDiscrimValMixtureDistr::reinitialize() {
  if(! gamma_ || charge_ == 0 || reinit_ > 10 || nonparam_)
    return False; // already done it, and can only be done once
  double min_fval[] = {-5.0, -5.0, -5.0, -5.0, -5.0}; // charge states 1+, 2+,3+,4+,5+
  reset(); // go back to initial settings
  ((GammaDistribution*)(negdistr_))->setDistrMinval(min_fval[charge_-1]);
  ((ContinuousDistribution*)negdistr_)->setTot(1);
  ((ContinuousDistribution*)posdistr_)->setTot(1);

  all_negs_ = False;

  reinit_ ++;
  return True;
}

Boolean CometDiscrimValMixtureDistr::initializeNegDistribution(Array<Boolean>* isdecoy) {
  if (nonparam_) {
    isdecoy_ = isdecoy;
    ((NonParametricDistribution*)(posdistr_))->initWithDecoy(isdecoy, doublevals_);
    ((NonParametricDistribution*)(negdistr_))->initWithDecoy(isdecoy, doublevals_);
    return True;
  }
  double mean = 0.0;
  double stdev = 0.0;
  int tot = 0;
  double totsq = 0.0;
  double zero;
  double posmean[] = { 2.0, 4, 4 , 2, 2, 2, 2};
  double posstdev[] = { 0.4, 2, 2, 1.5, 1.5, 1.5, 1.5};

  double MAX_SINGLY_NEGMEAN = 1.0;

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
    if((*isdecoy)[k]) {
      mean += (*doublevals_)[k];
      totsq += (*doublevals_)[k] * (*doublevals_)[k];
      tot++;
    }
  }

  //    cout << "tot: " << tot << " vs " << MIN_NUM_PSEUDOS_ << endl;
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


    //      cout << "zero: " << zero << ", min data val: " << minval_ << endl;

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
      stdev = sqrt(stdev);
    }
    else
      stdev = 10.0;

    double* newsettings = new double[3];
    newsettings[0] = mean;
    newsettings[1] = sqrt(stdev);
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
    if (zero < minval_) {
      zero = minval_;
    }

  }
  else {
    zero = 0.0;
  }

  //  cout << "zero: " << zero << " minval " << min_dataval_ << endl;
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
  double tmean = mean;
  double tsigma = stdev;
  mean = 0.0;
  totsq = 0.0;
  tot = 0;
  double pmean = 0.0;
  double ptotsq = 0.0;
  int ptot = 0; 
  for(k = 0; k < getNumVals(); k++) {
    if((*isdecoy)[k]) {
      mean += (*doublevals_)[k] - zero;
      totsq += ((*doublevals_)[k] - zero) * ((*doublevals_)[k] - zero);
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
    stdev = (totsq / tot) - mean * mean;
    stdev = sqrt(stdev);
  }
  
  Boolean reset = False;
  double newposmean = posinit_[0];
  double newposstdev = posinit_[1];
  if(ptot > 0) {
    newposmean = pmean / ptot;
    newposstdev = sqrt((ptotsq / ptot) - newposmean * newposmean);
  }
  setNegativeDistr(mean, stdev, zero);
  setPositiveDistr(newposmean, newposstdev);
  ((ContinuousDistribution*)negdistr_)->setTot(1);
  ((ContinuousDistribution*)posdistr_)->setTot(1);
  while(! maldi_ && noDistr()) {
    if (newposstdev < 0.0001) {
      newposstdev = 0.0001;
    }
    
    newposmean += 0.5;
    //    newposstdev += 0.95;
    setPositiveDistr(newposmean, newposstdev);
    ((ContinuousDistribution*)posdistr_)->setTot(1);
    //reset = True;
  }
  //if(reset) {
  //setPositiveDistr(newposmean, newposstdev);
  //}

  //setNegativeDistr(mean, stdev, zero);
  return True;


}

// use data with 0 tryptic termini to initialize negative distributions (when available)
Boolean CometDiscrimValMixtureDistr::initializeNegDistribution(NTTMixtureDistr* nttdistr) {

  // cout << "init neg..." << endl;

  assert(nttdistr->getNumVals() == getNumVals());
  int minNTT = 0;
  double mean = 0.0;
  double stdev = 0.0;
  int tot = 0;
  double totsq = 0.0;
  double zero;
  double MAX_SINGLY_NEGMEAN = 1.0;

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
    if(nttdistr->isValue(k, 0)  && (! gamma_ || ((GammaDistribution*)(negdistr_))->aboveMin((*doublevals_)[k]))) {
      mean += (*doublevals_)[k];
      totsq += (*doublevals_)[k] * (*doublevals_)[k];
      tot++;
    }
  }

  if(tot < MIN_NUM_PSEUDOS_) {
    tot = 0;
    mean = 0;
    totsq = 0;
    for(k = 0; k < getNumVals(); k++) {
      if(nttdistr->isValue(k, 1)  && (*doublevals_)[k] < posinit_[0] - posinit_[1] && (! gamma_ || ((GammaDistribution*)(negdistr_))->aboveMin((*doublevals_)[k]))) {
	mean += (*doublevals_)[k];
	totsq += (*doublevals_)[k] * (*doublevals_)[k];
	tot++;
      }
    }
    if (tot >= MIN_NUM_PSEUDOS_) {
      minNTT = 1;
    }
  }

  //    cout << "tot: " << tot << " vs " << MIN_NUM_PSEUDOS_ << endl;
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
      if (zero < minval_) {
	zero = minval_;
      }
    } // gamma


    //      cout << "zero: " << zero << ", min data val: " << minval_ << endl;

    tot = 0; // restart
    mean = 0.0;
    totsq = 0.0;
    for(int k = 0; k < getNumVals(); k++) {
      if((*doublevals_)[k] > zero && (*doublevals_)[k] < posinit_[0] - posinit_[1]) {
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
      negdistr_->init(newsettings);
      negmean_ = mean - negmean_num_stds[charge_] * stdev;
    }
    else {
      negdistr_->init(newsettings, False);
      negmean_ = zero + mean - negmean_num_stds[charge_] * stdev;
    }

    delete [] newsettings;

    if(negmean_ > MAX_SINGLY_NEGMEAN) {
      negmean_ = MAX_SINGLY_NEGMEAN;
    }
    //    cout << "setting negmean to " << negmean_ << " for charge " << (charge_+1) << " with zero " << zero << endl;

    USE_TR_NEG_DISTR_ = True;

    return False; // done
  } 
    
  mean /= tot;
  stdev = (totsq / tot) - mean * mean;
  stdev = sqrt(stdev);  // delete this later ?

  if(gamma_) {
    zero = mean - NUM_DEVS_ * stdev;
    //    cout << "zero: " << zero << " mean: " << mean << " numdevs: " << NUM_DEVS_ << " stdev: " << stdev << endl;
    if(min_dataval_ - ZERO_SET_ * stdev > zero) {
      zero = min_dataval_ - ZERO_SET_ * stdev;
    }
    if (zero < minval_) {
      zero = minval_;
    }
  }
  else {
    zero = 0.0;
  }
  //  cout << "zero: " << zero << " minval " << min_dataval_ << endl;
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
  double pmean = 0.0;
  double pstdev = 0.0;
  double ptotsq = 0.0;
  double ptot = 0;
    
  for(k = 0; k < getNumVals(); k++) {
    if(nttdistr->isValue(k, minNTT) && (*doublevals_)[k] > zero ) {
      mean += (*doublevals_)[k] - zero;
      totsq += ((*doublevals_)[k] - zero) * ((*doublevals_)[k] - zero);
      tot++;
    }
    else if (nttdistr->isValue(k, 2) && (*doublevals_)[k] > posinit_[0]-posinit_[1] ) {
      pmean += (*doublevals_)[k];
      ptotsq += ((*doublevals_)[k]) * ((*doublevals_)[k]);
      ptot++;
    }
  } // next
  
  if(tot > 0) {
    //stdev = sqrt( (totsq - (mean*mean) / tot) / (tot - 1) );
    mean /= tot;
    stdev = (totsq / tot) - mean * mean;
    stdev = sqrt(stdev);
  }
  if(ptot > 0) {
    //stdev = sqrt( (totsq - (mean*mean) / tot) / (tot - 1) );
    pmean /= ptot;
    pstdev = (ptotsq / ptot) - pmean * pmean;
    pstdev = sqrt(pstdev);
  }
  else {
    pmean = posinit_[0];
    pstdev = posinit_[1];
  }
  Boolean reset = False;
  double newposmean = pmean;
  double newposstdev = pstdev;  
  setNegativeDistr(mean, stdev, zero);
  //((GammaDistribution*)negdistr_)->init(neginit_, True);
  setPositiveDistr(newposmean, newposstdev);
  ((ContinuousDistribution*)negdistr_)->setTot(1);
  ((ContinuousDistribution*)posdistr_)->setTot(1);
  
  int c=0;
  while(c < 100 && ! maldi_ && noDistr()) {
    newposmean += 0.5;
    //newposstdev += 0.95;
    setPositiveDistr(newposmean, newposstdev);
    c++;
    //reset = True;
  }
  //if(reset) {
  //setPositiveDistr(newposmean, newposstdev);
  //}

  return True;
}
