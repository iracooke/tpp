#ifndef _BOOL_MODEL_
#define _BOOL_MODEL_
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

class BoolModel {
 public:
  BoolModel(const char* name);

  void insert(double prob, bool val);

  bool makeReady();
  void clear();
  double getPosProb(bool index);

  double getNegProb(bool index);

  void report(ostream& out);

  const char *getName() {
	  return name_.c_str();
  }

  ~BoolModel() ;

 private:
  bool isready_;

  double postot_;
  double negtot_;


  string name_;

  double num_bins_ ;
  double posprob_[2];
  double negprob_[2];


};

#endif
