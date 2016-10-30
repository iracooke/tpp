#ifndef PROBID_DISCRIMV_H
#define PROBID_DISCRIMV_H

#include "Validation/DiscriminateFunction/DiscrimValMixtureDistr.h"
//#include "Validation/Distribution/ExtremeValueDistribution.h"
//#include "Validation/Distribution/GaussianDistribution.h"

#include "ProbIDDiscrimFunction.h"
#include "common/sysdepend.h"
#include "Validation/Distribution/ExtremeValueDistribution.h"
/*

Program       : MascotDiscrimValMixtureDistr for PeptideProphet                                                       
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

class ProbIDDiscrimValMixtureDistr : public DiscrimValMixtureDistr {

 public:
  ProbIDDiscrimValMixtureDistr(int charge, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean nonparam=false);
  //  double getPosProb(int index);
  //  double getNegProb(int index);
  Boolean initializeNegDistribution(NTTMixtureDistr* nttdistr);
  Boolean initializeNegGammaDistribution(NTTMixtureDistr* nttdistr);
  Boolean initializeNegExtremeValueDistribution(NTTMixtureDistr* nttdistr);
  Boolean initializeNegDistribution(Array<Boolean>* isdecoy);
  Boolean initializeNegGammaDistribution(Array<Boolean>* isdecoy);
  Boolean initializeNegExtremeValueDistribution(Array<Boolean>* isdecoy);
  //  Boolean noDistr();
  void setDiscrimFunction(const char* mass_spec_type);
  //  Boolean reinitialize();
  Boolean noDistr();
  Boolean noDistrGamma();
  Boolean noDistrEVD();
  void setNegativeProbidDistr(double mean, double stdev);
  double getMinVal() { return (-12.0); }
  double getMaxVal() { return (30.0); }
  double getWindow() { return (0.2); }

 protected:

  //  void setNegativeDistr(double mean, double stdev);
  Boolean gammapos_; // whether pos distributions are gamma

};

#endif
