/*

Program       : InterProphet                                                       
Author        : David Shteynberg <dshteynb  AT systemsbiology.org>                                                       
Date          : 12.12.07

Primary data object holding all mixture distributions for each precursor ion charge

Copyright (C) 2007 David Shteynberg

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

David Shteynberg
Institute for Systems Biology
401 Terry Avenue North 
Seattle, WA  98109  USA
akeller@systemsbiology.org

*/

#include "NSSModel.h"

NSSModel::NSSModel()  {
  isready_ = false;
  postot_ = 0;
  negtot_ = 0;
  num_bins_ = 8;
  binthresh_ = new Array<double>(num_bins_-1);
  bincount_ = new Array<double>(num_bins_);
  posprob_ = new Array<double>(num_bins_);
  negprob_ = new Array<double>(num_bins_);
  
  
  //TODO Dynamic binning or better yet kernel density estimate
  (*binthresh_)[0] = -3;
  (*binthresh_)[1] = -1;
  (*binthresh_)[2] = -0.5;
  (*binthresh_)[3] = 0;
  (*binthresh_)[4] = 0.5;
  (*binthresh_)[5] = 1;
  (*binthresh_)[6] = 3;
  
  (*bincount_)[0] = 0;
  (*bincount_)[1] = 0;
  (*bincount_)[2] = 0;
  (*bincount_)[3] = 0;
  (*bincount_)[4] = 0;
  (*bincount_)[5] = 0;
  (*bincount_)[6] = 0;
  (*bincount_)[7] = 0;
}

void NSSModel::insert(double prob, double val) {
  if (prob < 0 || prob > 1) {
    return;
  }

  postot_ += prob;
  negtot_ += 1 - prob;
  
  int i = 0;
  while (i < num_bins_ - 1 && val > (*binthresh_)[i]) {
    i++;
  }
  (*bincount_)[i]++;
  (*posprob_)[i] += prob;
  (*negprob_)[i] += 1-prob;
}


void NSSModel::makeReady() {
  for (int i=0; i<num_bins_; i++) {
    (*posprob_)[i] /= postot_;
    (*negprob_)[i] /= negtot_;
  }
  isready_ = true;
}

double NSSModel::getPosProb(double val) {
  //TODO: smoothing
  int i = 0;
  while (i < num_bins_ - 1  && val > (*binthresh_)[i] ) {
    i++;
  }
  return (*posprob_)[i];
}

double NSSModel::getNegProb(double val) {
  //TODO: smoothing
  int i = 0;
  while (i < num_bins_ - 1 && val > (*binthresh_)[i] ) {
    i++;
  }
  return (*negprob_)[i];
}

void NSSModel::report() {
  cout << "MinBound(excl)\tMaxBound(incl)\tNegProb\tPosProb\tBinCount" << endl;

  int i = 0;

  cout << "-inf\t" 
       << (*binthresh_)[i] << "\t" 
       << (*negprob_)[i] << "\t" 
       << (*posprob_)[i] << "\t" 
       << (*bincount_)[i] << endl;

  while (i < num_bins_ - 2 ) {
    cout << (*binthresh_)[i] << "\t" 
	 << (*binthresh_)[i+1] << "\t" 
	 << (*negprob_)[i+1] << "\t" 
	 << (*posprob_)[i+1] << "\t" 
	 << (*bincount_)[i+1] << endl ;
    i++;
  }

  cout << (*binthresh_)[i] 
       << "\tinf"  << "\t" 
       << (*negprob_)[i+1] << "\t" 
       << (*posprob_)[i+1] << "\t" 
       << (*bincount_)[i+1] << endl;

}

NSSModel::~NSSModel() {
  delete binthresh_;
  delete bincount_;
  delete posprob_;
  delete negprob_;
}
