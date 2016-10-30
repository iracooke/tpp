#include "DiscreteMixtureDistr.h"

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


DiscreteMixtureDistr::DiscreteMixtureDistr(int charge, int numbins, const char* name, const char* tag) : MixtureDistr(charge, name, tag) {
  numbins_ = numbins;
  //cout << "...with " << numbins_ << " bins" << endl;
  maxdiff_ = 0.002;
  bindefs_ = NULL;
  priors_ = NULL;
  bindefs_ = NULL;
  numpos_priors_ = 2.0;
  numneg_priors_ = 2.0;
}

Boolean DiscreteMixtureDistr::update(Array<double>*probs) {
  if(priors_ == NULL && intvals_->size() > 0) {
    priors_ = new double[numbins_];
    int k;
    for(k = 0; k < numbins_; k++) {
      priors_[k] = 0.0;
    }
    for(k = 0; k < intvals_->size(); k++) {
      priors_[(*intvals_)[k]] += 1.0;
    }
    for(k = 0; k < numbins_; k++) {
      priors_[k] /= intvals_->size();
    }
    posdistr_->setPriors(priors_, numpos_priors_);
    negdistr_->setPriors(priors_, numneg_priors_);
  } // if no priors yet
  return MixtureDistr::update(probs);
}


void DiscreteMixtureDistr::initializeBinDefs(const char** bindefs) {
  bindefs_ = new Array<const char*>;
  for(int k = 0; k < numbins_; k++) {
    bindefs_->insertAtEnd(bindefs[k]);
  }
}

void DiscreteMixtureDistr::init(const char** bindefs) {
  if(bindefs != NULL)
    initializeBinDefs(bindefs);
  if(intvals_ == NULL)
    intvals_ = new Array<int>;
  posdistr_ = new DiscreteDistribution(numbins_, maxdiff_);
  negdistr_ = new DiscreteDistribution(numbins_, maxdiff_);
}


void DiscreteMixtureDistr::printDistr() {
  cout << name_ << endl;
  cout << "\tpos: ";
  printPosDistribution();
  cout << "\tneg: ";
  printNegDistribution();
}

void DiscreteMixtureDistr::writeDistr(FILE* fout) {
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

Array<Tag*>* DiscreteMixtureDistr::getMixtureDistrTags(const char* name) {
  Array<Tag*>* output = new Array<Tag*>;
  Tag* next = new Tag("mixturemodel_distribution", True, False);
  if(name == NULL)
    next->setAttributeValue("name", getName());
  else
    next->setAttributeValue("name", name);
  //cout << "next: "; next->write(cout);
  output->insertAtEnd(next);

  char text[500];
  next = new Tag("posmodel_distribution", True, False);
  output->insertAtEnd(next);
  int k;
  for(k = 0; k < numbins_; k++) {
    next = new Tag("parameter", True, True);
    // substitute special characters here....
    if(strstr((*bindefs_)[k], ">") != NULL || strstr((*bindefs_)[k], "<") != NULL) {
      char gt[] = "&gt;";
      char lt[] = "&lt;";
      int tot = 0;
      int j;
      for(j = 0; (*bindefs_)[k][j]; j++)
	if(((*bindefs_)[k])[j] == '<' || ((*bindefs_)[k])[j] == '>')
	  tot++;
      char* mod = new char[strlen((*bindefs_)[k])+3*tot+1];
      tot = 0;
      for(j = 0; (*bindefs_)[k][j]; j++)
	if(((*bindefs_)[k])[j] == '<')
	  for(int i = 0; lt[i]; i++)
	    mod[tot++] = lt[i];
	else if(((*bindefs_)[k])[j] == '>')
	  for(int i = 0; gt[i]; i++)
	    mod[tot++] = gt[i];
	else
	  mod[tot++] = ((*bindefs_)[k])[j];
      mod[tot] = 0;
      next->setAttributeValue("name", mod);
      delete [] mod;
    }
    else 
      next->setAttributeValue("name", (*bindefs_)[k]);

    sprintf(text, "%0.3f", posdistr_->getProb(k));
    next->setAttributeValue("value", text);
    output->insertAtEnd(next);
  }
  output->insertAtEnd(new Tag("posmodel_distribution", False, True));

  next = new Tag("negmodel_distribution", True, False);
  output->insertAtEnd(next);
  for(k = 0; k < numbins_; k++) {
    next = new Tag("parameter", True, True);

    if(strstr((*bindefs_)[k], ">") != NULL || strstr((*bindefs_)[k], "<") != NULL) {
      char gt[] = "&gt;";
      char lt[] = "&lt;";
      int tot = 0;
      int j;
      for(j = 0; (*bindefs_)[k][j]; j++)
	if(((*bindefs_)[k])[j] == '<' || ((*bindefs_)[k])[j] == '>')
	  tot++;
      char* mod = new char[strlen((*bindefs_)[k])+3*tot+1];
      tot = 0;
      for(j = 0; (*bindefs_)[k][j]; j++)
	if(((*bindefs_)[k])[j] == '<')
	  for(int i = 0; lt[i]; i++)
	    mod[tot++] = lt[i];
	else if(((*bindefs_)[k])[j] == '>')
	  for(int i = 0; gt[i]; i++)
	    mod[tot++] = gt[i];
	else
	  mod[tot++] = ((*bindefs_)[k])[j];
      mod[tot] = 0;
      next->setAttributeValue("name", mod);
      delete [] mod;
    }
    else 
      next->setAttributeValue("name", (*bindefs_)[k]);


    //next->setAttributeValue("name", (*bindefs_)[k]);
    sprintf(text, "%0.3f", negdistr_->getProb(k));
    next->setAttributeValue("value", text);
    output->insertAtEnd(next);
  }
  output->insertAtEnd(new Tag("negmodel_distribution", False, True));
  output->insertAtEnd(new Tag("mixturemodel_distribution", False, True));
  return output;

}

void DiscreteMixtureDistr::printPosDistribution() {
  printf("(");
  for(int k = 0; k < numbins_; k++) {
    printf("%0.3f %s", posdistr_->getProb(k), (*bindefs_)[k]);
    if(k < numbins_ - 1) {
      printf(", ");
    }
  }
  printf(")\n");
}

void DiscreteMixtureDistr::printNegDistribution() {
  printf("(");
  for(int k = 0; k < numbins_; k++) {
    printf("%0.3f %s", negdistr_->getProb(k), (*bindefs_)[k]);
    if(k < numbins_ - 1) {
      printf(", ");
    }
  }
  printf(")\n");
}


