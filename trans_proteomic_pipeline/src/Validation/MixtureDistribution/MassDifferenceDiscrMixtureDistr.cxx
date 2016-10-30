#include "MassDifferenceDiscrMixtureDistr.h"

/*

Program       : MassDifferenceDiscrMixtureDistr for PeptideProphet                                                       
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

MassDifferenceDiscrMixtureDistr::MassDifferenceDiscrMixtureDistr(int charge, const char* name, const char* tag, double range, double window, Boolean ppm) : DiscreteMixtureDistr(charge, (int)((2 * range/window) + 1.1), name, tag) {

  bindefs_ = new Array<const char*>;
  range_ = range;
  window_ = window;
  ppm_ = ppm;

  double bin;
  int num = 0;
  char* next;
  for(int k = 0; k < numbins_; k++) {
    bin = -1.0 * range_ + (k * window_);
    if(bin < 0.0)
      num++;
    next = new char[num + 100];
    sprintf(next, "massd=%f", bin); // , window_/2.0);
    //next[num+9] = 0;
    bindefs_->insertAtEnd(next);
  }

  maxdiff_ = 0.001;
  negOnly_ = True;
  DiscreteMixtureDistr::init(NULL);

}

void MassDifferenceDiscrMixtureDistr::enter(SearchResult* result) {
  
  double massdiff = result->massdiff_;

  // if (ppm_)  //ONLY for ACCMASS
  //  massdiff = 1e6*massdiff/result->neutral_mass_;

  MixtureDistr::enter(0, massdiff);

}

int MassDifferenceDiscrMixtureDistr::inttranslate(const char* val) {
  double value = atof(val);
  for(int k = 0; k < numbins_; k++)
    if(value <= (-1.0 * range_ + (k * window_)) + window_ / 2.0)
      return k;
  return numbins_ - 1;
}

Boolean MassDifferenceDiscrMixtureDistr::haveDataWithValue(int bin) {
  for(int k = 0; k < intvals_->length(); k++)
    if((*intvals_)[k] == bin)
      return True;
  return False;
}

void MassDifferenceDiscrMixtureDistr::writeDistr(FILE* fout) {
  
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


