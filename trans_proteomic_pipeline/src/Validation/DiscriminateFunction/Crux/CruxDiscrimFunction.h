#ifndef CRUXDISCR_FUN_H
#define CRUXDISCR_FUN_H

#include <string.h>
#include <math.h>
#include <iostream>
#include <fstream>

#include "Parsers/Algorithm2XML/SearchResult/CruxResult.h"
//#include "WindowsCruxResult.h"
#include "Validation/DiscriminateFunction/DiscriminantFunction.h"
#include "Parsers/Algorithm2XML/SearchResult/SearchResult.h"
#include "common/sysdepend.h"

/*

Program       : DiscriminantFunction for discr_calc of PeptideProphet                                                       
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


class CruxDiscrimFunction : public DiscriminantFunction {

 public:

  CruxDiscrimFunction(int charge);
  CruxDiscrimFunction(int charge, Boolean use_xcorr = False);
  Boolean isComputable(SearchResult* result);
  virtual double getDiscriminantScore(SearchResult* result);
  double getXcorrP(double xcorr, int peplen);
  int getPepLen(char* pep);

  double getMinVal() { return min_val_; }

 protected:

  double xcorr_p_wt_;
  double delta_wt_;
  double log_rank_wt_;
  double abs_massd_wt_;
  double min_val_;

  int max_pep_len_;
  int num_frags_;
  Boolean use_xcorr_;

}; // class

#endif
