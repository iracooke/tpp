#include "VariableOffsetpIMixtureDistr.h"

/*

Program       : VariableOffsetMassDiffDiscrMixtureDistr for PeptideProphet                                                       
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


VariableOffsetpIMixtureDistr::VariableOffsetpIMixtureDistr(int charge, const char* name, const char* tag, double range, double window, double orig) : pIMixtureDistr(charge, name, tag) {
  offset_init_ = orig;
  offset_ = offset_init_;
  vals_ = new Array<double>;
  pIvals_ = new Array<double>;
  offset_set_ = False;
  update_ctr_ = 0;
  min_ctr_ = 2; // min value for offset update
  range_ = 28.0;
  window_ = 1.0;
}

Boolean VariableOffsetpIMixtureDistr::haveDataWithValue(int bin) {
for(int k = 0; k < intvals_->size(); k++)
    if((*intvals_)[k] == bin)
      return True;
  return False;
}

void VariableOffsetpIMixtureDistr::write_pIstats(ostream& out) {
  for (int i = 0; i < run_pI_calc_->size(); i++) {
    (*run_pI_calc_)[i]->write_pIstats(out);
  }
}


void VariableOffsetpIMixtureDistr::recalc_pIstats(Array<Array<double>*>* probs) {
  char val[320];
  intvals_->reserve(intvals_->size());
  vals_->reserve(vals_->size());
  pIvals_->reserve(pIvals_->size());
  for (int i = 0; i < run_pI_calc_->size(); i++) {
    (*run_pI_calc_)[i]->recalc_pIstats((*probs)[i]);
    for (int j=0; j < (*run_pI_calc_)[i]->pIs_.size(); j++) {
      double dval = (*run_pI_calc_)[i]->getpIScore((*run_pI_calc_)[i]->pIs_[j], (char*)(*(*run_pI_calc_)[i]->peps_)[j]);
      snprintf(val, sizeof(val), "%f", dval);
      pIvals_->insertAtEnd((*run_pI_calc_)[i]->pIs_[j]);
      this->enter(0, val);
    }
  }

}

void VariableOffsetpIMixtureDistr::recalc_pIstats(Array<Array<double>*>* probs, double min_prob, Array<Array<int>*>* ntts, int  min_ntt) {
  char val[320];
  intvals_->reserve(intvals_->size());  // resets container count to 0, but leaves allocation alone for efficient repopulation
  vals_->reserve(vals_->size());  // resets container count to 0, but leaves allocation alone for efficient repopulation
  pIvals_->reserve(pIvals_->size());  // resets container count to 0, but leaves allocation alone for efficient repopulation
  for (int i = 0; i < run_pI_calc_->size(); i++) {
    if ((*run_pI_calc_)[i]->recalc_pIstats((*probs)[i], min_prob, (*ntts)[i], min_ntt)) {
      for (int j=0; j < (*run_pI_calc_)[i]->pIs_.size(); j++) {
	double dval = (*run_pI_calc_)[i]->getpIScore((*run_pI_calc_)[i]->pIs_[j], (char*)(*(*run_pI_calc_)[i]->peps_)[j]);
	snprintf(val, sizeof(val), "%f", dval);
	pIvals_->insertAtEnd((*run_pI_calc_)[i]->pIs_[j]);
	this->enter(0, val);
      }
    }
    else {
      for (int j=0; j < (*run_pI_calc_)[i]->pIs_.size(); j++) {
	double dval = 15;
	snprintf(val, sizeof(val), "%f", dval);
	this->enter(0, val);
      }
    }
  }

}

void VariableOffsetpIMixtureDistr::calc_pIstats() {
  char val[320];
  intvals_->reserve(intvals_->size());
  vals_->reserve(vals_->size());
  pIvals_->reserve(pIvals_->size());
  for (int i = 0; i < run_pI_calc_->size(); i++) {
    (*run_pI_calc_)[i]->calc_pIstats();
    for (int j=0; j < (*run_pI_calc_)[i]->pIs_.size(); j++) {
      double dval = (*run_pI_calc_)[i]->getpIScore((*run_pI_calc_)[i]->pIs_[j], (char*)(*(*run_pI_calc_)[i]->peps_)[j]);
      snprintf(val, sizeof(val), "%f", dval);
      pIvals_->insertAtEnd((*run_pI_calc_)[i]->pIs_[j]);
      this->enter(0, val);
    }
  }

}

void VariableOffsetpIMixtureDistr::enter(int index, const char* val) {
  assert(intvals_ != NULL);

  //  intvals_->insertAtEnd(inttranslate(val));
  intvals_->insertAtEnd(getpIBinNo(atof(val)));
  vals_->insertAtEnd(atof(val)); // put the double value here
}

/* Using Parent class
void VariableOffsetpIMixtureDistr::enter(SearchResult* result) {
  assert(intvals_ != NULL);
  if(result->pI_ > 0.0) { // already have it
    intvals_->insertAtEnd(getIntegralValue(result->pI_));
    vals_->insertAtEnd(result->pI_);
  }
  else {
#ifdef USE_STD_MODS
    double calc_pi = pI_calc_->Peptide_pI(result->peptide_, result->mod_info_);
    intvals_->insertAtEnd(getIntegralValue(calc_pi));
    vals_->insertAtEnd(calc_pi);
#endif
  }
}
*/

