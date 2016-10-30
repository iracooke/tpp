
/*

Program       : MixtureDistr for PeptideProphet                                                       
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

#define NOMINMAX // keeps MSVC from defining max(a,b), which horks std::numeric_limits<double>::max()
#include "MixtureDistr.h"
#include <limits> // for std::numeric_limits decl

MixtureDistr::MixtureDistr()
: posdistr_(0), 
  negdistr_(0), 
  intvals_(0), 
  doublevals_(0), 
  pospriors_(0), 
  negpriors_(0),
  posDistReset_(false),
  negDistReset_(false) {
}

MixtureDistr::MixtureDistr(int charge, const char* name, const char* tag)
: posdistr_(0), 
  negdistr_(0), 
  intvals_(0), 
  doublevals_(0), 
  pospriors_(0), 
  negpriors_(0),
  posDistReset_(false),
  negDistReset_(false) {
  initializeDistr(charge, name, tag);
}

MixtureDistr::~MixtureDistr() {
  if (posdistr_ != 0 && posDistReset_ == false)
    delete posdistr_;
  if (negdistr_ != 0 && negDistReset_ == false)
    delete negdistr_;
  if (intvals_ != 0)
    delete intvals_;
  if (doublevals_ != 0)
    delete doublevals_;
  if (pospriors_ != 0)
    delete [] pospriors_;
  if (negpriors_ != 0)
    delete [] negpriors_;


  posdistr_ = NULL;
  negdistr_ = NULL;
  intvals_ = NULL;
  doublevals_ = NULL;
  pospriors_ = NULL;
  negpriors_ = NULL;
 
}

void MixtureDistr::initializeDistr(int charge, const char* name, const char* tag) {
  charge_ = charge;
  intvals_ = NULL;
  doublevals_ = NULL;
  if(name == NULL) {
    cerr << "null name for MixtureDistr" << endl;
    exit(1);
  }
  if(tag == NULL) {
    cerr << "null tag for MixtureDistr" << endl;
    exit(1);
  }

  name_ = name;

  // now find abbrev_name_ in between []
  int start = 0;
  int stop = 0;
  for(int k = 0; name_[k]; k++)
    if(name_[k] == '[')
      start = k;
    else if(name_[k] == ']')
      stop = k;
  if(stop > start + 1) {
	  abbrev_name_ = name_.substr(start + 1, stop - start - 1);
  }

  tag_ = tag;
  pospriors_ = NULL;
  negpriors_ = NULL;
  negOnly_ = False;
  minval_ = std::numeric_limits<double>::max();;
  maxval_ = std::numeric_limits<double>::min();;
}

int MixtureDistr::getNumVals() { 
  if(intvals_ != NULL) {
    return intvals_->size();
  }
  return doublevals_->size();
}

void MixtureDistr::enter(int index, const char* val) {
  
  if(intvals_ != NULL) {
    enter(index, inttranslate(val));
  }
  else {
    enter(index, doubletranslate(val));
  }
  if (minval_ > doubletranslate(val)) {
    minval_ = doubletranslate(val);
  }
  if (maxval_ < doubletranslate(val)) {
    maxval_ = doubletranslate(val);
  }
}


Boolean MixtureDistr::isValue(int index, int val) {
  if(intvals_ == NULL) {
    return False;
  }
  return (*intvals_)[index] == val;
}




int MixtureDistr::inttranslate(const char* val) {
  return atoi(val);
}

double MixtureDistr::doubletranslate(const char* val) {
  return atof(val);
}

double MixtureDistr::getPosProb(int index) {
  if(intvals_ != NULL) {
    if(index < 0 || index >= intvals_->size()) {
      cerr << "violation of index " << index << " for " << intvals_->size() << endl;
      exit(1);
    }

    return posdistr_->getProb((*intvals_)[index]);
  }
  else {
    return posdistr_->getProb((*doublevals_)[index]);
  }
}


double MixtureDistr::getNegProb(int index) {
  if(intvals_ != NULL) {

    return negdistr_->getProb((*intvals_)[index]);
  }
  else {
    return negdistr_->getProb((*doublevals_)[index]);
  }
}

Boolean MixtureDistr::negOnly() {
  return negOnly_;
}

Distribution* MixtureDistr::getPosDistr() {
  return posdistr_;
}

Distribution* MixtureDistr::getNegDistr() {
  return negdistr_;
}

void MixtureDistr::setPosDistr(MixtureDistr* distr) {
  if (this == distr) return; //DDS: Getout right away
  Distribution* reject = distr->getPosDistr();
  if(posdistr_ != NULL)
    delete posdistr_;

  posdistr_ = reject;
  posDistReset_ = true;

}

void MixtureDistr::setNegDistr(MixtureDistr* distr) {
  if (this == distr) return; //DDS: Getout right away
  Distribution* reject = distr->getNegDistr();
  if(negdistr_ != NULL)
    delete negdistr_;

  negdistr_ = reject;
  negDistReset_ = true;
}

	
void MixtureDistr::enter(int index, double val) {
  doublevals_->insertAtEnd(val);
}


void MixtureDistr::enter(int index, int val) {
  intvals_->insertAtEnd(val);
}

void MixtureDistr::write_pIstats(ostream& out) {
  cerr << "ERROR: MixtureDistr::write_pIstats should not be called, please contact the developer for assistance." << endl;
}

void MixtureDistr::write_RTstats(ostream& out) {
  cerr << "ERROR: MixtureDistr::write_RTstats should not be called, please contact the developer for assistance." << endl;
}

void MixtureDistr::write_RTcoeff(ostream& out) {
  cerr << "ERROR: MixtureDistr::write_RTcoeff should not be called, please contact the developer for assistance." << endl;
}

Boolean MixtureDistr::update(Array<Array<double>*>* all_probs, int charge) {
   Array<double>* probs = new Array<double>(*(*all_probs)[charge]);
   Boolean rtn = this->update(probs);
   delete probs;
   return rtn;
}

Boolean MixtureDistr::update(Array<Array<double>*>* probs, double min_prob) {
  cerr << "ERROR: MixtureDistr::update[1] should not be called, please contact the developer for assistance." << endl;
  exit(1);
}

Boolean MixtureDistr::update(Array<Array<double>*>* probs, double min_prob, Array<Array<int>*>* ntts, int min_ntt) {
  cerr << "ERROR: MixtureDistr::update[2] should not be called, please contact the developer for assistance." << endl;
  exit(1);
}

Boolean MixtureDistr::update(Array<Array<double>*>* probs, double min_prob, Array<Array<int>*>* ntts, int min_ntt, int& code) {
  cerr << "ERROR: MixtureDistr::update[3] should not be called, please contact the developer for assistance." << endl;
  exit(1);
}

Boolean MixtureDistr::update(Array<Array<double>*>* probs, double min_prob, Array<Array<int>*>* ntts, int min_ntt, int& code, const char* c) {
  cerr << "ERROR: MixtureDistr::update[3] should not be called, please contact the developer for assistance." << endl;
  exit(1);
}

Boolean MixtureDistr::update(Array<double>* probs, double min_prob, Array<int>* ntts, int min_ntt, int& code) {
  cerr << "ERROR: MixtureDistr::update[3] should not be called, please contact the developer for assistance." << endl;
  exit(1);
}

Boolean MixtureDistr::update(Array<Array<double>*>* probs) {
  cerr << "ERROR: MixtureDistr::update[4] should not be called, please contact the developer for assistance." << endl;
  exit(1);
}

Boolean MixtureDistr::update(Array<Array<double>*>* probs, const char* c) {
  cerr << "ERROR: MixtureDistr::update[4] should not be called, please contact the developer for assistance." << endl;
  exit(1);
}

Boolean MixtureDistr::update(Array<double>* probs, Array<Boolean>* isdecoy, Boolean final) {
  posdistr_->initUpdate(NULL);
  negdistr_->initUpdate(NULL);
  
  double pos_wt = 0; 
  double count = 0; 
  double neg_wt = 0; 

  for(int k = 0; k < probs->size(); k++) {
    if ((*probs)[k] >= 0) {

      double prob = 0;

      if (!(*isdecoy)[k]) {
	prob = (*probs)[k];
      }

      if(intvals_ != NULL) {
	posdistr_->addVal(prob, (*intvals_)[k]);
	pos_wt += prob;
	negdistr_->addVal(1.0 - prob, (*intvals_)[k]);
	neg_wt += 1.0 - prob;
      }
      else {
	if (!(*isdecoy)[k]) { 
	  posdistr_->addVal(prob, (*doublevals_)[k]);
	  pos_wt += prob;
	  count++;
	}
	negdistr_->addVal(1.0 - prob,(*doublevals_)[k]);
	neg_wt += 1.0 - prob;
      }
    }
  }
  
  Boolean output = False;
  double pi = 1; //pos_wt/count;

  if (final) {
    posdistr_->update(pi);
    output = False;
  }
  else if(posdistr_->update(pi)) {
    output = True;
  }

  if (final) {
    negdistr_->update(pi);
    output = False;
  }
  else if(negdistr_->update(pi)) {
    output = True;
  }
  return output;
}

Boolean MixtureDistr::update(Array<double>* probs) {

  posdistr_->initUpdate(NULL);
  negdistr_->initUpdate(NULL);

  for(int k = 0; k < probs->size(); k++) {
    if ((*probs)[k] >= 0) {
      if(intvals_ != NULL) {
	posdistr_->addVal((*probs)[k], (*intvals_)[k]);
	negdistr_->addVal(1.0 - (*probs)[k], (*intvals_)[k]);
      }
      else {
	posdistr_->addVal((*probs)[k], (*doublevals_)[k]);
	negdistr_->addVal(1.0 - (*probs)[k],(*doublevals_)[k]);
      }
    }
  }
  
  Boolean output = False;
  
  if(posdistr_->update()) {
    output = True;
  }
  if(negdistr_->update()) {
    output = True;
  }
  return output;
}

const char* MixtureDistr::getName() { return name_.c_str(); }

const char* MixtureDistr::getAbbrevName() { 
	if(!abbrev_name_.length())
		return getName();
	return abbrev_name_.c_str(); 
}

const char* MixtureDistr::getTag() { return tag_.c_str(); }

void MixtureDistr::printVal(int index) {
  
  cout << name_ << "=";
  if(intvals_ != NULL) {
    cout << (*intvals_)[index];
  }
  else {
    cout << (*doublevals_)[index];
  }
  
}

char* MixtureDistr::getStringValue(int index) {
  char* output = new char[32]; 
  if(intvals_ != NULL)
    sprintf(output, "%d", (*intvals_)[index]);
  else
    sprintf(output, "%0.4f", (*doublevals_)[index]);
  return output;
}


void MixtureDistr::printDistr() {
  cout << name_ << endl;
  cout << "\tpos: ";
  posdistr_->printDistr();
  cout << "\tneg: ";
  negdistr_->printDistr();
}

void MixtureDistr::writeDistr(FILE* fout) {
  fprintf(fout, "%s\n", getName());
  fprintf(fout, "\tpos: ");
  posdistr_->writeDistr(fout);
  fprintf(fout, "\tneg: ");
  negdistr_->writeDistr(fout);
}

Array<Tag*>* MixtureDistr::getMixtureDistrTags(const char* name) {
  Array<Tag*>* output = new Array<Tag*>;
  Tag* next = new Tag("mixturemodel_distribution", True, False);
  if(name == NULL)
    next->setAttributeValue("name", getName());
  else
    next->setAttributeValue("name", name);
  //cout << "next: "; next->write(cout);
  output->insertAtEnd(next);

  Array<Tag*>* nexttags = posdistr_->getSummaryTags(True);
  if(nexttags != NULL) {
    for(int k = 0; k < nexttags->size(); k++)
      output->insertAtEnd((*nexttags)[k]);
    delete nexttags;
  }

  nexttags = negdistr_->getSummaryTags(False);
  if(nexttags != NULL) {
    for(int k = 0; k < nexttags->size(); k++)
      output->insertAtEnd((*nexttags)[k]);
    delete nexttags;
  }

  //output->insertAtEnd(posdistr_->getSummaryTags());
  //output->insertAtEnd(negdistr_->getSummaryTag());
  output->insertAtEnd(new Tag("mixturemodel_distribution", False, True));

  return output;
}
