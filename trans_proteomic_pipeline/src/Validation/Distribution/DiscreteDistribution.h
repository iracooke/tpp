#ifndef DISCR_DISTR_H
#define DISCR_DISTR_H


#include "Distribution.h"

/*

Program       : DiscreteDistribution for PeptideProphet                                                       
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


class DiscreteDistribution : public Distribution {
  public:
  DiscreteDistribution();
  DiscreteDistribution(double maxdiff);
  DiscreteDistribution(int numbins, double maxdiff);
  virtual ~DiscreteDistribution();

  double getProb(double val);
  double getProb(int val);
  void init(double* priors);
  int getCount(int val);
  void addVal(double wt, int val);
  void addVal(double wt, double val);
  Boolean update();
  void initUpdate(double* prior);
  void printDistr();
  virtual void setPriors(double* priors, double numpriors);
  void writeDistr(FILE* fout);
  int getNumBins();

  protected:
  void common_ctor_init(); // init code for ctors
  int *count_;
  int *newcount_;
  int num_bins_;
  double* priors_;
  double num_priors_;

};




#endif 
