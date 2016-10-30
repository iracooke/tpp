#include "SpectraSTDiscrimValMixtureDistr.h"
#include <boost/iterator/iterator_concepts.hpp>

/*

Program       : SpectraSTDiscrimValMixtureDistr for PeptideProphet                                                       
Author        : Henry Lam <hlam@systemsbiology.org>                                                       
Date          : 04.10.06 

Copyright (C) 2006 Henry Lam

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

Henry Lam
Institute for Systems Biology
401 Terry Avenue North 
Seattle, WA  98109  USA
hlam@systemsbiology.org

*/
SpectraSTDiscrimValMixtureDistr::SpectraSTDiscrimValMixtureDistr(int charge, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean nonparam, Boolean optimizefval) {  
  initializeDistr(charge, name, tag);
  all_negs_ = False;
  maldi_ = maldi;
  maxdiff_ = 0.005;
  gamma_ = False; //True;
  qtof_ = qtof;
  nonparam_ = nonparam;
  optimize_fval_ = optimizefval;
  sum_fval_ = 0.0;
  min_fval_ = 0.0;
  max_fval_ = 1.0;
  conservative_num_stdev_ = 0.0;
  
  doublevals_ = new Array<double>;
  if (nonparam_) {
    negdistr_ = new NonParametricDistribution(0.01, True);
    posdistr_ = new NonParametricDistribution(0.01, False);
  }
  else {
    posdistr_ = new GeneralizedGaussianDistribution(maxdiff_);
    //    posdistr_ = new GaussianDistribution(maxdiff_);
    if (!gamma_) {
      negdistr_ = new ExtremeValueDistribution(maxdiff_);
    } else {
      negdistr_ = new GammaDistribution(maxdiff_);
    }
  }
  //    double posprior[] = {0.63, 0.075}; // {mean, stdev} for the Gaussian
  //  	double posprior[] = {0.65, 0.005}; // {mean, stdev} for the Gaussian

  posinit_ = NULL;
  neginit_ = NULL;

  double posprior[] = {0.3, 0.1}; // {mean, stdev} for the Generalized Gaussian
  //   	double posprior[] = {0.65, 0.005}; // {mean, stdev} for the Gaussian
  posinit_ = copy(posprior, 2);

  if (!gamma_) {
  
    double negprior[] = {0.1, 0.05}; // {mean, stdev} for ExtremeValue
    neginit_ = copy(negprior, 3);

  } else {
    double negprior[] = {40.0, 4.0, 0.0}; // {alpha, beta, gamma} for Gamma
    // NOTE: mean = beta / alpha, stdev = sqrt(mean / alpha), gamma is the zero
    neginit_ = copy(negprior, 3);
  }
  
  // TODO: set negprior for {alpha, beta, gamma} for the Gamma
  //  double negprior[] = {0.25, 0.07, 0.0}; // {alpha, beta, gamma} for the Gamma
//    double negprior[] = {0.2, 0.01, 0.0}; // {alpha, beta, gamma} for the Gamma
	
  minval_ = 0.0;
  negmean_ = 0.0;

  MIN_NUM_PSEUDOS_ = 50;
  ZERO_SET_ = 100; // ?
  NUM_DEVS_ = 6;
  USE_TR_NEG_DISTR_ = False;

  reset();
  
  //  ((GaussianDistribution*)(posdistr_))->ignoreStdev();
  // if (!nonparam_) ((GammaDistribution*)(negdistr_))->setDistrMinval(0.0);
  if (!nonparam_) {
    if (!gamma_) {    
      ((ExtremeValueDistribution*)(negdistr_))->setMinVal(0.0);  
    } else {
      ((GammaDistribution*)(negdistr_))->setDistrMinval(0.0);
    }
  }
}

void SpectraSTDiscrimValMixtureDistr::setConservative(float num_stdev) { 
  conservative_num_stdev_ = num_stdev;
}


void SpectraSTDiscrimValMixtureDistr::enter(SearchResult* result) {

  SpectraSTResult* r = (SpectraSTResult*)result;
  
  if (!optimize_fval_) {
    sum_fval_ += r->fval_;
    if (r->fval_ > max_fval_) max_fval_ = r->fval_;
    if (r->fval_ < min_fval_) min_fval_ = r->fval_;
    DiscrimValMixtureDistr::enter(result);
  } else {
    dots_.push_back(r->dot_);
    deltas_.push_back(r->delta_);
  }
}

