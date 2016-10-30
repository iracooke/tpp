#ifndef VAR_OR_RTD_H
#define VAR_OR_RTD_H

#include <assert.h>

#include "RTMixtureDistr.h"


/*

Program       : ChymotrypticEnzymeDigestion for PeptideProphet                                                       
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

class VariableOffsetRTMixtureDistr : public RTMixtureDistr {

 public:

  VariableOffsetRTMixtureDistr(int charge, char* name, char* tag, double range, double window, double orig);
  int getIntegralValue(double val);
  double getMode(double window, Array<double>* probs);
  void enter(int index, char* val);
  Boolean update(Array<Array<double>*>* probs);
  //  Boolean update(Array<Array<double>*>* all_probs, double min_prob);
  Boolean update(Array<Array<double>*>* probs, double min_prob, Array<Array<int>*>* ntts, int  min_ntt);
  void writeDistr(FILE* fout);
  //DDS: using parent  void enter(SearchResult* result);
  Array<Tag*>* getMixtureDistrTags(char* name);
  char* getStringValue(int index);
  char* getStringRTValue(int index);
  Boolean haveDataWithValue(int bin);
  void recalc_RTstats(Array<Array<double>*>* probs);  
  void recalc_RTstats(Array<Array<double>*>* probs, double min_prob, Array<Array<int>*>* ntts, int  min_ntt);
  //  void recalc_RTstats(Array<Array<double>*>* probs, double min_prob);  
  void calc_RTstats();  
  void write_RTstats(std::ostream& out);
 protected:
  double offset_;
  double offset_init_;
  Array<double>* vals_;
  Array<double>* RTvals_;
  int update_ctr_;
  int min_ctr_;
  Boolean offset_set_;


};


#endif
