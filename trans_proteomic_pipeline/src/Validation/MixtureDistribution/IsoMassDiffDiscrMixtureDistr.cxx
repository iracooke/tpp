#include "IsoMassDiffDiscrMixtureDistr.h"

/*

Program       : IsoMassMixtureDistr for PeptideProphet                                                       
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
in PeptideProphet written by Andrew Keller

*/


IsoMassDiffDiscrMixtureDistr::IsoMassDiffDiscrMixtureDistr(int charge, const char* name, const char* tag) : DiscreteMixtureDistr(charge, 7, name, tag) {
  const char* bindefs[] = {"-3", "-2", "-1", "0", "1", "2", "3"};
  maxdiff_ = 0.001;
  negOnly_ = True;
  DiscreteMixtureDistr::init(bindefs);
}

void IsoMassDiffDiscrMixtureDistr::updateName(const char* name) {
  name_ = name;
}

double IsoMassDiffDiscrMixtureDistr::getIsoMassDiffPosFraction (int ntt) {
  return posdistr_->getProb(ntt);
}

double IsoMassDiffDiscrMixtureDistr::getIsoMassDiffNegFraction (int ntt) {
  return negdistr_->getProb(ntt);
}

int IsoMassDiffDiscrMixtureDistr::getIsoMassDiffValue(int index) {
  return (*intvals_)[index];
}

void IsoMassDiffDiscrMixtureDistr::enter(SearchResult* result) {
  double massdiff = result->massdiff_;
  int tmp = (int)massdiff;
  if (massdiff - ((double)tmp*_ISOMASSDIS_) > 0.5) { 
    massdiff =  massdiff - ((double)(tmp+1)*_ISOMASSDIS_);
    tmp = tmp+1;
  }
  else if (massdiff - ((double)tmp*_ISOMASSDIS_) > 0.0) {
    massdiff =  massdiff - ((double)tmp*_ISOMASSDIS_);
  }
  else if (massdiff - ((double)tmp*_ISOMASSDIS_) < -0.5) {
    massdiff =  massdiff - ((double)(tmp-1)*_ISOMASSDIS_);
    tmp = tmp-1;
  }
  else {
    massdiff =  massdiff - ((double)tmp*_ISOMASSDIS_);
  }
  if (tmp < -3) {
    tmp = -3;
  }
  else if (tmp > 3) {
    tmp = 3;
  }
  intvals_->insertAtEnd(tmp);
}


double IsoMassDiffDiscrMixtureDistr::getPosProb(int index) {
 if(intvals_ != NULL) {
    if(index < 0 || index >= intvals_->size()) {
      cerr << "violation of index " << index << " for " << intvals_->size() << endl;
      exit(1);
    }    
    return posdistr_->getProb((*intvals_)[index]+3);
 }
 return 0.0;
}

double IsoMassDiffDiscrMixtureDistr::getNegProb(int index) {
  if(intvals_ != NULL) {
    if(index < 0 || index >= intvals_->size()) {
      cerr << "violation of index " << index << " for " << intvals_->size() << endl;
      exit(1);
    }    
    return negdistr_->getProb((*intvals_)[index]+3);
  }
  return 0.0;
}

Boolean IsoMassDiffDiscrMixtureDistr::update(Array<double>* probs) { 

  if(priors_ == NULL && intvals_->length() > 0) {
    priors_ = new double[numbins_];
    int k;
    for(k = 0; k < numbins_; k++) {
      priors_[k] = 0.0;
    }
    for(k = 0; k < intvals_->length(); k++) {
      priors_[(*intvals_)[k]+3] += 1.0;
    }
    for(k = 0; k < numbins_; k++) {
      priors_[k] /= intvals_->length();
    }
    posdistr_->setPriors(priors_, numpos_priors_);
    negdistr_->setPriors(priors_, numneg_priors_);
  } // if no priors yet
  posdistr_->initUpdate(NULL);
  negdistr_->initUpdate(NULL);

  for(int k = 0; k < probs->length(); k++) {
    posdistr_->addVal((*probs)[k], (*intvals_)[k]+3);
    negdistr_->addVal(1.0 - (*probs)[k], (*intvals_)[k]+3); 
  }
  
  Boolean output = posdistr_->update();
 
  
  if(negdistr_->update()) {
    output = True;
  }
  return output;


}

void IsoMassDiffDiscrMixtureDistr::writeDistr(FILE* fout) {
  fprintf(fout, "%s\n", getName());
  fprintf(fout, "\tpos: ");
  fprintf(fout, "(");
  int k;
  for(k = 0; k < numbins_; k++) {
    fprintf(fout, "%0.3f %s", posdistr_->getProb(k), (*bindefs_)[k]);
    if(k < numbins_ - 1) {
      fprintf(fout, ", ");
    }
  }

  fprintf(fout, "\tneg: ");
  fprintf(fout, "(");
  for(k = 0; k < numbins_; k++) {
    fprintf(fout, "%0.3f %s", negdistr_->getProb(k), (*bindefs_)[k]);
    if(k < numbins_ - 1) {
      fprintf(fout, ", ");
    }
  }
  fprintf(fout, ")\n");
}