void SpectraSTDiscrimValMixtureDistr::optimizeFval() {

  if (!optimize_fval_) {
    //  if (getNumVals() >= 50) {
    //    double mean_fval = sum_fval_ / (double)(getNumVals());
    //    double posmean[] = {mean_fval + 0.1, 0.1};
    //    double negmean[] = {(mean_fval - 0.1 > 0.1 ? (mean_fval - 0.1) : 0.1), 0.005, 0.0};
    //    posinit_ = copy(posmean, 2);
    //    neginit_ = copy(negmean, 3);
    //    reset();
    //  }
    return;
  }

  ((SpectraSTDiscrimFunction*)discrim_func_)->optimize(dots_, deltas_);

  // re-enter f-values
  sum_fval_ = 0.0;
  for (int i = 0; i < dots_.size() && i < deltas_.size(); i++) {
    double fval = ((SpectraSTDiscrimFunction*)discrim_func_)->getDiscriminantScore(dots_[i], deltas_[i]);
    sum_fval_ += fval;
    if (fval > max_fval_) max_fval_ = fval;
    if (fval < min_fval_) min_fval_ = fval;
    MixtureDistr::enter(i, fval);
  }

  if (getNumVals() >= 50) {
    double mean_fval = sum_fval_ / (double)(getNumVals());
    double posmean[] = {mean_fval + 0.1, 0.1};
    double negmean[] = {(mean_fval - 0.1 > 0.1 ? (mean_fval - 0.1) : 0.1), 0.005, 0.0};
    posinit_ = copy(posmean, 2);
    neginit_ = copy(negmean, 3);
    // cerr << "optneg=" << negmean[0] << ";optpos=" << posmean[0] << endl;
    reset();
  }

}



Boolean SpectraSTDiscrimValMixtureDistr::initializeNegDistribution(NTTMixtureDistr* nttdistr) {
 
  return (false);
 
}

Boolean SpectraSTDiscrimValMixtureDistr::update(Array<double>* probs) {
  Boolean rtn = DiscrimValMixtureDistr::update(probs);
  negmean_ = ((ContinuousDistribution*)negdistr_)->getMean() + conservative_num_stdev_ * ((ContinuousDistribution*)negdistr_)->getStdev();
  // negmean_ = ((ContinuousDistribution*)negdistr_)->getMean();
  // +1.75*((ContinuousDistribution*)negdistr_)->getStdev();
  return rtn;
}

Boolean SpectraSTDiscrimValMixtureDistr::noDistr() {

  double num_stdevs[] = {0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8}; // for each charge
  double min_pos_means[] = {0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1};

  if(qtof_ || (charge_ == 0 && maldi_)) {
    return False;
  } else {
    double posMean = ((ContinuousDistribution*)(posdistr_))->getMean();
    double posStDev = ((ContinuousDistribution*)(posdistr_))->getStdev();
    double negMean = ((ContinuousDistribution*)(negdistr_))->getMean();
    double negStDev = ((ContinuousDistribution*)(negdistr_))->getStdev();  
    double negZero = 0;
    if (!nonparam_ && gamma_) ((GammaDistribution*)(negdistr_))->getZero();
    
    if (posMean < min_pos_means[charge_]) {
      //cout << "WARNING: No +" << charge_ + 1 << " distribution because PosMean (" << posMean << ") is smaller than " << min_pos_means[charge_] << endl;
    }
    if (posMean + posStDev < negMean + negZero + negStDev * num_stdevs[charge_]) {
      //cout << "WARNING: No +" << charge_ + 1 << " distribution because PosMean (" << posMean << ") + PosStDev (" << posStDev << ") is smaller than ";
      //cout << " NegMean (" << negMean << ") + NegZero (" << negZero << ") + NegStDev (" << negStDev << ") * " << num_stdevs[charge_] << endl;
    }


    return ((posMean < min_pos_means[charge_]) ||
            (posMean + posStDev < negMean + negZero + negStDev * num_stdevs[charge_]));
  }
}


void SpectraSTDiscrimValMixtureDistr::setDiscrimFunction(const char* mass_spec_type) {
  //cout << "setting new probid discrim function...for input file: " << mass_spec_type << endl;
  discrim_func_ = new SpectraSTDiscrimFunction(charge_);
}

