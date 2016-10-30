#include "DiscreteDistribution.h"


/*

Program       : DiscreteDistribution for PeptideProphet                                                       
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

#include <assert.h>

DiscreteDistribution::DiscreteDistribution() : Distribution() {   
   common_ctor_init();
}

DiscreteDistribution::~DiscreteDistribution() { 
  if(count_ != NULL) {
    delete [] count_;
  }
  if (newcount_ != NULL) {
    delete [] newcount_;
  }
  // note we don't manage the memory at *priors_
}

void DiscreteDistribution::common_ctor_init() {
	count_ = NULL;
	newcount_ = NULL;
	priors_ = NULL;
}


DiscreteDistribution::DiscreteDistribution(double maxdiff) : Distribution(maxdiff) { 
   common_ctor_init();
}

DiscreteDistribution::DiscreteDistribution(int numbins, double maxdiff) : Distribution(maxdiff) { 
  common_ctor_init();

  num_bins_ = numbins;

  tot_ = new double[num_bins_];
  count_ = new int[num_bins_];
  newcount_ = new int[num_bins_];
  for(int k = 0; k < num_bins_; k++) {
    tot_[k] = 1.0 / num_bins_;
    count_[k] = 0;
    newcount_[k] = 0;
  }
  totwt_ = 1.0;
  priors_ = NULL;
  num_priors_ = 0.0;
}

int DiscreteDistribution::getNumBins() { return num_bins_; }

double DiscreteDistribution::getProb(double val) {
  return -1;
}

void DiscreteDistribution::setPriors(double* priors, double numpriors) {
  priors_ = priors;
  num_priors_ = numpriors;
}

double DiscreteDistribution::getProb(int val) {
  assert (val >= 0 && val < num_bins_);
  return tot_[val] / totwt_;
}

int DiscreteDistribution::getCount(int val) {
  assert(val >= 0 && val < num_bins_);
  return count_[val];
}

void DiscreteDistribution::initUpdate(double* prior) {

  if(newtot_ == NULL) {
    newtot_ = new double[num_bins_];
  }
  if (newcount_ == NULL) {
    newcount_ = new int[num_bins_];
  }
  for(int k = 0; k < num_bins_; k++) {
    if(priors_ == NULL) {
      newtot_[k] = 0.0;
    }
    else {
      newtot_[k] = priors_[k] * num_priors_;
    }
  }
  if(priors_ == NULL) {
    newtotwt_ = 0.0;
  }
  else {
    newtotwt_ = num_priors_;
  }
}

void DiscreteDistribution::init(double* priors) {
    if(newtot_ == NULL) {
      newtot_ = new double[num_bins_];
    }
    if(newcount_ == NULL) {
      newcount_ = new int[num_bins_];
    }
    newtot_[0] = 0.0;
    newtotwt_ = 0.0;
    for(int k = 0; k < num_bins_; k++) {
      newtot_[k] = 0.0;
      newcount_[k] = 0;
    }
    set_ = False;
}

void DiscreteDistribution::addVal(double wt, double val) {
	cerr << "error: in DiscreteDistribution::addVal(double wt, double val)" << endl; // BSP
}

void DiscreteDistribution::addVal(double wt, int val) {
  newcount_[val] ++;
  newtot_[val] += wt;
  newtotwt_ += wt;
}

Boolean DiscreteDistribution::update() {

  Boolean output = False;
  double delta;
  for(int k = 0; k < num_bins_; k++) {
    if(! set_ || (totwt_ == 0 && newtotwt_ > 0) || (totwt_ > 0 && newtotwt_ > 0)) {
      delta = (tot_[k]/totwt_) - (newtot_[k]/newtotwt_);
      if(myfabs(delta) > maxdiff_) {
	output = True;
      }
    }

  }
  if(output) {
    for(int k = 0; k < num_bins_; k++) {
      tot_[k] = newtot_[k];
      count_[k] = newcount_[k];
    }
    totwt_ = newtotwt_;
    set_ = True;
  }
  if(! output) {
    //cout << "*** no change in discrete distribution..." << endl;
  }

  return output;
}
void DiscreteDistribution::writeDistr(FILE* fout) { 
}

void DiscreteDistribution::printDistr() {
}
