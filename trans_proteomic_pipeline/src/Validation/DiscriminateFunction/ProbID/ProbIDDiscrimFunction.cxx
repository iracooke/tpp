#include "ProbIDDiscrimFunction.h"

/*

Program       : MascotDiscriminantFunction for discr_calc of PeptideProphet                                                       
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

ProbIDDiscrimFunction::ProbIDDiscrimFunction(int charge) : DiscriminantFunction(charge) { 
  double consts[] = {-8.0, -8.0, -10.0, -10.0, -10.0, -10.0, -10.0};
  double bays[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  double zscores[] = {1.5, 1.5, 2.0, 2.0, 2.0, 2.0, 2.0};
  const_ = consts[charge_];


  bays_wt_ = bays[charge_];
  zscore_wt_ = zscores[charge_];


}




Boolean ProbIDDiscrimFunction::isComputable(SearchResult* result) {
  return True;
}
  
double ProbIDDiscrimFunction::getDiscriminantScore(SearchResult* result) {
  if(strcasecmp(result->getName(), "ProbID") != 0) {
    cerr << "illegal type of ProbID result: " << result->getName() << endl;
    exit(1);
  }
  ProbIDResult* probidresult = (ProbIDResult*)(result);

  double tot = const_;
  double tmp = bays_wt_ * probidresult->bays_ + zscore_wt_ * probidresult->zscore_;
  tot += tmp;

  return tot;
}
