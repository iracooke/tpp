#ifndef SPECTRAST_DISCRIMV_H
#define SPECTRAST_DISCRIMV_H

#include "Validation/DiscriminateFunction/DiscrimValMixtureDistr.h"
#include "Validation/Distribution/NonParametricDistribution.h"
#include "Validation/Distribution/GaussianDistribution.h"
#include "Validation/Distribution/GeneralizedGaussianDistribution.h"
#include "Validation/Distribution/GammaDistribution.h"
#include "Validation/Distribution/ExtremeValueDistribution.h"
#include "SpectraSTDiscrimFunction.h"
#include "common/sysdepend.h"
/*

Program       : SpectraSTDiscrimValMixtureDistr for PeptideProphet                                                       
Author        : Henry Lam <hlam@systemsbiology.org>                                                       
Date          : 04.10.06 

Copyright (C) 2006 Henry Lam

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

Henry Lam
Institute for Systems Biology
401 Terry Avenue North 
Seattle, WA  98109  USA
hlam@systemsbiology.org

*/

class SpectraSTDiscrimValMixtureDistr : public DiscrimValMixtureDistr {

 public:
  SpectraSTDiscrimValMixtureDistr(int charge, const char* name, const char* tag, Boolean maldi, Boolean qtof, Boolean nonparam=false, Boolean optimizefval=false);
  void setDiscrimFunction(const char* mass_spec_type);
  Boolean initializeNegDistribution(NTTMixtureDistr* nttdistr);

  Boolean noDistr();
  virtual Boolean update(Array<double>* probs);  
  virtual double getMinVal() { return (min_fval_); }
  virtual double getMaxVal() { return (max_fval_); }
  virtual double getWindow() { return ((max_fval_ - min_fval_) / 50.0); }

  virtual void enter(SearchResult* result);
  void optimizeFval();

  void setConservative(float num_stdev);
  
 protected:

  Boolean optimize_fval_; 
   
  vector<double> dots_;
  vector<double> deltas_;
  double sum_fval_;
  double min_fval_;
  double max_fval_;
  float conservative_num_stdev_;

};

#endif // SPECTRAST_DISCRIMV_H
