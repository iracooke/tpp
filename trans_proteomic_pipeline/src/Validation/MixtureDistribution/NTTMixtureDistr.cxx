#include "NTTMixtureDistr.h"

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


NTTMixtureDistr::NTTMixtureDistr(int charge, const char* const name, const char* tag) : DiscreteMixtureDistr(charge, 3, name, tag) {
  const char* bindefs[] = {"ntt=0", "ntt=1", "ntt=2"};
  maxdiff_ = 0.001;
  negOnly_ = True;
  DiscreteMixtureDistr::init(bindefs);
  constrain_ = True;
  oldpos_ = NULL;
  newpos_ = NULL;
  constrain_adj_ = False;
}

void NTTMixtureDistr::updateName(const char* name) {
  name_ = name;
}

double NTTMixtureDistr::getNTTPosFraction (int ntt) {
  return posdistr_->getProb(ntt);
}

double NTTMixtureDistr::getNTTNegFraction (int ntt) {
  return negdistr_->getProb(ntt);
}

int NTTMixtureDistr::getNTTValue(int index) {
  return (*intvals_)[index];
}

void NTTMixtureDistr::enter(SearchResult* result) {
  MixtureDistr::enter(0, result->num_tol_term_);
}

/*
Array<Tag*>* NTTMixtureDistr::getMixtureDistrTags(const char* name) {
  return MixtureDistr::getMixtureDistrTags(getName());
}
*/
Boolean NTTMixtureDistr::update(Array<double>* probs) { 
  constrain_adj_ = False;

  if(priors_ == NULL && intvals_->length() > 0) {
    priors_ = new double[numbins_];
    int k;
    for(k = 0; k < numbins_; k++) {
      priors_[k] = 0.0;
    }
    for(k = 0; k < intvals_->length(); k++) {
      priors_[(*intvals_)[k]] += 1.0;
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

      if(intvals_ != NULL) {
	posdistr_->addVal((*probs)[k], (*intvals_)[k]);
	negdistr_->addVal(1.0 - (*probs)[k], (*intvals_)[k]);
      }
      else {
	posdistr_->addVal((*probs)[k], (*doublevals_)[k]);
	negdistr_->addVal(1.0 - (*probs)[k],(*doublevals_)[k]);
      }

  }
  
  Boolean output = posdistr_->update();
  if(constrain_) {
    // constraint
    if(posdistr_->getProb(2) > posdistr_->getProb(1) && posdistr_->getProb(1) < posdistr_->getProb(0)) {
      output = False;
      if(newpos_ == NULL)
	newpos_ = new double[3];
      double tot = posdistr_->getProb(2) + 2.0 * posdistr_->getProb(1);
      if(tot > 0.0) {
	newpos_[2] = posdistr_->getProb(2) / tot;
	newpos_[1] = posdistr_->getProb(1) / tot;
	newpos_[0] = newpos_[1];
	//cerr << "(" << newpos_[0] << "," << newpos_[1] << "," << newpos_[2] << ") " << tot << endl;
      }
      for(int k = 0;  k < 3 ; k++)
	if(oldpos_ == NULL || myfabs(newpos_[k] - oldpos_[k]) >= maxdiff_)
	  output = True;
      if(output) {
	if(oldpos_ == NULL)
	  oldpos_ = new double[3];
	for(int k = 0; k < 3; k++)
	  oldpos_[k] = newpos_[k];
      }
      constrain_adj_ = True;
    } // fix problem constraint
    else if(output) {
      if(oldpos_ == NULL)
	oldpos_ = new double[3];
      if(newpos_ == NULL)
	newpos_ = new double[3];

      for(int k = 0;  k < 3 ; k++) {
	newpos_[k] = posdistr_->getProb(k);
	oldpos_[k] = newpos_[k];
      }
    }
  } // if constrain
  
  if(negdistr_->update()) {
    output = True;
  }
  return output;


}

double NTTMixtureDistr::getPosProb(int index) {
  if(! constrain_ || newpos_ == NULL) 
    return MixtureDistr::getPosProb(index);
  return newpos_[(*intvals_)[index]];
}

void NTTMixtureDistr::writeDistr(FILE* fout) {
  fprintf(fout, "%s\n", getName());
  fprintf(fout, "\tpos: ");
  fprintf(fout, "(");
  int k;
  for(k = 0; k < numbins_; k++) {
    if(constrain_ && newpos_ != NULL)
      fprintf(fout, "%0.3f %s", newpos_[k], (*bindefs_)[k]);
    else
      fprintf(fout, "%0.3f %s", posdistr_->getProb(k), (*bindefs_)[k]);
    if(k < numbins_ - 1) {
      fprintf(fout, ", ");
    }
  }
  if(constrain_ && constrain_adj_)
    fprintf(fout, ") *constrained\n");
  else
    fprintf(fout, ")\n");
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

