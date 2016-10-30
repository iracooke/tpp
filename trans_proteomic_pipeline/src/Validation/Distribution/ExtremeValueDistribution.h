#ifndef EXTREME_H
#define EXTREME_H

#include <math.h>

#include "ContinuousDistribution.h"

/*

Program       : ExtremeValueDistribution for PeptideProphet                                                       
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

class ExtremeValueDistribution : public ContinuousDistribution {

 public:
  ExtremeValueDistribution(double maxdiff);
  void init(double* prior);
  void initUpdate(double* prior);
  void addVal(double wt, double val);
  void addVal(double wt, int val);
  Boolean update();
  double getProb(int val);
  double getExtremeValueProb(double val, double beta, double mu);
  double getProb(double val);
  void computeMoments(double mean, double std);
  double cumulative(double x, double mu, double beta);
  double extremeValueSlice(double num, double left_val, double right_val, double mu, double beta);
  double slice(double num, double left_val, double right_val);
  void printDistr();
  void writeDistr(FILE* fout);
  Boolean aboveMin(double val);
  void setDistrMinval(double val);
  Array<Tag*>* getSummaryTags(Boolean pos);

 protected:
  double beta_;
  double mu_;
  double* minval_; // below which is excluded from model (and given prob 0)

};



#endif
