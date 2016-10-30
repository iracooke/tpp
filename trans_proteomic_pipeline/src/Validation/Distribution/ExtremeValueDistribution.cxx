#include "ExtremeValueDistribution.h"

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

ExtremeValueDistribution::ExtremeValueDistribution(double maxdiff) : ContinuousDistribution(maxdiff) {
  minval_ = NULL;
  mean_ = 0.0;
  stdev_ = 0.0;
}

void ExtremeValueDistribution::init(double* prior) {
  ContinuousDistribution::init(NULL);
  mean_ = prior[0];
  stdev_ = prior[1];
  computeMoments(mean_, stdev_);
}

void ExtremeValueDistribution::initUpdate(double* prior) {
  if(newtot_ == NULL) {
    newtot_ = new double[1];
  }
  newtot_[0] = 0.0;
  newtotsq_ = 0.0;
  newtotwt_ = 0.0;
}

void ExtremeValueDistribution::addVal(double wt, double val) {
  //if(! aboveMin(val)) {
  //  return;
  //}
  newtot_[0] += wt * val;
  newtotsq_ += wt * val * val;
  newtotwt_ += wt;
}

void ExtremeValueDistribution::addVal(double wt, int val) { }

Boolean ExtremeValueDistribution::update() {
  double newmean = newtot_[0]/newtotwt_;
  double newstdev = (newtotsq_ / newtotwt_) - newmean * newmean;
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
    computeMoments(mean_, stdev_);
  }

  return output;
}

double ExtremeValueDistribution::getProb(int val) { return 0.0; }

double ExtremeValueDistribution::getExtremeValueProb(double val, double mu, double beta) { 
  if(! aboveMin(val))
    return 0.0;


  if(beta == 0.0) {
    if(val == mu) {
      return 1.0;
    }
    else {
      return 0.0;
    }
  }
  double negexponent = exp(-(val - mu)/beta);
  return negexponent * exp(-negexponent) / beta;
}

double ExtremeValueDistribution::getProb(double val) {
  return getExtremeValueProb(val, mu_, beta_);
}

void ExtremeValueDistribution::computeMoments(double mean, double std) {
  beta_ = 0.779697  * std;
  mu_ = mean - 0.5772 * beta_;
}


double ExtremeValueDistribution::cumulative(double val, double mu, double beta) {
  if(beta == 0.0)
    return 1.0;
  double negexponent = exp(-(val - mu)/beta);
  return exp(-negexponent);

}

Boolean ExtremeValueDistribution::aboveMin(double val) {
  return (minval_ == NULL || val >= minval_[0]);
}

void ExtremeValueDistribution::setDistrMinval(double val) {
  //cerr << "setting minval to " << val << endl;
  if (minval_ == NULL) {
    minval_ = new double[1];
    minval_[0] = val;
  }
  else {
    minval_[0] = minval_[0] + 0.5;
  }

}



double ExtremeValueDistribution::extremeValueSlice(double num, double left_val, double right_val, double mu, double beta) {
  double diff = cumulative(right_val, mu, beta) - cumulative(left_val, mu, beta);
  return num * diff;
}

double ExtremeValueDistribution::slice(double num, double left_val, double right_val) {
  return extremeValueSlice(num, left_val, right_val, mu_, beta_);
}

void ExtremeValueDistribution::printDistr() {
  printf("(evd mean: %0.2f, stdev: %0.2f, mu: %0.2f, beta: %0.2f)\n", mean_, stdev_, mu_, beta_);

}

void ExtremeValueDistribution::writeDistr(FILE* fout) {
  if(minval_ == NULL)
    fprintf(fout, "(evd mean: %0.2f, stdev: %0.2f, mu: %0.2f, beta: %0.2f)\n", mean_, stdev_, mu_, beta_);
  else
    fprintf(fout, "(evd mean: %0.2f, stdev: %0.2f, mu: %0.2f, beta: %0.2f, minval: %0.2f)\n", mean_, stdev_, mu_, beta_, minval_[0]);

}

Array<Tag*>* ExtremeValueDistribution::getSummaryTags(Boolean pos) {
  const char* tagnames[] = {"posmodel_distribution", "negmodel_distribution"};
  int index = pos ? 0 : 1;
  char next[500];
  Array<Tag*>* output = new Array<Tag*>;
  Tag* nexttag = new Tag(tagnames[index], True, False);
  nexttag->setAttributeValue("type", "evd");
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
  nexttag->setAttributeValue("name", "mu");
  sprintf(next, "%0.2f", mu_);
  nexttag->setAttributeValue("value", next);
  output->insertAtEnd(nexttag);

  nexttag = new Tag("parameter", True, True);
  nexttag->setAttributeValue("name", "beta");
  sprintf(next, "%0.2f", beta_);
  nexttag->setAttributeValue("value", next);
  output->insertAtEnd(nexttag);

  output->insertAtEnd(new Tag(tagnames[index], False, True));
  return output;
}
