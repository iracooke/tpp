#include "GaussianDistribution.h"

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

GaussianDistribution::GaussianDistribution(double maxdiff) : ContinuousDistribution(maxdiff) {
}

Boolean GaussianDistribution::oneProb(double val) {
  return val > mean_ + stdev_;
}

void GaussianDistribution::init(double* prior) {
  ContinuousDistribution::init(NULL);
  mean_ = prior[0];
  stdev_ = prior[1];
}

void GaussianDistribution::initUpdate(double* prior) {
  if(newtot_ == NULL) {
    newtot_ = new double[1];
  }
  newtot_[0] = 0.0;
  newtotsq_ = 0.0;
  newtotwt_ = 0.0;
}


void GaussianDistribution::addVal(double wt, double val) {
  newtot_[0] += wt * val;
  newtotsq_ += wt * val * val;
  newtotwt_ += wt;
}


void GaussianDistribution::addVal(double wt, int val) { }

Boolean GaussianDistribution::update() {
  double newmean = newtot_[0]/newtotwt_;
  double newstdev = (newtotsq_ / newtotwt_) - newmean * newmean;
  newstdev = sqrt(newstdev);
  Boolean output = False;
  double delta = newmean - mean_;
  if(myfabs(delta) >= maxdiff_) {
    output = True;
  }
  if(use_stdev_)
    delta = newstdev - stdev_;
  if(myfabs(delta) >= maxdiff_) {
    output = True;
  }
  if(output) {
    mean_ = newmean;
    stdev_ = newstdev;
  }
  return output;
}


double GaussianDistribution::getProb(int val) { return 0.0; }

// add real thing here...
double GaussianDistribution::getGaussianProb(double val, double mean, double stdev) { 
  if(stdev == 0) {
    if(val == mean) {
      return 1.0;
    }
    else {
      return 0.0;
    }
  }
  double exponent = (-0.5) * (val - mean) * (val - mean) / (stdev * stdev);
  return ((exp (exponent)) / (stdev * sqrt(6.28318)));
}

// add real thing here...
double GaussianDistribution::getProb(double val) {
  return getGaussianProb(val, mean_, stdev_);
}

double GaussianDistribution::slice(double num, double left_val, double right_val) {
  return gaussianSlice(num, left_val, right_val, mean_, stdev_);
}
  
double GaussianDistribution::gaussianSlice(double num, double left_val, double right_val, double mean, double stdev) {
  if(stdev == 0) {
    if(mean >= left_val && mean <= right_val) {
      return num;
    }
    else {
      return 0.0;
    }
  }
  double left = N((left_val - mean_)/stdev_);
  double right = N((right_val - mean_)/stdev_);
  return num * (right - left);
}


// cumulative univariate normal distribution.
// This is a numerical approximation to the normal distribution.  
// See Abramowitz and Stegun: Handbook of Mathemathical functions
// for description.  The arguments to the functions are assumed 
// normalized to a (0,1 ) distribution. 

double GaussianDistribution::N(double z) {
    double b1 =  0.31938153; 
    double b2 = -0.356563782; 
    double b3 =  1.781477937;
    double b4 = -1.821255978;
    double b5 =  1.330274429; 
    double p  =  0.2316419; 
    double c2 =  0.3989423; 

    if (z >  6.0) { return 1.0; }; // this guards against overflow 
    if (z < -6.0) { return 0.0; };
    double a=fabs(z); 
    double t = 1.0/(1.0+a*p); 
    double b = c2*exp((-z)*(z/2.0)); 
    double n = ((((b5*t+b4)*t+b3)*t+b2)*t+b1)*t; 
    n = 1.0-b*n; 
    if ( z < 0.0 ) n = 1.0 - n; 
    return n; 
}



void GaussianDistribution::printDistr() {
  printf("(gaussian mean: %0.2f, stdev: %0.2f)\n", mean_, stdev_);

}
void GaussianDistribution::writeDistr(FILE* fout) {
  fprintf(fout, "(gaussian mean: %0.2f, stdev: %0.2f)\n", mean_, stdev_);

}

Array<Tag*>* GaussianDistribution::getSummaryTags(Boolean pos) {
  const char* tagnames[] = {"posmodel_distribution", "negmodel_distribution"};
  int index = pos ? 0 : 1;
  char next[500];
  Array<Tag*>* output = new Array<Tag*>;
  Tag* nexttag = new Tag(tagnames[index], True, False);
  nexttag->setAttributeValue("type", "gaussian");
  output->insertAtEnd(nexttag);
  nexttag = new Tag("parameter", True, True);
  nexttag->setAttributeValue("name", "mean");
  sprintf(next, "%0.2f", mean_);
  nexttag->setAttributeValue("value", next);
  output->insertAtEnd(nexttag);

  nexttag = new Tag("parameter", True, True);
  nexttag->setAttributeValue("name", "stdev");
  sprintf(next, "%0.2f", stdev_);
  nexttag->setAttributeValue("value", next);
  output->insertAtEnd(nexttag);

  output->insertAtEnd(new Tag(tagnames[index], False, True));
  return output;
}
