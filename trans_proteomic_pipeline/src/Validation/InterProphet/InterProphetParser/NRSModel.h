#ifndef _NRS_MODEL_
#define _NRS_MODEL_
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

#include <ostream>
#include "common/util.h"
#include "common/sysdepend.h"
#include "common/Array.h"
using namespace std;

class NRSModel {
 public:
  NRSModel();

  void insert(double prob, double val);

  void report();
  
  void makeReady();

  double getPosProb(double val);

  double getNegProb(double val);
  ~NRSModel() ;

 private:
  bool isready_;
  int num_bins_;
  double postot_;
  double negtot_;
  Array<double>* binthresh_;
  Array<double>* bincount_;
  Array<double>* posprob_;
  Array<double>* negprob_;
  
  
};

#endif
