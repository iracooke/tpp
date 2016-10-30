#ifndef ISOMASSDIFF_DISTR
#define ISOMASSDIFF_DISTR

#include "DiscreteMixtureDistr.h"
#include "common/constants.h"
/*

Program       : IsoMassDiffDiscrMixtureDistr for PeptideProphet                                                       
Author        : David Shteynberg <dshteynb@systemsbiology.org>                                                       
Date          : 01.29.07 

Mixture distributions for peptide number of tryptic termini

Copyright (C) 2007 David Shteynberg

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

Institute for Systems Biology, hereby disclaims all copyright interest 
in PeptideProphet written by David Shteynberg

*/


class IsoMassDiffDiscrMixtureDistr : public DiscreteMixtureDistr {

 public:
  IsoMassDiffDiscrMixtureDistr(int charge, const char* name, const char* tag);
  virtual ~IsoMassDiffDiscrMixtureDistr() {};
  double getIsoMassDiffPosFraction (int ntt);
  double getIsoMassDiffNegFraction (int ntt);
  int getIsoMassDiffValue(int index);
  void enter(SearchResult* result);

  // these were added to enforce constraint that if IsoMassDiff(2) > IsoMassDiff(1), then IsoMassDiff(1) must be greater than IsoMassDiff(0)
  Boolean update(Array<double>* probs);
  double getPosProb(int index);
  double getNegProb(int index);
  void writeDistr(FILE* fout);
  //Array<Tag*>* getMixtureDistrTags(char* name);
  void updateName(const char* name);

};

#endif
