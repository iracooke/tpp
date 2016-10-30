#include "MzDifferenceDiscrMixtureDistr.h"
#include "Parsers/Algorithm2XML/SearchResult/SpectraSTResult.h"
/*

Program       : MzDifferenceDiscrMixtureDistr for PeptideProphet                                                       
Author        : Henry Lam <hlam@systemsbiology.org>                                                       
Date          : 04.10.06 

Copyright (C) 2006 Henry Lam

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

Henry Lam
Institute for Systems Biology
401 Terry Avenue North 
Seattle, WA  98109  USA
hlam@systemsbiology.org

*/

MzDifferenceDiscrMixtureDistr::MzDifferenceDiscrMixtureDistr(int charge, const char* name, const char* tag, double range, double window) : DiscreteMixtureDistr(charge, ((int)(range/window)) + 1, name, tag) {

  bindefs_ = new Array<const char*>;
  range_ = range;
  window_ = window;

  double bin;
  int num = 0;
  char* next;
  for(int k = 0; k < numbins_; k++) {
    bin = k * window_ + window_;
    next = new char[18];
    sprintf(next, "%0.1f<=mzd<%0.1f", bin - window_, bin); // , window_/2.0);
    next[17] = 0;
    bindefs_->insertAtEnd(next);
  }

  maxdiff_ = 0.001;
  negOnly_ = True;
  DiscreteMixtureDistr::init(NULL);

}

void MzDifferenceDiscrMixtureDistr::enter(SearchResult* result) {
  intvals_->insertAtEnd(getIntegralValue(((SpectraSTResult*)(result))->mzdiff_));

 // cout << "here with mzdiff: " << ((SpectraSTResult*)(result))->mzdiff_ << endl;
 // MixtureDistr::enter(0, ((SpectraSTResult*)(result))->mzdiff_);
 // cout << "done here" << endl;
}

int MzDifferenceDiscrMixtureDistr::getIntegralValue(double val) {
  for(int k = 0; k < numbins_; k++)
    if(val <= (k * window_ + window_))
      return k;
  return numbins_ - 1;

}

int MzDifferenceDiscrMixtureDistr::inttranslate(const char* val) {
  double value = atof(val);
  for(int k = 0; k < numbins_; k++)
    if(value <= (k * window_ + window_ ))
      return k;
  return numbins_ - 1;
}

Boolean MzDifferenceDiscrMixtureDistr::haveDataWithValue(int bin) {
  for(int k = 0; k < intvals_->length(); k++)
    if((*intvals_)[k] == bin)
      return True;
  return False;
}

void MzDifferenceDiscrMixtureDistr::writeDistr(FILE* fout) {
  
  fprintf(fout, "%s\n", getName());
  fprintf(fout, "\tpos: ");
  fprintf(fout, "(");
  int counter = 0;
  int k, column_width = 4;
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


