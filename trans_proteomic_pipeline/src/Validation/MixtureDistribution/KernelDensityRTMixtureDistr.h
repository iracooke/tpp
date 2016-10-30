#ifndef KERD_RTD_H
#define KERD_RTD_H

#include <assert.h>

#include "RTMixtureDistr.h"
#include "Validation/InterProphet/InterProphetParser/KDModel.h"


/*

Program       : KernelDensityRTMixtureDistribution for PeptideProphet                                                       
Author        : Andrew Keller <akeller@systemsbiology.org>                                
                David Shteynberg
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

class KernelDensityRTMixtureDistr : public RTMixtureDistr {

 public:

  KernelDensityRTMixtureDistr(int charge, const char* name, const char* tag);
  ~KernelDensityRTMixtureDistr();
  int getIntegralValue(double val);
  double getMode(double window, Array<double>* probs);
  void enter(int index, char* val);
  Boolean update(Array<Array<double>*>* probs);
  Boolean update(Array<Array<double>*>* probs, const char* catfile);
  //  Boolean update(Array<Array<double>*>* all_probs, double min_prob);
  Boolean update(Array<Array<double>*>* probs, double min_prob, Array<Array<int>*>* ntts, int  min_ntt, int& code);

  Boolean update(Array<Array<double>*>* all_probs, double min_prob, Array<Array<int>*>* all_ntts, int min_ntt, int& code, const char* catfile);

  Boolean update(Array<double>* probs, double min_prob, Array<int>* ntts, int  min_ntt, int& code);
  
  //DDS: using parent  void enter(SearchResult* result);
  Array<Tag*>* getMixtureDistrTags(const char* name);
  char* getStringValue(int index);
  char* getStringRTValue(int index);
  Boolean haveDataWithValue(int bin);
  double getPosProb(int index);
  double getNegProb(int index);
  void recalc_RTstats(Array<Array<double>*>* probs);  
  void recalc_RTstats(Array<Array<double>*>* probs, bool catalog);  

  bool recalc_RTstats(Array<Array<double>*>* probs, double min_prob, Array<Array<int>*>* ntts, int  min_ntt);
  bool recalc_RTstats(Array<Array<double>*>* probs, double min_prob, Array<Array<int>*>* ntts, int  min_ntt, bool catalog);

  bool recalc_RTstats(Array<double>* probs, double min_prob, Array<int>* ntts, int  min_ntt);
  bool recalc_RTstats(Array<double>* probs, double min_prob, int  min_ntt);
  //  void recalc_RTstats(Array<Array<double>*>* probs, double min_prob);  
  int getNumVals();
  void calc_RTstats();  
  void write_RTcoeff(std::ostream& out);
  void write_RTstats(std::ostream& out);
 protected:
  int update_ctr_;
  int min_ctr_;
  Array<double>* vals_;
  Array<double>* RTvals_;
  KDModel* model_;
  bool ready_;


};


#endif
