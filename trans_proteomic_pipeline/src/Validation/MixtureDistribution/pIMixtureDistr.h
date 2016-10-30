#ifndef pI_DISTR_H
#define pI_DISTR_H

#include "DiscreteMixtureDistr.h"
#include "Parsers/Algorithm2XML/pICalculator.h"

/*

Program       : pIMixtureDistr for PeptideProphet                                                       
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


class pIMixtureDistr : public DiscreteMixtureDistr {

 public:
  pIMixtureDistr(int charge, const char* name, const char* tag);
  ~pIMixtureDistr();
  double getPosProb(int index); 
  double getNegProb(int index);
#ifndef USE_STD_MODS
  int inttranslate(const char* val);
#endif

  void enter(SearchResult* result);
  void calc_pIstats();

 protected:

  int getpIBinNo(double pI);
  //DDS: pI model, separate pI_calc_ by runs so make
  pICalculator* pI_calc_;
  Array<pICalculator*>* run_pI_calc_;
  double range_;
  double window_;

};


#endif
