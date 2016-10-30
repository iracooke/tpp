#ifndef DISCR_MIX_DISTR_H
#define DISCR_MIX_DISTR_H

#include "MixtureDistr.h"
#include "common/Array.h"

/*

Program       : DiscreteMixtureDistr for PeptideProphet                                                       
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


class DiscreteMixtureDistr : public MixtureDistr {
 public:

  DiscreteMixtureDistr(int charge, int numbins, const char* name, const char* tag);
  virtual ~DiscreteMixtureDistr() { 
    if (bindefs_ != NULL)
      delete bindefs_; 
    if (priors_ != NULL)
      delete[] priors_; 

    bindefs_ = NULL;
    priors_ = NULL;
  };
 
  virtual void init(const char** bindefs);
  virtual void printDistr();
  virtual void printPosDistribution();
  virtual void printNegDistribution();
  Boolean update(Array<double>* probs);
  Boolean update(Array<Array<double>*>* probs) {return False;} ;
  void writeDistr(FILE* fout);
  Array<Tag*>* getMixtureDistrTags(const char* name);

 protected:

  void initializeBinDefs(const char** bindefs);
  double maxdiff_;
  Array<const char*>* bindefs_;
  int numbins_;
  double* priors_;
  double numpos_priors_;
  double numneg_priors_;



};

#endif
