#ifndef GENERALIZED_GAUSS_H
#define GENERALIZED_GAUSS_H

#include <vector>
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

using namespace std;

class GeneralizedGaussianDistribution : public ContinuousDistribution {
 public:
  GeneralizedGaussianDistribution();
  GeneralizedGaussianDistribution(double maxdiff);
  virtual ~GeneralizedGaussianDistribution() {};
  double getProb(double val);
  double getProb(int val);
  double getGeneralizedGaussianProb(double val);
  void init(double* prior);
  void addVal(double wt, int val);
  void addVal(double wt, double val);
  Boolean update();
  void initUpdate(double* prior);
  void printDistr();
  Boolean oneProb(double val);
  double slice(double num, double left_val, double right_val);
  double generalizedGaussianSlice(double left_val, double right_val);
  double generalizedGaussianSlice(double num, double left_val, double right_val);
  void writeDistr(FILE* fout);
  Array<Tag*>* getSummaryTags(Boolean pos);


 protected:
  double shape_;
  double scalingFactor_;
  double normalizationFactor_;

  vector<double> allvals_;
  vector<double> allwts_;
  
  double computeShape(double M);

};

#endif
