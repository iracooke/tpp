#include "GammaDistribution.h"

/*

Program       : GammaDistribution for PeptideProphet                                                       
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


GammaDistribution::GammaDistribution(double maxdiff) : ContinuousDistribution(maxdiff) {
  minval_ = NULL;
  mean_ = 0.0;
  stdev_ = 0.0;
  zero_ = 0.0;
  invscale_ = 0.0;
  shape_ = 0.0;
}

GammaDistribution::~GammaDistribution() {
  if(minval_ != NULL) {
    delete minval_;
  }
}

void GammaDistribution::init(double* prior, Boolean alphabeta) {
  ContinuousDistribution::init(NULL);
  if(prior != NULL) {
    zero_ = prior[2];
    if (alphabeta) {
      invscale_ = prior[0];
      shape_ = prior[1];
      mean_ = shape_ / invscale_;
      stdev_ = sqrt(mean_ / invscale_);
    }
    else {
      mean_ = prior[0];
      stdev_ = prior[1];
      computeAlphaBeta();
    }
  }
}

double GammaDistribution::getZero() { return zero_; }

Boolean GammaDistribution::zeroProb(double val) {
  return (getProb(val) < 0.01 && val < (1 / invscale_) * (shape_ - 1.0) + zero_); //DDS ???
}


void GammaDistribution::computeAlphaBeta() {
  double maxbeta = 125.0;

  if (stdev_ == 0) {
    invscale_ = 999.0;
    shape_ = 999.0;
  }
  else {
    invscale_ = mean_ / (stdev_ * stdev_);
    shape_ = (mean_ * mean_) / (stdev_ * stdev_);
  }

  if (shape_ > maxbeta) {
    shape_ = maxbeta;
    invscale_ = shape_ / mean_;
  }
}


void GammaDistribution::initUpdate(double* prior) {
  if(newtot_ == NULL) {
    newtot_ = new double[1];
  }
  newtot_[0] = 0.0;
  newtotsq_ = 0.0;
  newtotwt_ = 0.0;
}

void GammaDistribution::addVal(double wt, double val) {
  if(val < zero_ || ! aboveMin(val)) {
    return;
  }
  double adjval = val - zero_;
  newtot_[0] += wt * adjval;
  newtotsq_ += wt * adjval * adjval;
  newtotwt_ += wt;
}

void GammaDistribution::addVal(double wt, int val) { }

Boolean GammaDistribution::update() {
  if(newtotwt_ == 0.0)
    return False;
  double newmean = newtot_[0]/newtotwt_;
  double newstdev = newtotsq_/newtotwt_ - newmean * newmean;
  newstdev = sqrt(newstdev);
  Boolean output = False;
  double delta = newmean - mean_;
  if(myfabs(delta) >= maxdiff_) {
    output = True;
  }
  delta = newstdev - stdev_;
  if(myfabs(delta) >= maxdiff_) {
    output = True;
  }
  if(output) {
    mean_ = newmean;
    stdev_ = newstdev;
    computeAlphaBeta();
  }
  return output;
}

double GammaDistribution::getProb(int val) { return 0.0; }

double GammaDistribution::getGammaProb(double val, double invscale, double shape, double zero) {
  double prob = 0;
  double gamm;
  double helper;
  double value = val - zero;
  double loggamma;
  double first;
  if(shape <= 0 || invscale <= 0) {
    return 0.0;
  }
  if(value > 0 && invscale > 0 && aboveMin(val)) {
    loggamma = gammln(shape);
    helper = ((shape - 1) * log(value)) + (shape * log(invscale)) - loggamma - invscale * value;
   

    //helper = exp(helper) * exp(- value / invscale);
 
    //helper = exp((shape - 1) * log(value) - (value / invscale));

    prob = exp(helper);


    gamm = exp(loggamma);
    if(gamm == 0.0) {
      return 1.0;
    }

    //in log space
    helper = shape*log(invscale) + loggamma;
    helper = exp(helper);
    
    first =  helper;
    if(first == 0.0) {
      return 0.0;
    }
    //prob /= first;
  }
  return (double)prob;
}


// add real thing here...
double GammaDistribution::getProb(double val) { 
  return getGammaProb(val, invscale_, shape_, zero_);
}



double GammaDistribution::gammln(double xx) {
  double x,y,tmp,ser;
  static double cof[6]={76.18009172947146,-86.50532032941677,
			24.01409824083091,-1.231739572450155,
			0.1208650973866179e-2,-0.5395239384953e-5};
        int j;

        y=x=xx;
        tmp=x+5.5;
        tmp -= (x+0.5)*log(tmp);
        ser=1.000000000190015;
        for (j=0;j<=5;j++) ser += cof[j]/++y;
        return -tmp+log(2.5066282746310005*ser/x);
}

void GammaDistribution::setDistrMinval(double val) {
  if (minval_ == NULL) {
    minval_ = new double[1];
    minval_[0] = val;
  }
  else {
    minval_[0] = minval_[0] + 0.5;
  }
}

Boolean GammaDistribution::aboveMin(double val) {
  return (minval_ == NULL || val >= minval_[0]);
}

double GammaDistribution::slice(double num, double left_val, double right_val) {
  return gammaSlice(num, left_val, right_val, mean_, stdev_);
}

double GammaDistribution::gammaSlice(double num, double left_val, double right_val, double m1, double m2) {
  if(m2 == 0) {
    if(m2 >= left_val && m2 <= right_val) {
      return num;
    }
    else {
      return 0.0;
    }
  }
  int num_windows = 20;
  double window_width = (right_val - left_val) / num_windows;
  double next;
  double tot = 0.0;
  for(int k = 0; k <= num_windows; k++) {
    next = getProb((k * window_width) + left_val);
    if(k == 0 || k == num_windows) {
      tot += next / 2;
    }
    else {
      tot += next;
    }
  }
  return (tot * window_width * num);

}


void GammaDistribution::printDistr() {
  printf("(gamma m1: %0.2f, m2: %0.2f, invscale: %0.2f, shape: %0.2f, zero: %0.2f)\n", mean_, stdev_, invscale_, shape_, zero_);
}
void GammaDistribution::writeDistr(FILE* fout) {
  if(minval_ == NULL)
    fprintf(fout, "(gamma m1: %0.2f, m2: %0.2f, invscale: %0.2f, shape: %0.2f, zero: %0.2f)\n", mean_, stdev_, invscale_, shape_, zero_);
  else
    fprintf(fout, "(gamma m1: %0.2f, m2: %0.2f, invscale: %0.2f, shape: %0.2f, zero: %0.2f, minval: %0.2f)\n", mean_, stdev_, invscale_, shape_, zero_, minval_[0]);
}

Array<Tag*>* GammaDistribution::getSummaryTags(Boolean pos) {
  const char* tagnames[] = {"posmodel_distribution", "negmodel_distribution"};
  int index = pos ? 0 : 1;
  char next[500];
  Array<Tag*>* output = new Array<Tag*>;
  Tag* nexttag = new Tag(tagnames[index], True, False);
  nexttag->setAttributeValue("type", "gamma");
  output->insertAtEnd(nexttag);
  nexttag = new Tag("parameter", True, True);
  nexttag->setAttributeValue("name", "m1");
  sprintf(next, "%0.2f", mean_);
  nexttag->setAttributeValue("value", next);
  output->insertAtEnd(nexttag);

  nexttag = new Tag("parameter", True, True);
  nexttag->setAttributeValue("name", "m2");
  sprintf(next, "%0.2f", stdev_);
  nexttag->setAttributeValue("value", next);
  output->insertAtEnd(nexttag);
  
  nexttag = new Tag("parameter", True, True);
  nexttag->setAttributeValue("name", "invscale");
  sprintf(next, "%0.2f", invscale_);
  nexttag->setAttributeValue("value", next);
  output->insertAtEnd(nexttag);

  nexttag = new Tag("parameter", True, True);
  nexttag->setAttributeValue("name", "shape");
  sprintf(next, "%0.2f", shape_);
  nexttag->setAttributeValue("value", next);
  output->insertAtEnd(nexttag);

  nexttag = new Tag("parameter", True, True);
  nexttag->setAttributeValue("name", "zero");
  sprintf(next, "%0.2f", zero_);
  nexttag->setAttributeValue("value", next);
  output->insertAtEnd(nexttag);

  if(minval_ != NULL) {
    nexttag = new Tag("parameter", True, True);
    nexttag->setAttributeValue("name", "minval");
    sprintf(next, "%0.2f", minval_[0]);
    nexttag->setAttributeValue("value", next);
    output->insertAtEnd(nexttag);
  }
  output->insertAtEnd(new Tag(tagnames[index], False, True));
  return output;
}
