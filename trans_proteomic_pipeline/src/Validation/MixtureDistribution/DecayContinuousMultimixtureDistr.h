#ifndef DECAY_MULT_H
#define DECAY_MULT_H

#include "ContinuousMultimixtureDistr.h"
#include "NTTMixtureDistr.h"

/*

Program       : DecayContinuousMultimixtureDistr for PeptideProphet                                                       
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


class DecayContinuousMultimixtureDistr : public ContinuousMultimixtureDistr {

 public:

  DecayContinuousMultimixtureDistr(double maxdiff, Boolean qtof);
  virtual ~DecayContinuousMultimixtureDistr();

  void initWithNTTs(NTTMixtureDistr* ntt);
  int presetZvalue(int index);
  double getMixtureProb(int index, double val);
  double getMixtureProbWithNTT(int ntt, int index);
  void addVal(double wt, double val);
  Boolean update();
  void printDistr();
  void writeDistr(FILE* fout);
  Boolean updateDistr(int k);
  void commence();
  Boolean oneProb(double val);
  void removeViolatingDistrs();
  Boolean violation(int leftdistr, int rightdistr);
  Boolean unmixed(int leftdistr, int rightdistr);
  void reset();
  double sliceWithNTT(double left_val, double right_val, int ntt);
  void writeNondecayProbs(FILE* fout, Array<double>* probs);


 protected:

  NTTMixtureDistr* ntt_;
  Array<double*>* nttpriors_;
  double* newtotnttpriors_;
  double* newnttpriors_;
  Boolean qtof_;
  double min_total_wts_;
  Boolean equal_stdevs_;
  double* nttdistrtots_;
  Boolean* mean_bar_;
};

#endif
