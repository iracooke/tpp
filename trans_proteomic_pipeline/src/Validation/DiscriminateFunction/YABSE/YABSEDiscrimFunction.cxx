#include "YABSEDiscrimFunction.h"

/*
 * WARNING!! This discriminant function is not yet complete.  It is presented
 *           here to help facilitate trial and discussion.  Reliance on this
 *           code for publishable scientific results is not recommended.
 */

/*

Program       : YABSEDiscriminantFunction for discr_calc of PeptideProphet                                                       
Author        : David Shteynberg <dshteynb%at%systemsbiology.org>                                                       
Date          : 05.16.07 


Copyright (C) 2007 David Shteynberg and Andrew Keller

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

YABSEDiscrimFunction::YABSEDiscrimFunction(int charge) : DiscriminantFunction(charge) { 
  const_ = 0;
  //pvalue_wt_ = 14;
  //expect_wt_ = 0;
  
  //const_ = -4;
  //  pvalue_wt_ = 0;
  //snratio_wt_ = 14.;

  hyper_wt_ = 0.09;

  delta_wt_ = 0.024;

  spc_wt_ = -0.079;
  
  len_wt_ = 0;
  min_val_ = -4;
}

Boolean YABSEDiscrimFunction::isComputable(SearchResult* result) {
  return True;
}
  
double YABSEDiscrimFunction::getDiscriminantScore(SearchResult* result) {
  if(strcasecmp(result->getName(), "YABSE") != 0) {
    cerr << "illegal type of YABSE result: " << result->getName() << endl;
    exit(1);
  }
  YABSEResult* tresult = (YABSEResult*)(result);

  double tot = const_;
  // double eval = (double)tresult->expect_;
  double hyperval =(double)tresult->hypergeo_;
  double deltahyperval =(double)tresult->deltahypergeo_;
  double spcval =(double)tresult->spc_;
  
  // if (eval <= 0) {
  //  eval = 500;
  //}
  //else {
  //  eval = -log10(eval);
  //}

  double disc = hyper_wt_ * hyperval + delta_wt_ * deltahyperval + spc_wt_ * spcval ; 

  if (len_wt_)
      disc /= len_wt_ * sqrt((double)strlen(tresult->peptide_));

  tot += disc;

  return tot;
}

void YABSEDiscrimFunction::setMassSpecType(const char* mass_spec_type) {
}
