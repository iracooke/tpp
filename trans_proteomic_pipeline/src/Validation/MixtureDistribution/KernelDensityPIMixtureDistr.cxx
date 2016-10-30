#include "KernelDensityPIMixtureDistr.h"

/*

Program       : KernelDensityPIDiscrMixtureDistr for PeptideProphet                                                       
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


KernelDensityPIMixtureDistr::KernelDensityPIMixtureDistr(int charge, const char* name, const char* tag, double range, double window, double orig) : pIMixtureDistr(charge, name, tag) {
  offset_init_ = orig;
  offset_ = offset_init_;
  vals_ = new Array<double>;
  doublevals_ = vals_;
  pIvals_ = new Array<double>;
  model_ = new KDModel(name, 100);
  update_ctr_ = 0;
  min_ctr_ = 2; 
  ready_ = false;
}

void KernelDensityPIMixtureDistr::write_pIstats(ostream& out) {
  for (int i = 0; i < run_pI_calc_->size(); i++) {
    (*run_pI_calc_)[i]->write_pIstats(out);
  }
}


void KernelDensityPIMixtureDistr::recalc_pIstats(Array<Array<double>*>* probs) {
  vals_->reserve(vals_->size());
  pIvals_->reserve(pIvals_->size());
  for (int i = 0; i < run_pI_calc_->size(); i++) {
    (*run_pI_calc_)[i]->recalc_pIstats((*probs)[i]);
    for (int j=0; j < (*run_pI_calc_)[i]->pIs_.size(); j++) {
      double dval = (*run_pI_calc_)[i]->getpIScore((*run_pI_calc_)[i]->pIs_[j], (char*)(*(*run_pI_calc_)[i]->peps_)[j]);
      pIvals_->insertAtEnd((*run_pI_calc_)[i]->pIs_[j]);
      vals_->insertAtEnd(dval);
      model_->insert((*(*probs)[i])[j], dval);
    }
  }

}

void KernelDensityPIMixtureDistr::recalc_pIstats(Array<Array<double>*>* probs, double min_prob, Array<Array<int>*>* ntts, int  min_ntt) {
  vals_->reserve(vals_->size());  // resets container count to 0, but leaves allocation alone for efficient repopulation
  pIvals_->reserve(pIvals_->size());  // resets container count to 0, but leaves allocation alone for efficient repopulation
  for (int i = 0; i < run_pI_calc_->size(); i++) {
    (*run_pI_calc_)[i]->recalc_pIstats((*probs)[i], min_prob, (*ntts)[i], min_ntt);
    for (int j=0; j < (*run_pI_calc_)[i]->pIs_.size(); j++) {
      double dval = (*run_pI_calc_)[i]->getpIScore((*run_pI_calc_)[i]->pIs_[j], (char*)(*(*run_pI_calc_)[i]->peps_)[j]);
      pIvals_->insertAtEnd((*run_pI_calc_)[i]->pIs_[j]);
      vals_->insertAtEnd(dval);
      model_->insert((*(*probs)[i])[j], dval);
    }
  
  }

}

void KernelDensityPIMixtureDistr::calc_pIstats() {
  vals_->reserve(vals_->size());
  pIvals_->reserve(pIvals_->size());
  for (int i = 0; i < run_pI_calc_->size(); i++) {
    (*run_pI_calc_)[i]->calc_pIstats();
    for (int j=0; j < (*run_pI_calc_)[i]->pIs_.size(); j++) {
      double dval = (*run_pI_calc_)[i]->getpIScore((*run_pI_calc_)[i]->pIs_[j], (char*)(*(*run_pI_calc_)[i]->peps_)[j]);
      pIvals_->insertAtEnd((*run_pI_calc_)[i]->pIs_[j]);
      vals_->insertAtEnd(dval);
      model_->insert(0.5, dval);
    }
  }

}

void KernelDensityPIMixtureDistr::enter(int index, char* val) {
  double dval = atof(val);
  vals_->insertAtEnd(dval); // put the double value here
  model_->insert(0.5, dval);
}


Boolean KernelDensityPIMixtureDistr::update(Array<Array<double>*>* all_probs) {
  Boolean rtn = False;

  Array<double>* probs = new Array<double>;
  
  
  for (int i=0; i<all_probs->size(); i++) {
    int len = (*all_probs)[i]->size();
    for (int j=0; j<len; j++) {
      probs->insertAtEnd((*(*all_probs)[i])[j]);
    }
  }
  
  if (!ready_) {  
    recalc_pIstats(all_probs);
    //    model_->makeReady(1,1);
    model_->makeReady(true, 10);
    ready_ = true;
  }
  else if(model_->update(probs,0)) {
    rtn = True;
  }

  delete probs;
  return rtn;

}
Boolean KernelDensityPIMixtureDistr::update(Array<Array<double>*>* all_probs, double min_prob, Array<Array<int>*>* all_ntts, int min_ntt) {
   Boolean rtn = False;

  Array<double>* probs = new Array<double>;
  
  
  for (int i=0; i<all_probs->size(); i++) {
    int len = (*all_probs)[i]->size();
    for (int j=0; j<len; j++) {
      probs->insertAtEnd((*(*all_probs)[i])[j]);
    }
  }
  
  if (!ready_) {  
    recalc_pIstats(all_probs, min_prob, all_ntts, min_ntt);
    //    model_->makeReady(1,1);
    model_->makeReady(true, 10);
    ready_ = true;
  }
  else if(model_->update(probs,0)) {
    rtn = True;
  }

  delete probs;
  return rtn;


}


Array<Tag*>* KernelDensityPIMixtureDistr::getMixtureDistrTags(const char* name) {
  Array<Tag*>* output = model_->reportTags();
  return output;

}


double KernelDensityPIMixtureDistr::getPosProb(int index) {
  if (ready_) 
    return model_->getPosProb((*vals_)[index]);  
  
  return 0.5;
}

double KernelDensityPIMixtureDistr::getNegProb(int index) {
   if (ready_)
    return model_->getNegProb((*vals_)[index]);
  
  return 0.5;
}

#define RESLEN 32
char* KernelDensityPIMixtureDistr::getStringValue(int index) {
  char* output = new char[RESLEN+1];
   if(vals_ != NULL)
      snprintf(output, RESLEN, "%0.2f", (*vals_)[index]);

  return output;
}

char* KernelDensityPIMixtureDistr::getStringpIValue(int index) {
  char* output = new char[RESLEN+1];
   if(pIvals_ != NULL)
      snprintf(output, RESLEN, "%0.2f", (*pIvals_)[index]);
  return output;
}

int KernelDensityPIMixtureDistr::getNumVals() { 
  if (vals_ != NULL) {
    return vals_->size();
  }
  return 0;
}
