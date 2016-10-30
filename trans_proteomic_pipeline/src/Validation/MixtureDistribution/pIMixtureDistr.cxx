
#include "pIMixtureDistr.h"


/*

Program       : pIMixtureDistr for PeptideProphet                                                       
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


pIMixtureDistr::pIMixtureDistr(int charge, const char* name, const char* tag) : DiscreteMixtureDistr(charge, 112, name, tag) {
  const char* bindefs[] = {"(-inf, -10.0)", "[-10.0, -9.5)", "[-9.5, -9.0)", "[-9.0, -8.5)", "[-8.5, -8.0)", "[-8.0, -7.5)", "[-7.5, -7.0)", "[-7.0, -6.5)", "[-6.5, -6.0)", "[-6.0, -5.5)", "[-5.5, -5.0)", "[-5.0, -4.5)", "[-4.5, -4.0)", "[-4.0, -3.5)", "[-3.5, -3.0)", "[-3.0, -2.5)", "[-2.5, -2.0)", "[-2.0, -1.9)","[-1.9, -1.8)","[-1.8, -1.7)","[-1.7, -1.6)", "[-1.6, -1.5)", "[-1.5, -1.4)", "[-1.4, -1.3)", "[-1.3, -1.2)", "[-1.2, -1.1)", "[-1.1, -1.0)", "[-1.0, -0.9)","[-0.9, -0.8)","[-0.8, -0.7)","[-0.7, -0.6)", "[-0.6, -0.5)", "[-0.5, -0.4)", "[-0.4, -0.3)", "[-0.3, -0.2)", "[-0.2, -0.1)", "[-0.1, 0.1]", "(0.1, 0.2]", "(0.2, 0.3]", "(0.3, 0.4]", "(0.4, 0.5]", "(0.5, 0.6]", "(0.6, 0.7]", "(0.7, 0.8]", "(0.8, 0.9]", "(0.9, 1.0]", "(1.0, 1.1]",  "(1.1, 1.2]", "(1.2, 1.3]", "(1.3, 1.4]", "(1.4, 1.5]", "(1.5, 1.6]", "(1.6, 1.7]", "(1.7, 1.8]", "(1.8, 1.9]", "(1.9, 2.0]", "(2.0, 2.5]", "(2.5, 3.0]", "(3.0, 3.5]", "(3.5, 4.0]", "(4.0, 4.5]", "(4.5, 5.0]", "(5.0, 5.5]", "(5.5, 6.0]", "(6.0, 6.5]", "(6.5, 7.0]", "(7.0, 7.5]", "(7.5, 8.0]", "(8.0, 8.5]", "(8.5, 9.0]", "(9.0, 9.5]", "(9.5, 10.0]", "(10, 11]", "(11, 12]", "(12, 13]", "(13, 14]", "(14, 15]", "(15, 16]", "(16, 17]", "(17, 18]", "(18, 19]", "(19, 20]", "(20, 21]", "(21, 22]", "(22, 23]", "(23, 24]", "(24, 25]", "(25, 26]", "(26, 27]", "(27, 28]", "(28, 29]", "(29, 30]", "(30, 31]", "(31, 32]", "(32, 33]", "(33, 34]", "(34, 35]", "(35, 36]", "(36, 37]", "(37, 38]", "(38, 39]", "(39, 40]", "(40, 41]", "(41, 42]", "(42, 43]", "(43, 44]", "(44, 45]", "(45, 46]", "(46, 47]", "(47, 48]", "(48, 49]", "(49, inf)" };
		 
  
  maxdiff_ = 0.005;
  negOnly_ = True;
  //DDS: pI model
  pI_calc_ = new pICalculator();
  run_pI_calc_ = new Array<pICalculator*>();
}

pIMixtureDistr::~pIMixtureDistr() {
  if(run_pI_calc_ != NULL) {
    for (int i = 0; i < run_pI_calc_->size(); i++) 
      delete (*run_pI_calc_)[i];
    delete run_pI_calc_;
  }
  if(pI_calc_ != NULL) 
    delete pI_calc_;
}

void pIMixtureDistr::calc_pIstats() {
  for (int i = 0; i < run_pI_calc_->size(); i++) {
    (*run_pI_calc_)[i]->calc_pIstats();
    for (int j=0; j < (*run_pI_calc_)[i]->pIs_.size(); j++) {
      //cout <<"DDS: here" << endl; 
      MixtureDistr::enter(0, getpIBinNo((*run_pI_calc_)[i]->getpIScore((*run_pI_calc_)[i]->pIs_[j], (*(*run_pI_calc_)[i]->peps_)[j])));
    }
  }
}


void pIMixtureDistr::enter(SearchResult* result) {
  
  //DDS: pI model
  while (run_pI_calc_->size() <= result->run_idx_) { // BSP this was "if" instead of "while"
    run_pI_calc_->insertAtEnd(new pICalculator(run_pI_calc_->size()));
  }
  
#ifdef USE_STD_MODS
  // have access to modifications
  

  double calc_pi = (*run_pI_calc_)[result->run_idx_]->Peptide_pI(result->peptide_, result->mod_info_, result->pI_);
  return;
#endif

  cout << "USE_STD_MODS is not enabled, cannot use pI model.  "
       << "Please contact spctools-discuss@googlegroups.com for further support." << endl;  
  exit(1);
  //MixtureDistr::enter(0, result->peptide_);
}

#ifndef USE_STD_MODS
int pIMixtureDistr::inttranslate(const char* val) {
  cout << "val: " << val << endl;

  // format X.PEPTIDE.X
  double result;
  if(strlen(val) > 4 && val[1] == '.')
    result = pI_calc_->ModifiedPeptide_pI(val, 2, strlen(val)-4);
  else
    result = pI_calc_->ModifiedPeptide_pI(val, 0, strlen(val));

  cout << "result: " << result << endl;

  return getpIBinNo((double)result);
  /*
  // decide which bin based on calculated pI
  int bin_no = (int)result;
  if(bin_no < 0 || bin_no >= numbins_) {
    cout << "error, bin_no " << bin_no << " for pI value: " << result << endl;
    return 0;
  }

  return bin_no;

  return 0;
  */
}
#endif

