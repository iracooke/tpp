#include "ContinuousDistribution.h"

/*

Program       : ContinuousDistribution for PeptideProphet                                                       
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


ContinuousDistribution::ContinuousDistribution() : Distribution() { }
ContinuousDistribution::ContinuousDistribution(double maxdiff) : Distribution(maxdiff) {
  use_stdev_ = True;
 }

void ContinuousDistribution::init(double* prior) {
    if(prior == NULL) {
      delete[] newtot_;  // this was leaking - bpratt
      newtot_ = new double[1];
      newtot_[0] = 0.0;
      newtotwt_ = 0.0;
      newtotsq_ = 0.0;
    }
    set_ = False;
}
void ContinuousDistribution::init(double* prior, Boolean alphabeta) {
  init(prior);
}
void ContinuousDistribution::ignoreStdev() { use_stdev_ = False; }

double ContinuousDistribution::slice(double left_val, double right_val) {
  return slice(newtotwt_, left_val, right_val);
}

double ContinuousDistribution::getMean() { return mean_; }
void ContinuousDistribution::setMean(double mean) { mean_ = mean; }
double ContinuousDistribution::getStdev() { return stdev_; }

void ContinuousDistribution::resetTot() { newtotwt_ = 0.0; }
void ContinuousDistribution::setTot(double tot) { newtotwt_ = tot; }
//double ContinuousDistribution::getTot() { return newtotwt_; }
void ContinuousDistribution::setStdev(double stdev) { stdev_ = stdev; }
