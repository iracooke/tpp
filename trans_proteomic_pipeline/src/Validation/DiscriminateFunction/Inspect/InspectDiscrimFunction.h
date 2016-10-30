#ifndef Inspect_DISCR_FUN_H
#define Inspect_DISCR_FUN_H

#include "Validation/DiscriminateFunction/DiscriminantFunction.h"
#include "Parsers/Algorithm2XML/SearchResult/SearchResult.h"
#include "common/sysdepend.h"
#include "Parsers/Algorithm2XML/SearchResult/InspectResult.h"

/*
 * WARNING!! This discriminant function is not yet complete.  It is presented
 *           here to help facilitate trial and discussion.  Reliance on this
 *           code for publishable scientific results is not recommended.
 */

/*

Program       : InspectDiscriminantFunction for discr_calc of PeptideProphet                                                       
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

class InspectDiscrimFunction : public DiscriminantFunction {

 public:
  InspectDiscrimFunction(int charge);

  virtual Boolean isComputable(SearchResult* result);
  virtual double getDiscriminantScore(SearchResult* result);
  virtual void setMassSpecType(const char* mass_spec_type);

  double getMinVal() { return min_val_; }

 protected:
  double mq_wt_;
  double fscore_wt_;
  double massError_wt_;
  double mzSSE_wt_;
  double min_val_;
  double len_wt_;
};
#endif