int VariableOffsetpIMixtureDistr::getIntegralValue(double val) {
  for(int k = 0; k < numbins_; k++)
    if(val <= 1.0 + (k * window_) + offset_ + window_ / 2.0)
      return k;
  return numbins_ - 1;

}

Boolean VariableOffsetpIMixtureDistr::update(Array<Array<double>*>* all_probs) {
  Boolean output = False;

  Array<double>* probs = new Array<double>;
  
  
  for (int i=0; i<all_probs->size(); i++) {
    int len = (*all_probs)[i]->size();
    for (int j=0; j<len; j++) {
      probs->insertAtEnd((*(*all_probs)[i])[j]);
    }
  }

  recalc_pIstats(all_probs);

  if(! offset_set_ && update_ctr_ >= min_ctr_) {
    //cerr << "here1" << endl;
    double new_offset = getMode(0.1, probs);
    // want just the non-integral part
    //cout << "max: " << new_offset << endl;
    new_offset = new_offset - (int)new_offset; // just the fraction

    if(new_offset - offset_ > maxdiff_ || offset_ - new_offset > maxdiff_) { // update
      output = True;
      offset_ = new_offset;
      assert(vals_->size() == intvals_->size());
      for(int k = 0; k < vals_->size(); k++) {
	intvals_->replace(k, getpIBinNo((*vals_)[k]));
	  //intvals_->replace(k, getIntegralValue((*vals_)[k]));
      }
    }
    else {
      offset_set_ = True; // done
    }
  } // if update offset
  //cerr << "here2" << endl;
  Boolean rtn = False;
  if(! offset_set_)
    update_ctr_++;
  if(DiscreteMixtureDistr::update(probs) || output) {
    rtn = True;
  }
  delete probs;
  return rtn;

}
Boolean VariableOffsetpIMixtureDistr::update(Array<Array<double>*>* all_probs, double min_prob, Array<Array<int>*>* all_ntts, int min_ntt) {
  Boolean output = False;

  Array<double>* probs = new Array<double>;
  
  
  for (int i=0; i<all_probs->size(); i++) {
    int len = (*all_probs)[i]->size();
    for (int j=0; j<len; j++) {
      probs->insertAtEnd((*(*all_probs)[i])[j]);
    }
  }

  recalc_pIstats(all_probs, min_prob, all_ntts, min_ntt);

  if(! offset_set_  && update_ctr_ >= min_ctr_) {
    //cerr << "here1" << endl;
    double new_offset = getMode(0.1, probs);
    // want just the non-integral part
    //cout << "max: " << new_offset << endl;
    new_offset = new_offset - (int)new_offset; // just the fraction

    if(new_offset - offset_ > maxdiff_ || offset_ - new_offset > maxdiff_) { // update
      output = True;
      offset_ = new_offset;
      assert(vals_->size() == intvals_->size());
      for(int k = 0; k < vals_->size(); k++) {
	int val = getpIBinNo((*vals_)[k]);
	intvals_->replace(k, val);
	  //intvals_->replace(k, getIntegralValue((*vals_)[k]));
      }
    }
    else {
      offset_set_ = True; // done
    }
  } 
  
  Boolean rtn = False;
  if(! offset_set_)
    update_ctr_++;
  
  if(DiscreteMixtureDistr::update(probs) || output) {
    rtn = True;
  }
  delete probs;
  return rtn;

}

