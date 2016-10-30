#ifndef ORDERED_RES_H
#define ORDERED_RES_H

/*
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
#include "common/constants.h"
class OrderedResult {

 public:

  OrderedResult(){}; // danger!!! values could be anything
  OrderedResult(int charge, int ind, int input);
  void init(int charge, int ind, int input) {
    init(charge,ind,input,false);
  }
  void init(int charge, int ind, int input, Boolean isdec) {
    charge_ = charge;
    index_ = ind;
    input_index_ = input;
    is_decoy_ = isdec;
  }
  
  int charge_;
  int index_;
  int input_index_; // to filter by
  Boolean is_decoy_;
};

#endif
