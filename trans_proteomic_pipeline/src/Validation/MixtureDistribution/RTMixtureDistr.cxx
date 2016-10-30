
#include "RTMixtureDistr.h"


/*

Program       : RTMixtureDistr for PeptideProphet                                                       
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


RTMixtureDistr::RTMixtureDistr(int charge, const char* name, const char* tag) : DiscreteMixtureDistr(charge, 1, name, tag) {
  //DDS: RT model
  //RT_calc_ = new RTCalculator();
  run_RT_calc_ = new Array<RTCalculator*>();
  all_RT_calc_ = new RTCalculator();
}

RTMixtureDistr::~RTMixtureDistr() {
  if(run_RT_calc_ != NULL) {
    for (int i = 0; i < run_RT_calc_->size(); i++) 
      delete (*run_RT_calc_)[i];
    delete run_RT_calc_;
  }
  delete all_RT_calc_;
}

void RTMixtureDistr::enter(SearchResult* result) {
  //  if(result->RT_ > 0.0) { // already have it
    //DDS: MixtureDistr::enter(0, getRTBinNo(result->RT_));
  // return;
    //}
  
  //DDS: Learn RT coeffs from all peptides together (not by run) 
  //DDS: RT model
  //while (run_RT_calc_->size() <= 0) { //result->run_idx_) {  // BSP this was "if", not "while"
  //  run_RT_calc_->insertAtEnd(new RTCalculator(run_RT_calc_->size()));
  //}
  
  //(*run_RT_calc_)[0]->addPeptide_RT(result->peptide_,result->modified_peptide_, result->scan_, result->RT_ );
  
  //DDS: Separate by run
  while (run_RT_calc_->size() <= result->run_idx_) {  // BSP this was "if", not "while"
    run_RT_calc_->insertAtEnd(new RTCalculator(result->run_name_));
    (*run_RT_calc_)[run_RT_calc_->size()-1]->read_RTcoeff();
  }
  
  (*run_RT_calc_)[result->run_idx_]->addPeptide_RT(result->peptide_,result->modified_peptide_, result->scan_, result->RT_, result->run_name_);
 
  
  //all_RT_calc_->addPeptide_RT(result->peptide_,result->modified_peptide_, result->scan_, result->RT_, result->run_idx_ );


  return;
}

