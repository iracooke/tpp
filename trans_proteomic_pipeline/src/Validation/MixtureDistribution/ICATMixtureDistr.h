#ifndef ICAT_DISTR_H
#define ICAT_DISTR_H

#include "DiscreteMixtureDistr.h"
#include "common/constants.h"


/*

Program       : ICATMixtureDistr for PeptideProphet                                                       
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


class ICATMixtureDistr : public DiscreteMixtureDistr {

 public:
 // HENRY: add optional argument to allow for matching uncleavable or cleavable ICAT only
 // icatType = 0 : any ICAT
 // icatType = 1 : cleavable ICAT only
 // icatType = 2 : uncleavable ICAT only
  ICATMixtureDistr(int charge, const char* name, const char* tag, int icatType = 0);


#ifndef USE_STD_MODS
  int inttranslate(const char* val);
#endif
  Boolean icatCompatible(const char* pep);
  void enter(SearchResult* result);

  static double light_icat_masses_[]; // = {545.2, 330,26}; // old fashioned, cleavable
  static double heavy_icat_masses_[]; // = {553.2, 339.26}; // 
  static int num_icat_;
  static Boolean isICAT(double mass, double error);

 protected:


};


#endif
