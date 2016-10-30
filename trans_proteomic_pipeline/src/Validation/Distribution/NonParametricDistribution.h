#ifndef NONPARAMETRIC_DISTR_H
#define NONPARAMETRIC_DISTR_H


#include "ContinuousDistribution.h"
#include "GaussianDistribution.h"
#include "Fit.h"
#include <vector>

/*

Program       : NonParametricDistribution for PeptideProphet                                                       
Author        : Henry Lam <hlam@systemsbiology.org>                                                       
Date          : 07.20.07 

Copyright (C) 2007 Henry Lam

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
akeller@systemsbiology.org

*/

class NonParametricDistribution : public ContinuousDistribution {
  
  public:
  NonParametricDistribution();
  NonParametricDistribution(double bandwidth, Boolean decoy);
  NonParametricDistribution(double bandwidth, Boolean decoy, Boolean varBW);
  virtual ~NonParametricDistribution();

  virtual void init(double* prior);
  virtual void initUpdate(double* prior);

  virtual void addVal(double wt, double val);
  virtual void addVal(double wt, int val);

  virtual Boolean update(double wt);
  virtual Boolean update();

  virtual Boolean zeroProb(double val);
  virtual Boolean oneProb(double val);

  Boolean initWithDecoy(Array<Boolean>* isdecoy, Array<double>* vals);


  virtual double getProb(int val);
  virtual double getProb(double val);
  
  virtual double slice(double num, double left_val, double right_val);

  virtual void printDistr();
  virtual void writeDistr(FILE* fout);
  virtual Array<Tag*>* getSummaryTags(Boolean pos);

  virtual void setMean(double mean) {};
  virtual void setStdev(double stdev) {};
  

  Array<double>* kernel(Array<double>* vals, double point, double bw);
  Array<double>* densityFit(Array<double>* grid, Array<double>* vals, double bw);
  Array<double>* varBWdensityFit(Array<double>* grid, Array<double>* vals, Array<double>* bws);
  Array<double>* wtDensityFit(Array<double>* grid, Array<double>* vals, double bw, Array<double>* wts);
  Array<double>* wtDensityFit(Array<double>* grid, Array<double>* vals, double bw, Array<double>* wts, bool posprob);
  Array<double>* wtDensityFit(Array<double>* grid, Array<double>* vals, double bw, Array<double>* wts, Array<Array<double>*>* kerns, bool posprob);
  Array<double>* predictDensityFit(Array<double>* newgrid, Fit* fit);
  Array<double>* varBWwtDensityFit(Fit* fit, Array<double>* grid, Array<double>* vals, double bw, Array<double>* wts, bool posprob);
  Array<double>* varBWkernel(Fit* fit, Array<double>* vals, double point, double bw);
  Array<double>* varBWkernel(Fit* fit, Array<double>* vals, double point);
  Array<double>* varBWdensityFit(Array<double>* grid, Array<double>* vals, double bw);
  Array<double>* varBWdensityFit(Array<double>* grid, Array<double>* vals, double bw, Fit* fit);
  
  void initVarBWs(Fit* fit, Array<double>* grid, Array<double>* vals, double bw, Array<double>* probs, bool posprobs);
  void initVarBWs(Fit* fit, Array<double>* grid, Array<double>* vals, double bw);
  double predictDensityFit(double val, Fit* fit) ;
  void computeMeanStdev();
  double optimizeBandWidth(Array<double>* vals, Fit* fit);  
 protected:

  double bandwidth_;

  double pos_bw_;
  double neg_bw_;

  double myRound(double x, double toNearest);
  double calcFirstMoment(double x1, double x2, double y1, double y2);
  double calcSecondMoment(double x1, double x2, double y1, double y2);

  Fit* decoyFit_;
  Fit* forwardFit_;
  Array<double>* allDensities_;
  Array<double>* fwdDensities_;
  Array<double>* vals_;
  Array<double>* isdecoy_;
  Array<double>* wts_;
  Array<double>* grid_; 
  Array<double>* posvarbws_; 
  Array<double>* negvarbws_; 

  double pi_;
  Boolean decoy_;
  int numitrs_;
  Boolean varBW_;
};






# endif