double VariableOffsetpIMixtureDistr::getMode(double window, Array<double>* probs) {
  double min_tot = 5.0;
  int num_windows = (int)(range_ / window);
  if(probs == NULL)
    return offset_init_;

  Array<double>* win = new Array<double>();
  int k;
  for(k = 0; k < num_windows; k++)
    win->insertAtEnd(0.0);
  double max = 0.0;
  int max_ind = -1;
  double tot = 0.0;

  assert(probs->size() == vals_->size());


  for(k = 0; k < probs->size(); k++) {
    if ((*probs)[k] >= 0) {
      int next = (int)(((*vals_)[k]) / window);
      //cerr << k << " out of " << probs->size() << ": " << next << " vs " << num_windows << endl;
      if(next < 0)
	next = 0;
      if(next >= num_windows)
	next = num_windows - 1;
      (*win)[next] += (*probs)[k];
      tot += (*probs)[k];
    }
  }

  // now find the max
  if(tot < min_tot) {
    delete win;
    return offset_init_;
  }

  for(k = 0; k < num_windows; k++)
    if((*win)[k] > max) {
      max = (*win)[k];
      max_ind = k;
    }
  delete win;

  if(max_ind == -1)
    return offset_init_;
  //cerr << update_ctr_ << ": " << max_ind << " " << max << endl;
  return ((double)max_ind * window);
}


Array<Tag*>* VariableOffsetpIMixtureDistr::getMixtureDistrTags(const char* name) {
  Array<Tag*>* output = new Array<Tag*>;
  Tag* next = new Tag("mixturemodel_distribution", True, False);
  char text[500];
  if(name == NULL)
    snprintf(text, sizeof(text), "%s (offset: %0.2f)", getName(), offset_);
  else
    snprintf(text, sizeof(text), "%s", name);
  next->setAttributeValue("name", text);
  output->insertAtEnd(next);

  next = new Tag("posmodel_distribution", True, False);
  output->insertAtEnd(next);
  int k;
  for(k = 0; k < numbins_; k++) 
    if(haveDataWithValue(k)) {
      next = new Tag("parameter", True, True);
      next->setAttributeValue("name", (*bindefs_)[k]);
      snprintf(text, sizeof(text), "%0.2f", posdistr_->getProb(k));
      next->setAttributeValue("value", text);
      output->insertAtEnd(next);
    }
  output->insertAtEnd(new Tag("posmodel_distribution", False, True));

  next = new Tag("negmodel_distribution", True, False);
  output->insertAtEnd(next);
  for(k = 0; k < numbins_; k++) 
    if(haveDataWithValue(k)) {
      next = new Tag("parameter", True, True);
      next->setAttributeValue("name", (*bindefs_)[k]);
      snprintf(text, sizeof(text), "%0.2f", negdistr_->getProb(k));
      next->setAttributeValue("value", text);
      output->insertAtEnd(next);
    }
  output->insertAtEnd(new Tag("negmodel_distribution", False, True));
  output->insertAtEnd(new Tag("mixturemodel_distribution", False, True));
  return output;

}

void VariableOffsetpIMixtureDistr::writeDistr(FILE* fout) {
  
  fprintf(fout, "%s (offset: %0.2f)\n", getName(), offset_);
  fprintf(fout, "\tpos: ");
  fprintf(fout, "(");
//  int next;
  int counter = 0;
  int k,column_width = 4;
  for(k = 0; k < numbins_; k++) {
    if(haveDataWithValue(k)) {
      counter++;
      fprintf(fout, "%0.2f %s", posdistr_->getProb(k), (*bindefs_)[k]);
      if(k < numbins_ - 1 && haveDataWithValue(k+1)) {
	fprintf(fout, ", ");
      }
      if(counter%column_width == 0)
	fprintf(fout, "\n\t\t");
    } // if have value
  }
  fprintf(fout, ")\n");
  
  fprintf(fout, "\tneg: ");
  fprintf(fout, "(");
  counter = 0;
  for(k = 0; k < numbins_; k++) {
    if(haveDataWithValue(k)) {
      counter++;
      fprintf(fout, "%0.2f %s", negdistr_->getProb(k), (*bindefs_)[k]);
      if(k < numbins_ - 1 && haveDataWithValue(k+1)) {
	fprintf(fout, ", ");
      }
      if(counter%column_width == 0)
	fprintf(fout, "\n\t\t");
    } // if have value
  }
  
  fprintf(fout, ")\n");
}

#define RESLEN 32
char* VariableOffsetpIMixtureDistr::getStringValue(int index) {
  char* output = new char[RESLEN+1];
   if(vals_ != NULL)
      snprintf(output, RESLEN, "%0.2f", (*vals_)[index]);
    else 
    if(intvals_ != NULL)
      snprintf(output, RESLEN, "%0d", (*intvals_)[index]);
  return output;
}

char* VariableOffsetpIMixtureDistr::getStringpIValue(int index) {
  char* output = new char[RESLEN+1];
   if(pIvals_ != NULL)
      snprintf(output, RESLEN, "%0.2f", (*pIvals_)[index]);
  return output;
}
