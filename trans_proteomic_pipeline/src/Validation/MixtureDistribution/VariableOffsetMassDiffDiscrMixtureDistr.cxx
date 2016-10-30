#include "VariableOffsetMassDiffDiscrMixtureDistr.h"

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


VariableOffsetMassDiffDiscrMixtureDistr::VariableOffsetMassDiffDiscrMixtureDistr(int charge, const char* name, const char* tag, double range, double window, double orig) : MassDifferenceDiscrMixtureDistr(charge, name, tag, range, window) {
  offset_init_ = orig;
  offset_ = offset_init_;
  vals_ = new Array<double>;
  offset_set_ = False;
  update_ctr_ = 0;
  min_ctr_ = 2; // min value for offset update
  accMass_ = False;
  if (range <= 0.5) 
    accMass_= True;
}

void VariableOffsetMassDiffDiscrMixtureDistr::enter(int index, const char* val) {
  assert(intvals_ != NULL);
  
  intvals_->insertAtEnd(inttranslate(val));
  vals_->insertAtEnd(atof(val)); // put the double value here
}


void VariableOffsetMassDiffDiscrMixtureDistr::enter(SearchResult* result) {
  assert(intvals_ != NULL);
  double massdiff = result->massdiff_;
  if (accMass_) { //DDS: get difference from nearest integer
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
  }
  intvals_->insertAtEnd(getIntegralValue(massdiff));
  vals_->insertAtEnd(massdiff); // put the double value here
}

int VariableOffsetMassDiffDiscrMixtureDistr::getIntegralValue(double val) {
  for(int k = 0; k < numbins_; k++)
    if(val <= (-1.0 * range_ + (k * window_) + offset_) + window_ / 2.0)
      return k;
  return numbins_ - 1;

}

Boolean VariableOffsetMassDiffDiscrMixtureDistr::update(Array<double>* probs) {
  Boolean output = False;
  if(! offset_set_ && update_ctr_ >= min_ctr_) {
    double new_offset;
    if (accMass_) {
      new_offset = getMode(range_/500, probs);
      maxdiff_ = 0.0001;
    }
    else
      new_offset = getMode(range_/10, probs);
    
    if(new_offset - offset_ > maxdiff_ || offset_ - new_offset > maxdiff_) { // update
      output = True;
      offset_ = new_offset;
      assert(vals_->length() == intvals_->length());
      for(int k = 0; k < vals_->length(); k++)
	intvals_->replace(k, getIntegralValue((*vals_)[k]));
    }
    else {
      offset_set_ = True; // done
    }
  } // if update offset
  if(! offset_set_)
    update_ctr_++;
  if(DiscreteMixtureDistr::update(probs) || output)
    return True;

  return False;

}

double VariableOffsetMassDiffDiscrMixtureDistr::getMode(double window, Array<double>* probs) {
  double min_tot = 5.0;
  double tmp = range_ / window;
  int num_windows = 2 * (int)tmp + 1;
  if(probs == NULL)
    return offset_init_;

  double* win = new double[num_windows];
  int k;
  for(k = 0; k < num_windows; k++)
    win[k] = 0.0;
  double max = 0.0;
  int max_ind = -1;
  double tot = 0.0;

  assert(probs->length() == vals_->length());


  for(k = 0; k < probs->length(); k++) {
    int next = (int)(((*vals_)[k] + range_) / window);
    //cerr << k << " out of " << probs->length() << ": " << next << " vs " << num_windows << endl;
    if(next < 0)
      next = 0;
    if(next >= num_windows)
      next = num_windows - 1;
    win[next] += (*probs)[k];
    tot += (*probs)[k];
  }

  // now find the max
  if(tot < min_tot) {
    delete[] win;
    return offset_init_;
  }

  for(k = 0; k < num_windows; k++)
    if(win[k] > max) {
      max = win[k];
      max_ind = k;
    }
  delete[] win;

  if(max_ind == -1)
    return offset_init_;
  double out =  max_ind * window;
  out -= range_;
  //cerr << update_ctr_ << ": " << max_ind << " " << max << endl;
  return out;
}


