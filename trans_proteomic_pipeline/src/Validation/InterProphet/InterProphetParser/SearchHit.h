#ifndef _SEARCH_HIT_H_
#define _SEARCH_HIT_H_
//#define PROGRAM_VERSION "InterProphet v3.0 April 1, 2004  ISB"
//#define PROGRAM_AUTHOR "David Shteynberg"
/*

Program       : InterProphet                                                       
Author        : David Shteynberg <dshteynb  AT systemsbiology.org>                                                       
Date          : 12.12.07

Primary data object holding all mixture distributions for each precursor ion charge

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

*/
#include "SearchHit.h"
#include "common/util.h"
#include "common/sysdepend.h"
#include "KDModel.h"
#include "BoolModel.h"

#include "common/tpp_hashmap.h" // deals with different compilers
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

class SearchHit {

 public:
  SearchHit();
  SearchHit(const int run_idx, string& spect, 
	    double pepproph_prob, Array<double>* allntt_pepproph_prob, 
	    string& pepseq, string& modpep,  string& swathpep, 
	    string& msrun, double calcnmass, string& exp, string& charge);
  SearchHit(const int run_idx, string& spect, 
	    double pepproph_prob, Array<double>* allntt_pepproph_prob, 
	    string& pepseq, string& modpep,  string& swathpep, 
	    string& msrun, double calcnmass, string& exp, string& charge, Array<string*>* prots);
  SearchHit(const int run_idx, string& spect, 
	    double pepproph_prob, Array<double>* allntt_pepproph_prob, 
	    string& pepseq, string& modpep,  string& swathpep, 
	    string& msrun, double calcnmass, double rt, string& exp, string& charge);
  SearchHit(const int run_idx, string& spect, 
	    double pepproph_prob, Array<double>* allntt_pepproph_prob, 
	    string& pepseq, string& modpep,  string& swathpep, 
	    string& msrun, double calcnmass, double rt, int swath_window, int alt_swath, string& exp, string& charge, Array<string*>* prots);
  ~SearchHit();
  
  void setMaxFPKM(double fpkm) { 
    fpkm_ = fpkm;
  }

  Array<string*> * prots_;
  int run_idx_;
  double pepproph_prob_;
  double allntt_pepproph_prob_[3];
  double nssadj_prob_;
  double allntt_nssadj_prob_[3];
  double adj_prob_;
  double allntt_adj_prob_[3];
  double calcnmass_;
  double rt_;
  double nss_;
  double nrs_;
  double nse_;
  double nsi_;
  double nsm_;
  double nsp_;
  double nsw_;
  double fpkm_;
  string chg_;
  string spect_;
  string exp_;
  string peptide_;
  string modpep_;
  string swathpep_;
  int swathwin_;
  int altswath_;
  string msrun_;
  bool topcat_;
  bool nocat_;
};

using namespace std;

typedef TPP_STDSTRING_HASHMAP(SearchHit*) hit_hash;
typedef TPP_STDSTRING_HASHMAP(Array<SearchHit*>*) arrhit_hash;
typedef TPP_STDSTRING_HASHMAP(Array<double>*) dblarr_hash;
typedef TPP_STDSTRING_HASHMAP(dblarr_hash*) dblarr_hash_hash;
typedef TPP_STDSTRING_HASHMAP(Array<int>*) intarr_hash;
typedef TPP_STDSTRING_HASHMAP(Array<string*>*) strparr_hash;
typedef TPP_STDSTRING_HASHMAP(double) dbl_hash;
typedef TPP_STDSTRING_HASHMAP(string) str_hash;
typedef TPP_STDSTRING_HASHMAP(int) int_hash;
typedef TPP_STDSTRING_HASHMAP(int) bool_hash;
typedef TPP_STDSTRING_HASHMAP(dbl_hash*) dbl_hash_hash;
typedef TPP_STDSTRING_HASHMAP(dbl_hash_hash*) dbl_hash_hash_hash;
typedef TPP_STDSTRING_HASHMAP(int_hash*) int_hash_hash;

#endif
