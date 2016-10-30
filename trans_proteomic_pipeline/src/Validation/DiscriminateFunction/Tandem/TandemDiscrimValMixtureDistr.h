#ifndef TANDEM_DISCRIMV_H
#define TANDEM_DISCRIMV_H

#include <string>

#include "common/sysdepend.h"
#include "Validation/DiscriminateFunction/DiscrimValMixtureDistr.h"
#include "Validation/Distribution/ExtremeValueDistribution.h"
#include "Validation/Distribution/GaussianDistribution.h"
#include "TandemDiscrimFunction.h"

/*

Program       : TandemDiscrimValMixtureDistr for PeptideProphet                                                       
Author        : Brendan MacLean <bmaclean%at%fhcrc.org>                                                       
Date          : 06.20.06 

Copyright (C) 2006 Brendan MacLean and Andrew Keller

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
#include "Validation/MixtureModel/OrderedResult.h"

using namespace std;

class TandemDiscrimFunctionFactory;

class TandemDiscrimValMixtureDistr : public DiscrimValMixtureDistr {

 public:
  TandemDiscrimValMixtureDistr(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof);
  TandemDiscrimValMixtureDistr(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean gamma);
  TandemDiscrimValMixtureDistr(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean gamma, Boolean nonparam);
  TandemDiscrimValMixtureDistr(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean gamma, Boolean nonparam, Boolean use_expect);
  void init(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean gamma);
  void init(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean gamma, Boolean nonparam);
  void init(int charge, const char* engine, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean gamma, Boolean nonparam, Boolean use_expect);
  double getPosProb(int index);
  double getNegProb(int index);
  Boolean initializeNegDistribution(NTTMixtureDistr* nttdistr);
  Boolean initializeNegDistribution(Array<Boolean>* isdecoy);


  //  Boolean noDistr();
  void setDiscrimFunction(const char* mass_spec_type);

  virtual Boolean update(Array<double>* probs);
 
  // HENRY
  virtual double getMinVal();
  virtual double getMaxVal();
  // END HENRY

 protected:

  Boolean initializeNegGammaDistribution(NTTMixtureDistr* nttdistr);
  Boolean initializeNegExtremeValueDistribution(NTTMixtureDistr* nttdistr);

  Boolean initializeNegGammaDistribution(Array<Boolean>* isdecoy);
  Boolean initializeNegExtremeValueDistribution(Array<Boolean>* isdecoy);

  void setNegativeDistr(double mean, double stdev);

  void setNegativeDistr(double mean, double stdev, double tot);
  std::string engine_;
  Boolean gammapos_; // whether pos distributions are gamma
  string algorithmName_;
  Boolean use_expect_;
 public:
  static void registerDiscrimFunctionFactory(TandemDiscrimFunctionFactory* factory);
};

#include "Validation/MixtureDistribution/DiscrimFunctionFactory.h"
class TandemDiscrimFunctionFactory : public DiscrimFunctionFactory
{
 public:
  TandemDiscrimFunctionFactory(const char* name) : DiscrimFunctionFactory(name)
  {
      TandemDiscrimValMixtureDistr::registerDiscrimFunctionFactory(this);
  }

  virtual TandemDiscrimFunction* createDiscrimFunction(int charge) = 0;
  virtual TandemDiscrimFunction* createDiscrimFunction(int charge, Boolean use_expect) = 0;

};

#endif
