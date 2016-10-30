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

#include "BoolModel.h"

BoolModel::BoolModel(const char* name)  {
  isready_ = false;
  postot_ = 0;
  negtot_ = 0;
  num_bins_ = 2;
  
  name_ = name;
  posprob_[0] = 0;
  posprob_[1] = 0;
  negprob_[0] = 0;
  negprob_[1] = 0;

}

void BoolModel::insert(double prob, bool val) {
  if (prob < 0 || prob > 1) {
    return;
  }

  postot_ += prob;
  negtot_ += 1 - prob;
  
  int i = val ? 1 : 0;
  
  posprob_[i] += prob;
  negprob_[i] += 1-prob;
}

void BoolModel::clear() {
  isready_ = false;
  postot_ = 0;
  negtot_ = 0;
  num_bins_ = 2;
  
  posprob_[0] = 0;
  posprob_[1] = 0;
  negprob_[0] = 0;
  negprob_[1] = 0;
}

bool BoolModel::makeReady() {
  for (int i=0; i<num_bins_; i++) {
    posprob_[i] /= postot_;
    negprob_[i] /= negtot_;
  }
  isready_ = true;
  return isready_;
}
void BoolModel::report(ostream& out) {
  out << "<mixturemodel name=\"" << name_ << "\">" << endl;
  if (isready_) {
    out << "<bin value=\"true\" pos_prob=\"" << posprob_[1] 
	<< "\" neg_prob=\"" << negprob_[1]  << "\"/> ";
    out << "<bin value=\"false\" pos_prob=\"" << posprob_[0] 
	<< "\" neg_prob=\"" << negprob_[0]  << "\"/> ";
  }
  out << "</mixturemodel>" << endl;
}


double BoolModel::getPosProb(bool i) {
  int x = i ? 1 : 0;
  return posprob_[x];
}

double BoolModel::getNegProb(bool i) {
  int x = i ? 1 : 0;
  return negprob_[x];
}


BoolModel::~BoolModel() {

}

