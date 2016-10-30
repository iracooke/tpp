#ifndef ACCMASS_OR_MASSD_H
#define ACCMASS_OR_MASSD_H

#include <assert.h>
#include "common/constants.h"
#include "MassDifferenceDiscrMixtureDistr.h"
#include "Validation/InterProphet/InterProphetParser/KDModel.h"

/*

Program       : AccurateMassDiffDiscrMixtureDistr for PeptideProphet                                                      
Authors       : Andrew Keller <akeller@systemsbiology.org>, David Shteynberg                                                        
Date          : 11.27.02 

Copyright (C) 2003 Andrew Keller (c) 2008 David Shteynberg

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

class AccurateMassDiffDiscrMixtureDistr : public MassDifferenceDiscrMixtureDistr {

 public:

  AccurateMassDiffDiscrMixtureDistr(int charge, const char* name, const char* tag, double range, double window, double orig, Boolean ppm = False);
  virtual ~AccurateMassDiffDiscrMixtureDistr() { 
    // vals_ deleted in MixtureDistr as doublevals_
    delete model_;
  }
  Boolean update(Array<double>* probs);
  Boolean update(Array<Array<double>*>* probs);
  void enter(SearchResult* result);
  Array<Tag*>* getMixtureDistrTags(const char* name);
  char* getStringValue(int index);
  double getPosProb(int index);
  double getNegProb(int index);
  int getNumVals();
 protected:
  
  KDModel* model_;

  double offset_;
  double offset_init_;
  Array<double>* vals_;
  int update_ctr_;
  int min_ctr_;
  Boolean offset_set_;
  bool ready_;
};


#endif
