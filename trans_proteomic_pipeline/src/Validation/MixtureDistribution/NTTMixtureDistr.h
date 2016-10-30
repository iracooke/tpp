#ifndef NTT_DISTR
#define NTT_DISTR

#include "DiscreteMixtureDistr.h"

/*

Program       : NTTMixtureDistr for PeptideProphet                                                       
Author        : Andrew Keller <akeller@systemsbiology.org>                                                       
Date          : 11.27.02 

Mixture distributions for peptide number of tryptic termini

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


class NTTMixtureDistr : public DiscreteMixtureDistr {

 public:
  NTTMixtureDistr(int charge, const char* name, const char* tag);
  virtual ~NTTMixtureDistr() { 
      delete[] oldpos_; 
      delete [] newpos_;
  };
  double getNTTPosFraction (int ntt);
  double getNTTNegFraction (int ntt);
  int getNTTValue(int index);
  void enter(SearchResult* result);

  // these were added to enforce constraint that if NTT(2) > NTT(1), then NTT(1) must be greater than NTT(0)
  Boolean update(Array<double>* probs);
  double getPosProb(int index);
  void writeDistr(FILE* fout);
  //Array<Tag*>* getMixtureDistrTags(const char* name);
  void updateName(const char* name);

 protected:

  // these were added to enforce constraint that if NTT(2) > NTT(1), then NTT(1) must be greater than NTT(0)
  Boolean constrain_; // whether or not to constrain NTT(0) vs NTT(1) and NTT(2)
  Boolean constrain_adj_; // whether or not distributions were adjusted due to constraint
  double* oldpos_; // prev distributions (for comparison)
  double* newpos_; // current distributions

};

#endif
