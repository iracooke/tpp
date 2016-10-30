#ifndef DISTR_H
#define DISTR_H


#include <iostream>
#include <stdio.h>
#include <math.h>

#include "common/sysdepend.h"
#include "Parsers/Parser/Tag.h"
/*

Program       : Distribution for PeptideProphet                                                       
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

class Distribution {

public:
  Distribution();
  Distribution(double maxdiff);
  virtual ~Distribution(); 

  virtual double getProb(double val) = 0;
  virtual double getProb(int val) = 0;
  virtual int getCount(int val);
  virtual void init(double* priors) = 0;
  virtual void init(double* priors, Boolean alphabeta);
  virtual void addVal(double wt, double val) = 0;
  virtual void addVal(double wt, int val) = 0;
  virtual Boolean update() = 0;
  virtual Boolean update(double wt) { return update(); }
  virtual void initUpdate(double* prior) = 0;
  virtual void printDistr() = 0;
  virtual void writeDistr(FILE* fout) = 0;
  //virtual double getTot() { return newtotwt_; }
  virtual void setMinVal(double min); 
  virtual Array<Tag*>* getSummaryTags(Boolean pos) { return NULL; }
  virtual void setPriors(double *priors,double numpriors); // DiscreteDistribution implements this, at least

  protected: 

  double* tot_; 
  double totwt_;
  double* newtot_; // for update 
  double newtotwt_; // for update Boolean set_;
  double maxdiff_;
  Boolean set_;
  double minval_;
};

















#endif
