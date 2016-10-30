#ifndef SPECTRAST_DISCR_FUN_H
#define SPECTRAST_DISCR_FUN_H

#include "Validation/DiscriminateFunction/DiscriminantFunction.h"
#include "Parsers/Algorithm2XML/SearchResult/SearchResult.h"
#include "common/sysdepend.h"
#include "Parsers/Algorithm2XML/SearchResult/SpectraSTResult.h"


/*

Program       : SpectraSTDiscriminantFunction for discr_calc of PeptideProphet                                                       
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

class SpectraSTDiscrimFunction : public DiscriminantFunction {

 public:

  SpectraSTDiscrimFunction(int charge);
  Boolean isComputable(SearchResult* result);
  double getDiscriminantScore(SearchResult* result);
  double getDiscriminantScore(double dot, double delta); 
  void optimize(vector<double>& dots, vector<double>& deltas); 
  //double getMeanIdentity(char* xmlfile);

 protected:
  double dot_wt_;
  double normdeltadot_wt_;
  double meanDot_;
  double meanDelta_;

};

#endif // SPECTRAST_DISCR_FUN_H
