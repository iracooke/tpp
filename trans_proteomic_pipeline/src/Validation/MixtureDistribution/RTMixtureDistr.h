#ifndef RT_DISTR_H
#define RT_DISTR_H

#include "DiscreteMixtureDistr.h"
#include "Parsers/Algorithm2XML/RTCalculator.h"

/*

Program       : RTMixtureDistr for PeptideProphet                                                       
Author        : Andrew Keller <akeller@systemsbiology.org>, David Shteynberg                                                       
Date          : 11.27.02 

Copyright (C) 2003 Andrew Keller (c) 2008 David Shteynberg

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
akeller@systemsbiology.org

*/


class RTMixtureDistr : public DiscreteMixtureDistr {

 public:
  RTMixtureDistr(int charge, const char* name, const char* tag);
  ~RTMixtureDistr();

  void enter(SearchResult* result);

 protected:
  Array<RTCalculator*>* run_RT_calc_;

  RTCalculator* all_RT_calc_;

};


#endif
