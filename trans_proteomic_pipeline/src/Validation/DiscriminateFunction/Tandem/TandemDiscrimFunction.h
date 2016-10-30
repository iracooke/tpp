#ifndef TAN_DISCR_FUN_H
#define TAN_DISCR_FUN_H

#include "Validation/DiscriminateFunction/DiscriminantFunction.h"
#include "Parsers/Algorithm2XML/SearchResult/SearchResult.h"
#include "common/sysdepend.h"
#include "Parsers/Algorithm2XML/SearchResult/TandemResult.h"

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

class TandemDiscrimFunction : public DiscriminantFunction {

 public:
  TandemDiscrimFunction(int charge);
  TandemDiscrimFunction(int charge, Boolean use_expect = False);

  virtual Boolean isComputable(SearchResult* result);
  virtual double getDiscriminantScore(SearchResult* result);
  virtual void setMassSpecType(const char* mass_spec_type);

  double getMinVal() { return min_val_; }

 protected:
  double score_wt_;
  double expect_wt_;
  double delta_wt_;
  double len_wt_;
  double min_val_;
  Boolean use_expect_;
};
#endif
