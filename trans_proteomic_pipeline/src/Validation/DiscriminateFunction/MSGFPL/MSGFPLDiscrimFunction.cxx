#include "MSGFPLDiscrimFunction.h"

/*
 * WARNING!! This discriminant function is not yet complete.  It is presented
 *           here to help facilitate trial and discussion.  Reliance on this
 *           code for publishable scientific results is not recommended.
 */

/*

Program       : MSGFPLDiscriminantFunction for discr_calc of PeptideProphet                                                       
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

MSGFPLDiscrimFunction::MSGFPLDiscrimFunction(int charge) : DiscriminantFunction(charge) {   
   const_ = 0.0;
   min_val_ = -10.0;
   len_wt_ = 0;
}

Boolean MSGFPLDiscrimFunction::isComputable(SearchResult* result) {
  return True;
}
  
double MSGFPLDiscrimFunction::getDiscriminantScore(SearchResult* result) {
  if(strcasecmp(result->getName(), "MSGFPL") != 0) {
    cerr << "illegal type of MSGFPL result: " << result->getName() << endl;
    exit(1);
  }
  MSGFPLResult* tresult = (MSGFPLResult*)(result);

  double tot = const_;
  double msgf_val = (double)tresult->msgfScore_ ;
  double fval = -log10(msgf_val);

  tot += fval;

  return tot;
}

void MSGFPLDiscrimFunction::setMassSpecType(const char* mass_spec_type) {
}