int pIMixtureDistr::getpIBinNo(double pI) {
  double binbounds_[] = {-10, -9.5, -9, -8.5, -8, -7.5, -7, -6.5, -6, -5.5, -5, -4.5, -4, -3.5, -3, -2.5, -2, -1.9, -1.8, -1.7, -1.6, -1.5, -1.4, -1.3, -1.2, -1.1, -1.0, -0.9, -0.8, -0.7, -0.6, -0.5, -0.4, -0.3, -0.2, -0.1, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 6.5, 7.0, 7.5, 8.0, 8.5, 9.0, 9.5, 10, 10.5, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49};

  int numbins = 112;  
  int neg = False;
  
  if (pI < 0) neg = True;

  for (int i=0; i<numbins; i++) {
    if (neg) {
      if (pI < binbounds_[i]) {
	return i;
      }
    }
    else {
      if (pI <= binbounds_[i]) {
	return i;
      }
    }
  }

  return numbins-1;


}


double pIMixtureDistr::getPosProb(int index) {
  if(index < 0 || index >= intvals_->size()) {
    cerr << "violation of index " << index << " for " << intvals_->size() << endl;
    exit(1);
  }
  double pos = 0;
  int offset = 0;
  int count = 0;
  int mincount = 30;
  while (count < mincount) {
    if (((*intvals_)[index]+offset >= numbins_) && ((*intvals_)[index]-offset < 0)) {
      break;
    }
    if ((*intvals_)[index]+offset < numbins_) {
      count += posdistr_->getCount((*intvals_)[index]+offset);
      count += negdistr_->getCount((*intvals_)[index]+offset);
      pos += posdistr_->getProb((*intvals_)[index]+offset);
    }   
    if (offset > 0 && (*intvals_)[index]-offset > 0) {
      count += posdistr_->getCount((*intvals_)[index]-offset);
      count += negdistr_->getCount((*intvals_)[index]-offset);
      pos += posdistr_->getProb((*intvals_)[index]-offset);
    }
    offset++ ;
  }
  
  return pos;
  
}

double pIMixtureDistr::getNegProb(int index) {
  if(index < 0 || index >= intvals_->size()) {
    cerr << "violation of index " << index << " for " << intvals_->size() << endl;
    exit(1);
  }
  double neg = 0;
  int offset = 0;
  int count = 0;
  int mincount = 30;
  while (count < mincount) {
    if (((*intvals_)[index]+offset >= numbins_) && ((*intvals_)[index]-offset < 0)) {
      break;
    }
    if ((*intvals_)[index]+offset < numbins_) {
      count += posdistr_->getCount((*intvals_)[index]+offset);
      count += negdistr_->getCount((*intvals_)[index]+offset);
      neg += negdistr_->getProb((*intvals_)[index]+offset);
    }   
    if (offset > 0 && (*intvals_)[index]-offset > 0) {
      count += posdistr_->getCount((*intvals_)[index]-offset);
      count += negdistr_->getCount((*intvals_)[index]-offset);
      neg += negdistr_->getProb((*intvals_)[index]-offset);
    }
    offset++ ;
  }
  
  return neg;
}
