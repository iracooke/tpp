#ifndef CONT_MULT_MIX_H
#define CONT_MULT_MIX_H

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "Validation/Distribution/ContinuousDistribution.h"
#include "Validation/Distribution/GaussianDistribution.h"
#include "Validation/Distribution/GammaDistribution.h"
#include "common/Array.h"

/*

Program       : ContinuousMultimixtureDistr for PeptideProphet                                                       
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

class ContinuousMultimixtureDistr : public ContinuousDistribution {


 public:

  ContinuousMultimixtureDistr(double maxdiff);
  virtual ~ContinuousMultimixtureDistr();
  void initialize();
  void addComponent(double* settings, const char* distr, const char* def);
  double getProb(double val);
  double getProb(int val);
  void init(double* priors); // set index_ to 0
  void addVal(double wt, double val);
  void addVal(double wt, int val);
  int getNumDistributions();
  Boolean update();
  void initUpdate(double* prior);
  void printDistr();
  void writeDistr(FILE* fout);
  void setMinVal(double min); 
  double slice(double num, double left_val, double right_val);
  double slice(double left_val, double right_val);
  virtual int presetZvalue(int index);
  double getMean();
  double getStdev();
  double getMixtureDistrProb(int k, double val);
  virtual void removeViolatingDistrs();
  virtual Boolean violation(int leftdistr, int rightdistr);

 protected:

  Array<ContinuousDistribution*>* distrs_;
  //Array<GaussianDistribution*>* distrs_;
  double* priors_;
  Array<double*>* zvals_;
  int index_;
  Array<double>* vals_; // keep copy of data
  Array<char*>* defs_; // describe each distr
  Array<double>* wts_;
  double totalwts_;
  Array<char*>* distrtypes_;
};

#endif
