#ifndef GAUSS_H
#define GAUSS_H

#include <math.h>

#include "ContinuousDistribution.h"

/*

Program       : GaussianDistribution for PeptideProphet                                                       
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

class GaussianDistribution : public ContinuousDistribution {
 public:
  GaussianDistribution();
  GaussianDistribution(double maxdiff);
  virtual ~GaussianDistribution() {};
  double getProb(double val);
  double getProb(int val);
  double getGaussianProb(double val, double mean, double stdev);
  void init(double* prior);
  void addVal(double wt, int val);
  void addVal(double wt, double val);
  Boolean update();
  void computeAlphaBeta();
  void initUpdate(double* prior);
  void printDistr();
  Boolean oneProb(double val);
  double slice(double num, double left_val, double right_val);
  double gaussianSlice(double left_val, double right_val);
  double gaussianSlice(double num, double left_val, double right_val, double mean, double stdev);
  double N(double z);
  void writeDistr(FILE* fout);
  Array<Tag*>* getSummaryTags(Boolean pos);


 protected:

};

#endif
