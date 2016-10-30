#ifndef MZ_DIFF_DISCR_H
#define MZ_DIFF_DISCR_H

#include "DiscreteMixtureDistr.h"

/*

Program       : MzDifferenceDiscrMixtureDistr for PeptideProphet                                                       
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

class MzDifferenceDiscrMixtureDistr : public DiscreteMixtureDistr {

 public:

  MzDifferenceDiscrMixtureDistr(int charge, const char* name, const char* tag, double range, double window);
  int inttranslate(const char* val);
  Boolean haveDataWithValue(int bin);
  void writeDistr(FILE* fout);
  void enter(SearchResult* result);

 protected:

  int getIntegralValue(double val);


  double range_;
  double window_;


};

#endif
