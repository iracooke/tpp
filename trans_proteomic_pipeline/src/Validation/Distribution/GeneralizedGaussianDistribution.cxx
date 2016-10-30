#include "GeneralizedGaussianDistribution.h"
#include <cmath>

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

GeneralizedGaussianDistribution::GeneralizedGaussianDistribution(double maxdiff) : ContinuousDistribution(maxdiff) {

  allvals_.clear();
  allwts_.clear();
}

Boolean GeneralizedGaussianDistribution::oneProb(double val) {
  return val > mean_ + stdev_;
}

void GeneralizedGaussianDistribution::init(double* prior) {
  ContinuousDistribution::init(NULL);
  mean_ = prior[0];
  stdev_ = prior[1];
  shape_ = 2.0; // default to ordinary Gaussian

  scalingFactor_ = stdev_ * 1.4142; // sqrt(2) = 1.4142  
  normalizationFactor_ = stdev_ * 2.5066; // sqrt(2 * pi) = 2.5066

}

void GeneralizedGaussianDistribution::initUpdate(double* prior) {
  if(newtot_ == NULL) {
    newtot_ = new double[1];
  }
  newtot_[0] = 0.0;
  newtotsq_ = 0.0;
  newtotwt_ = 0.0;
}


void GeneralizedGaussianDistribution::addVal(double wt, double val) {
  newtot_[0] += wt * val;
  newtotsq_ += wt * val * val;
  newtotwt_ += wt;

  allwts_.push_back(wt);
  allvals_.push_back(val);
}


void GeneralizedGaussianDistribution::addVal(double wt, int val) { }

Boolean GeneralizedGaussianDistribution::update() {
  double newmean = newtot_[0] / newtotwt_;
  double newm2 = newtotsq_ / newtotwt_;
  double newvar = newm2 - newmean * newmean;
  double newstdev = sqrt(newvar);
  
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

    double sumAbsDiff = 0.0;
    for (int i = 0; i < allvals_.size() && i < allwts_.size(); i++) {
      sumAbsDiff += allwts_[i] * fabs(allvals_[i] - mean_);
    }
    sumAbsDiff /= newtotwt_;

    // cerr << "SIZE=" << allvals_.size() << ",m1=" << sumAbsDiff << ",var=" << newvar << endl;

    shape_ = computeShape(sumAbsDiff * sumAbsDiff / newvar);
  }

  allvals_.clear();
  allwts_.clear();

  return output;
}

double GeneralizedGaussianDistribution::computeShape(double M) {

  // recipe from http://www.cimat.mx/reportes/enlinea/I-01-18_eng.pdf
 
  if (M >= 0.75) {
    shape_ = 7.0;
  
  } else if (M >= 0.671256) {
    shape_ = 14.7780339 * (0.6723532 - sqrt(0.4520588 + 0.135336 * log((3.0 - 4.0 * M) / 1.4620628)));
  
  } else if (M >= 0.448994) {
    double tmp = 0.9694429 - 0.8727534 * M;
    shape_ = 6.8019585 / M * (tmp - sqrt(tmp * tmp - 0.294033 * M * M));
  
  } else if (M >= 0.131246) {
    shape_ = -0.9333454 * (-1.1689399 + sqrt(1.041527 - 2.1428294 * M));
    
  } else {
    shape_ = 1.0464963 / log(0.75 / (M * M));
  
  }
  
  scalingFactor_ = stdev_ * sqrt(tgamma(1.0 / shape_) / tgamma(3.0 / shape_));
  
  normalizationFactor_ = 2.0 * tgamma(1.0 + 1.0 / shape_) * scalingFactor_;
 
  //  cerr << "M=" << M << ",shape=" << shape_ << ",scalingFactor=" << scalingFactor_ << ",normalizationFactor=" << normalizationFactor_ << endl;

  return (shape_);
 
}

double GeneralizedGaussianDistribution::getProb(int val) { return 0.0; }

double GeneralizedGaussianDistribution::getGeneralizedGaussianProb(double val) {   
  double exponent = -pow(fabs(val - mean_) / scalingFactor_, shape_);

  //  cerr << "val=" << val << ",exponent=" << exponent << ",mean=" << mean_ << ",scaling=" << scalingFactor_ << ",shape=" << shape_ << ",P=" << exp(exponent) / normalizationFactor_ << endl;

  return ((exp(exponent)) / normalizationFactor_);
}

double GeneralizedGaussianDistribution::getProb(double val) {
  return getGeneralizedGaussianProb(val);
}

double GeneralizedGaussianDistribution::slice(double num, double left_val, double right_val) {
  return generalizedGaussianSlice(num, left_val, right_val);
}
  
double GeneralizedGaussianDistribution::generalizedGaussianSlice(double num, double left_val, double right_val) {
  if(stdev_ == 0) {
    if(mean_ >= left_val && mean_ <= right_val) {
      return num;
    }
    else {
      return 0.0;
    }
  }
  if (right_val < left_val) return (0.0);
  
  double step = (right_val - left_val) / 20.0;
  double lastPdf = getGeneralizedGaussianProb(left_val);
  double cdf = 0.0;
  
  for (double v = left_val + step; v <= right_val; v += step) {
    double pdf = getGeneralizedGaussianProb(v);
    cdf += 0.5 * (lastPdf + pdf) * step; // trapezoid integration
    lastPdf = pdf;
  }
  
  return num * cdf;
}


void GeneralizedGaussianDistribution::printDistr() {
  printf("(generalized gaussian mean: %0.2f, stdev: %0.2f, shape: %0.2f)\n", mean_, stdev_, shape_);

}
void GeneralizedGaussianDistribution::writeDistr(FILE* fout) {
  fprintf(fout, "(generalized gaussian mean: %0.2f, stdev: %0.2f, shape: %0.2f)\n", mean_, stdev_, shape_);

}

Array<Tag*>* GeneralizedGaussianDistribution::getSummaryTags(Boolean pos) {
  const char* tagnames[] = {"posmodel_distribution", "negmodel_distribution"};
  int index = pos ? 0 : 1;
  char next[500];
  Array<Tag*>* output = new Array<Tag*>;
  Tag* nexttag = new Tag(tagnames[index], True, False);
  nexttag->setAttributeValue("type", "generalized_gaussian");
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

  nexttag = new Tag("parameter", True, True);
  nexttag->setAttributeValue("name", "shape");
  sprintf(next, "%0.2f", shape_);
  nexttag->setAttributeValue("value", next);
  output->insertAtEnd(nexttag);

  output->insertAtEnd(new Tag(tagnames[index], False, True));
  return output;
}
