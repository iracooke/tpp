#ifndef MIX_DISTR
#define MIX_DISTR
/*

Program       : MixtureDist                                                       
Author        : Andrew Keller <akeller@systemsbiology.org>                                                       
Date          : 11.27.02 

Primary data object holding all mixture distributions for each precursor ion charge

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
#include <iostream>
//#undef abs
#include "common/sysdepend.h"
#include <string>
#include "common/Array.h"
#include "Validation/Distribution/GammaDistribution.h"
#include "Validation/Distribution/GaussianDistribution.h"
#include "Validation/Distribution/DiscreteDistribution.h"
#include "Validation/Distribution/Distribution.h"
#include "Parsers/Algorithm2XML/SearchResult/SearchResult.h"
#include "Parsers/Parser/Tag.h"

/*

Program       : MixtureDistr for PeptideProphet                                                       
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

Institute for Systems Biology, hereby disclaims all copyright interest 
in PeptideProphet written by Andrew Keller

*/

class MixtureDistr {

 public:

  MixtureDistr();
  MixtureDistr(int charge, const char* name, const char* tag);
  virtual ~MixtureDistr();
  virtual double getPosProb(int index);
  virtual double getNegProb(int index);
  virtual void enter(int index, const char* val);
  virtual Boolean update(Array<Array<double>*>* all_probs, int charge);
  virtual Boolean update(Array<Array<double>*>* all_probs, const char* c);
  virtual Boolean update(Array<Array<double>*>* all_probs, double min_prob);
  virtual Boolean update(Array<Array<double>*>* all_probs, double min_prob, Array<Array<int>*>* all_ntts, int min_ntt);
  virtual Boolean update(Array<Array<double>*>* all_probs, double min_prob, Array<Array<int>*>* all_ntts, int min_ntt, int& code);
  virtual Boolean update(Array<Array<double>*>* all_probs, double min_prob, Array<Array<int>*>* all_ntts, int min_ntt, int& code, const char* c);
  virtual  Boolean update(Array<double>* probs, double min_prob, Array<int>* ntts, int  min_ntt, int& code);
  virtual Boolean update(Array<double>* probs);
  virtual Boolean update(Array<Array<double>*>* probs);
  Boolean update(Array<double>* probs, Array<Boolean>* isdecoy, Boolean final);
  virtual void write_pIstats(ostream& out);
  virtual void write_RTstats(ostream& out);
  virtual void write_RTcoeff(ostream& out);
  virtual const char* getName();
  virtual const char* getAbbrevName();
  virtual const char* getTag();
  virtual void printVal(int index);
  virtual void enter(int index, double val);
  virtual void enter(int index, int val);
  virtual void enter(SearchResult* result) = 0;
  virtual bool valid(SearchResult* result) { return true; }
  virtual void calc_pIstats() {};
  
  virtual void printDistr();

  virtual void setConservative(float) { };
  int getNumVals();
  Boolean isValue(int index, int val);
  virtual void setPosDistr(MixtureDistr* distr);
  virtual void setNegDistr(MixtureDistr* distr);
  virtual Distribution* getPosDistr();  
  virtual Distribution* getNegDistr();
  virtual Boolean negOnly();
  virtual void writeDistr(FILE* fout);
  virtual char* getStringValue(int index);
  virtual Array<Tag*>* getMixtureDistrTags(const char* name);

  protected:

  virtual int inttranslate(const char* val);
  virtual double doubletranslate(const char* val);
  void initializeDistr(int charge, const char* name, const char* tag);
  Distribution* posdistr_;
  Distribution* negdistr_;
  std::string name_;
  std::string abbrev_name_;
  std::string tag_; // for data entry
  Array<int>* intvals_;
  Array<double>* doublevals_;
  int charge_;
  double* pospriors_;
  double* negpriors_;
  double minval_;
  double maxval_;
  Boolean negOnly_;
  bool posDistReset_;
  bool negDistReset_;
};

#endif