Array<Tag*>* VariableOffsetMassDiffDiscrMixtureDistr::getMixtureDistrTags(const char* name) {
  Array<Tag*>* output = new Array<Tag*>;
  Tag* next = new Tag("mixturemodel_distribution", True, False);
  char text[500];
  if(name == NULL)
    sprintf(text, "%s (offset: %f)", getName(), offset_);
  else
    sprintf(text, "%s", name);
  next->setAttributeValue("name", text);
  output->insertAtEnd(next);

  next = new Tag("posmodel_distribution", True, False);
  output->insertAtEnd(next);
  int k;
  for(k = 0; k < numbins_; k++)  {
      next = new Tag("parameter", True, True);
      next->setAttributeValue("name", (*bindefs_)[k]);
      sprintf(text, "%0.9f", posdistr_->getProb(k));
      next->setAttributeValue("value", text);
      output->insertAtEnd(next);
  }

  output->insertAtEnd(new Tag("posmodel_distribution", False, True));

  next = new Tag("negmodel_distribution", True, False);
  output->insertAtEnd(next);
  for(k = 0; k < numbins_; k++) {
      next = new Tag("parameter", True, True);
      next->setAttributeValue("name", (*bindefs_)[k]);
      sprintf(text, "%0.9f", negdistr_->getProb(k));
      next->setAttributeValue("value", text);
      output->insertAtEnd(next);
  }

  output->insertAtEnd(new Tag("negmodel_distribution", False, True));
  output->insertAtEnd(new Tag("mixturemodel_distribution", False, True));
  return output;

}

void VariableOffsetMassDiffDiscrMixtureDistr::writeDistr(FILE* fout) {
  
  fprintf(fout, "%s (offset: %f)\n", getName(), offset_);
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


char* VariableOffsetMassDiffDiscrMixtureDistr::getStringValue(int index) {
  char* output = new char[32];
  if(vals_ != NULL)
    sprintf(output, "%0.3f", (*vals_)[index]);
  else if(intvals_ != NULL)
    sprintf(output, "%0d", (*intvals_)[index]);
  return output;
}


double VariableOffsetMassDiffDiscrMixtureDistr::getPosProb(int index) {
  if (accMass_) {
    if(index < 0 || index >= intvals_->size()) {
      cerr << "violation of index " << index << " for " << intvals_->size() << endl;
      exit(1);
    }
    double pos = 0;
    int offset = 0;
    int count = 0;
    int mincount = 30;
    while (count < mincount) {
      if (((*intvals_)[index]+offset >= numbins_) || ((*intvals_)[index]-offset < 0)) {
	break;
      }
      if ((*intvals_)[index]+offset < numbins_) {
	count += posdistr_->getCount((*intvals_)[index]+offset);
	count += negdistr_->getCount((*intvals_)[index]+offset);
	pos += posdistr_->getProb((*intvals_)[index]+offset);
      }   
      if (offset > 0 && (*intvals_)[index]-offset >= 0) {
	count += posdistr_->getCount((*intvals_)[index]-offset);
	count += negdistr_->getCount((*intvals_)[index]-offset);
	pos += posdistr_->getProb((*intvals_)[index]-offset);
      }
      offset++ ;
    }
    
    return pos;

  }
  return MixtureDistr::getPosProb(index);
  
}

double VariableOffsetMassDiffDiscrMixtureDistr::getNegProb(int index) {
  if (accMass_) {
    if(index < 0 || index >= intvals_->size()) {
      cerr << "violation of index " << index << " for " << intvals_->size() << endl;
      exit(1);
    }
    double neg = 0;
    int offset = 0;
    int count = 0;
    int mincount = 30;
    while (count < mincount) {
      if (((*intvals_)[index]+offset >= numbins_) || ((*intvals_)[index]-offset < 0)) {
	break;
      }
      if ((*intvals_)[index]+offset < numbins_) {
	count += posdistr_->getCount((*intvals_)[index]+offset);
	count += negdistr_->getCount((*intvals_)[index]+offset);
	neg += negdistr_->getProb((*intvals_)[index]+offset);
      }   
      if (offset > 0 && (*intvals_)[index]-offset >= 0) {
	count += posdistr_->getCount((*intvals_)[index]-offset);
	count += negdistr_->getCount((*intvals_)[index]-offset);
	neg += negdistr_->getProb((*intvals_)[index]-offset);
      }
      offset++ ;
    }
    
    return neg;
  }
  return MixtureDistr::getNegProb(index);
}
