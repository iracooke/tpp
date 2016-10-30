#ifndef YABSE_DISCRIMV_H
#define YABSE_DISCRIMV_H

#include <string>

#include "common/sysdepend.h"
#include "Validation/DiscriminateFunction/DiscrimValMixtureDistr.h"
#include "Validation/Distribution/ExtremeValueDistribution.h"
#include "Validation/Distribution/GaussianDistribution.h"
#include "YABSEDiscrimFunction.h"

/*
 * WARNING!! This discriminant function is not yet complete.  It is presented
 *           here to help facilitate trial and discussion.  Reliance on this
 *           code for publishable scientific results is not recommended.
 */

/*

Program       : YABSEDiscrimValMixtureDistr for PeptideProphet                                                       
Author        : David Shteynberg <dshteynb%at%systemsbiology.org>                                                       
Date          : 05.16.07 


Copyright (C) 2007 David Shteynberg and Andrew Keller

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
#include "Validation/MixtureModel/OrderedResult.h"

using namespace std;

class YABSEDiscrimFunctionFactory;

class YABSEDiscrimValMixtureDistr : public DiscrimValMixtureDistr {

 public:
  YABSEDiscrimValMixtureDistr(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean gamma, Boolean nonparam);
  YABSEDiscrimValMixtureDistr(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean gamma);
  YABSEDiscrimValMixtureDistr(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof);
  void init(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean gamma);
  void init(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean gamma, Boolean nonparam);
  double getPosProb(int index);
  double getNegProb(int index);
  //Boolean initializeNegDistribution(NTTMixtureDistr* nttdistr);
  Boolean initializeNegDistribution(Array<Boolean>* isdecoy);


  //  Boolean noDistr();
  void setDiscrimFunction(const char* mass_spec_type);

  // HENRY
  virtual double getMinVal();
  virtual double getMaxVal();
  // END HENRY

 protected:

  // Boolean initializeNegGammaDistribution(NTTMixtureDistr* nttdistr);
  // Boolean initializeNegExtremeValueDistribution(NTTMixtureDistr* nttdistr);

  //Boolean initializeNegGammaDistribution(Array<Boolean>* isdecoy);
  //Boolean initializeNegExtremeValueDistribution(Array<Boolean>* isdecoy);

  //void setNegativeDistr(double mean, double stdev);
  
  std::string engine_;
  Boolean gammapos_; // whether pos distributions are gamma
  string algorithmName_;

 public:
  static void registerDiscrimFunctionFactory(YABSEDiscrimFunctionFactory* factory);
};

#include "Validation/MixtureDistribution/DiscrimFunctionFactory.h"
class YABSEDiscrimFunctionFactory  : public DiscrimFunctionFactory
{
 public:
  YABSEDiscrimFunctionFactory(const char* name) : DiscrimFunctionFactory(name)
  {
      YABSEDiscrimValMixtureDistr::registerDiscrimFunctionFactory(this);
  }

  virtual YABSEDiscrimFunction* createDiscrimFunction(int charge) = 0;

};

#endif

