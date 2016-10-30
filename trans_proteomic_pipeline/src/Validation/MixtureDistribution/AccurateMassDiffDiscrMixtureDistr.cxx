#include "AccurateMassDiffDiscrMixtureDistr.h"

/*

Program       : AccurateMassDiffDiscrMixtureDistr for PeptideProphet                                                       
Author        : Andrew Keller <akeller@systemsbiology.org>, 
                David Shteynberg <2008>                                                    
Date          : 11.27.02 

Copyright (C) 2003 Andrew Keller 2008 David Shteynberg

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


AccurateMassDiffDiscrMixtureDistr::AccurateMassDiffDiscrMixtureDistr(int charge, const char* name, const char* tag, double range, double window, double orig, Boolean ppm) : MassDifferenceDiscrMixtureDistr(charge, name, tag, range, window, ppm) {
  offset_init_ = orig;
  offset_ = offset_init_;
  vals_ = new Array<double>();
  doublevals_ = vals_;
  intvals_ = NULL; //TODO: this pointer is allocated in MassDifferenceDiscrMixtureDistr, here we set it to null.  This needs to be fixed.
  offset_set_ = False;
  update_ctr_ = 0;
  min_ctr_ = 2; // min value for offset update
  model_ = new KDModel("Accurate Mass Model", 100);
  ready_ = false;
}

void AccurateMassDiffDiscrMixtureDistr::enter(SearchResult* result) {
  assert(vals_ != NULL);
  double massdiff = result->massdiff_;
  int tmp = (int)massdiff;

  if (massdiff - ((double)tmp*_ISOMASSDIS_) > 0.5) { 
    massdiff =  massdiff - ((double)(tmp+1)*_ISOMASSDIS_);
  }
  else if (massdiff - ((double)tmp*_ISOMASSDIS_) > 0.0) {
    massdiff =  massdiff - ((double)tmp*_ISOMASSDIS_);
  }
  else if (massdiff - ((double)tmp*_ISOMASSDIS_) < -0.5) {
    massdiff =  massdiff - ((double)(tmp-1)*_ISOMASSDIS_);
  }
  else {
    massdiff =  massdiff - ((double)tmp*_ISOMASSDIS_);
  }

  ///DDS : Try ACCMASS in ppm
  
  if (ppm_) massdiff = 1e6*massdiff/result->neutral_mass_;


  vals_->insertAtEnd(massdiff); // put the double value here


  //model_->insert(0.5, massdiff, -0.2, 0.2);
  model_->insert(0.5, massdiff);

}

int AccurateMassDiffDiscrMixtureDistr::getNumVals() {
  return vals_->size();
}

Boolean AccurateMassDiffDiscrMixtureDistr::update(Array<double>* probs) {
  Boolean output = False;
  if (!ready_) {
    //    model_->makeReady(0.01, 0.1);
    model_->makeReady(true);
    ready_ = true;
  }
  //  else if(model_->update(probs,0, -0.2, 0.2) || output) {
  else if(model_->update(probs,0) || output) {
      return True;
  }
  return False;

}

Boolean AccurateMassDiffDiscrMixtureDistr::update(Array<Array<double>*>* all_probs) {
  Boolean output = False;

  Array<double>* probs = new Array<double>;
  
  
  for (int i=0; i<all_probs->size(); i++) {
    int len = (*all_probs)[i]->size();
    for (int j=0; j<len; j++) {
      probs->insertAtEnd((*(*all_probs)[i])[j]);
    }
  }
  
  output = update(probs);

  delete probs;
  return output;

}

Array<Tag*>* AccurateMassDiffDiscrMixtureDistr::getMixtureDistrTags(const char* name) {
  Array<Tag*>* output = model_->reportTags();
  return output;

}

char* AccurateMassDiffDiscrMixtureDistr::getStringValue(int index) {
  char* output = new char[32];
  if(vals_ != NULL)
    sprintf(output, "%0.3f", (*vals_)[index]);
  else if(intvals_ != NULL)
    sprintf(output, "%0d", (*intvals_)[index]);
  return output;
}

double AccurateMassDiffDiscrMixtureDistr::getPosProb(int index) {
  if (ready_) 
    return model_->getPosProb((*vals_)[index]);  

  return 0.5;
}

double AccurateMassDiffDiscrMixtureDistr::getNegProb(int index) {
  if (ready_)
    return model_->getNegProb((*vals_)[index]);
  
  return 0.5;
}
