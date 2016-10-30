#ifndef CONT_DISTR_H
#define CONT_DISTR_H


#include "Distribution.h"

/*

Program       : ContinuousDistribution for PeptideProphet                                                       
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


class ContinuousDistribution : public Distribution {

  public:
  ContinuousDistribution();
  ContinuousDistribution(double maxdiff);
  virtual ~ContinuousDistribution() {};

  virtual void init(double* prior);
  virtual void init(double* prior, Boolean alphabeta);
  virtual double slice(double left_val, double right_val);
  virtual double slice(double num, double left_val, double right_val) = 0;
  virtual double getMean();
  virtual double getStdev();
  //virtual double getTot();

  void resetTot();
  void setTot(double tot);
  void setMean(double mean);
  void setStdev(double stdev);
  void ignoreStdev();


  protected:

  double mean_;
  double stdev_;
  double newtotsq_;
  Boolean use_stdev_;
};






# endif
