#ifndef DISCRVAL_MIX_H
#define DISCRVAL_MIX_H

#include "Validation/MixtureDistribution/MixtureDistr.h"
#include "Validation/MixtureDistribution/NTTMixtureDistr.h"
#include "Validation/Distribution/NonParametricDistribution.h"
#include "Validation/Distribution/ContinuousDistribution.h"
#include "Validation/MixtureDistribution/DecayContinuousMultimixtureDistr.h"
#include "DiscriminantFunction.h"
#include "Validation/DiscriminateFunction/Sequest/AbbrevSequestDiscrimFunction.h"
/*

Program       : DiscrimValMixtureDistr for PeptideProphet                                                       
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


class DiscrimValMixtureDistr : public MixtureDistr {

 public:
  DiscrimValMixtureDistr(int charge, const char* name, const char* tag, Boolean gamma, Boolean maldi, Boolean qtof, Boolean nonparam=false);
  DiscrimValMixtureDistr(int charge, const char* name, const char* tag, Boolean gamma, Boolean maldi, Boolean qtof, Boolean nonparam=false, bool use_expect=false);
  DiscrimValMixtureDistr() { };
  virtual ~DiscrimValMixtureDistr();

  virtual Boolean initializeNegDistribution(NTTMixtureDistr* nttdistr);
  virtual Boolean initializeNegDistribution(Array<Boolean>* isdecoy);

  void setConservative(float stdev);
  double getPosProb(int index);
  double getNegProb(int index);

  void writePosDistribution();
  void enter(int index, double val);
  virtual void enter(SearchResult* result);
  bool valid(SearchResult* result);

  void printDistr();
  int slice(double left_val, double right_val);
  double posSlice(double left_val, double right_val);
  double negSlice(double left_val, double right_val);
  double getRightCumulativeNegProb(int index, double right_val);
  double getRightCumulativeNegProb(double total, int index, double right_val);
  void writeDistr(FILE* fout);
  void reset();
  virtual Boolean noDistr();
  Boolean update(Array<double>* probs);
  Boolean finalupdate(Array<double>* probs);
  void resetTot();
  Boolean decayMultimixture();
  double posSliceWithNTT(double left_val, double right_val, int ntt);

  virtual Boolean reinitialize();
  virtual void setDiscrimFunction(const char* mass_spec_type);
  virtual Array<Tag*>* getMixtureDistrTags(const char* name);
 
  // HENRY: supplies the minval, maxval and window for MixtureModel to print histograms. These are the defaults. Subclasses should override these methods to specify their own values suited to their respective F value range.
  virtual double getMinVal() { return (-5.0); }
  virtual double getMaxVal() { return (10.0); }
  virtual double getWindow() { return (0.2); }
  // END HENRY

 protected:

  double* copy(double* init, int num);
  double maxdiff_;
  Boolean gamma_;
  Array<Boolean>* isdecoy_;
  double min_dataval_;
  double minval_; // for gamma distribution
  void setNegativeDistr(double mean, double stdev, double zero);
  void setPositiveDistr(double mean, double stdev);
  void setPositiveDistr(double mean, double stdev, double tot);
  double negmean_;
  int MIN_NUM_PSEUDOS_;
  int ZERO_SET_;
  int NUM_DEVS_;
  Boolean USE_TR_NEG_DISTR_;
  double* posinit_;
  double* neginit_;
  Boolean maldi_;
  Boolean all_negs_;
  Boolean qtof_;
  Boolean nonparam_;
  float consStdev_;
  int reinit_;
  DiscriminantFunction* discrim_func_;

  bool use_expect_;
}; 



#endif
