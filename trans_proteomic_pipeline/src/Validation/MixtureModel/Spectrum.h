#ifndef SPECTRUM_H
#define SPECTRUM_H

/*

Program       : Spectrum for PeptideProphet                                                       
Author        : Andrew Keller <akeller@systemsbiology.org>                                                       
Date          : 11.27.02 

Object to enable sorting of 2+ and 3+ spectra by name

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
#include <string.h>
#include <stdlib.h>

class Spectrum {

 public:

  Spectrum() {
	  memset(this,0,sizeof(*this));
  }; 
  ~Spectrum() {
	  delete[] name_;
  }
  Spectrum(const char* name,  int ntt, int ind);
  Spectrum(const char* name,  int ntt, int ind, int data_ind);
  void init(const char* name, int ntt, int ind, int data_ind) {
          prob_ = 0;
	  int len;
	  name_ = new char[len=((int)strlen(name)+1)];
	  memcpy(name_,name,len);
	  // BSP - charge is expensive to extract from name_ repeatedly
	  charge_ = atoi(name+len-2)-1; // using index convention "1+"=0 "2+"=1 "3+"=2 etc
	  ntt_ = ntt;
	  ind_ = ind;
	  data_index_ = data_ind;
  }
  double prob_;
  char* name_;
  int ntt_;
  int ind_;
  int data_index_; // for original mixturedistr
  int charge_; // using index convention "1+"=0 "2+"=1 "3+"=2 etc
 };








#endif
