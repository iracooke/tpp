#include "TandemDiscrimFunction.h"

/*

Program       : TandemDiscriminantFunction for discr_calc of PeptideProphet                                                       
Author        : Brendan MacLean <bmaclean%at%fhcrc.org>                                                       
Date          : 06.20.06 


Copyright (C) 2006 Brendan MacLean and Andrew Keller

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

TandemDiscrimFunction::TandemDiscrimFunction(int charge, Boolean use_expect) : DiscriminantFunction(charge) { 
  const_ = 0;
  score_wt_ = 0;
  expect_wt_ = 0;
  delta_wt_ = 0;
  len_wt_ = 0;
  min_val_ = -100;
  use_expect_ = use_expect;
}

Boolean TandemDiscrimFunction::isComputable(SearchResult* result) {
  return True;
}
  
double TandemDiscrimFunction::getDiscriminantScore(SearchResult* result) {
  if(strcasecmp(result->getName(), "X! Tandem") != 0) {
    cerr << "illegal type of Tandem result: " << result->getName() << endl;
    exit(1);
  }
  TandemResult* tresult = (TandemResult*)(result);

  double tot = const_;
  
  double disc = score_wt_ * log((double)tresult->hyper_) 
    + expect_wt_ * (0-log((double)tresult->expect_)) 
    + delta_wt_ * (1.0 - (tresult->next_ / tresult->hyper_));

  if (len_wt_)
      disc /= len_wt_ * sqrt((double)strlen(tresult->peptide_));

  tot += disc;


  if (use_expect_) {
    tot = 3 * tot  - 8;
  }

  return tot;
}

void TandemDiscrimFunction::setMassSpecType(const char* mass_spec_type) {
}
