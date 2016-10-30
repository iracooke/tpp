#include "NonParametricDistribution.h"
#include <sstream>
#include <fstream>

// for isnan()
#ifdef _MSC_VER
#include "common/sysdepend.h"
#include <float.h>
#endif

/*

Program       : NonParametricDistribution for PeptideProphet                                                       
Author        : Henry Lam <hlam@systemsbiology.org>                                                       
Date          : 07.20.07 

Copyright (C) 2003 Andrew Keller

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
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


NonParametricDistribution::NonParametricDistribution() 
: wts_(0),
  vals_(0), 
  grid_(0), 
  allDensities_(0), 
  fwdDensities_(0), 
  isdecoy_(0), 
  decoyFit_(0), 
  forwardFit_(0), 
  posvarbws_(0), 
  negvarbws_(0) {
}

NonParametricDistribution::NonParametricDistribution(double bandwidth, Boolean decoy) 
: grid_(0), 
  allDensities_(0),
  fwdDensities_(0),
  isdecoy_(0),
  decoyFit_(0),
  forwardFit_(0),
  posvarbws_(0),
  negvarbws_(0) {
  bandwidth_ = (bandwidth <= 0) ? 0.081 : bandwidth;
  pos_bw_ = bandwidth_;
  neg_bw_ = bandwidth_;
  decoy_ = decoy;
  numitrs_ = 0;
  pi_ = 1;
  varBW_ = False;
  wts_ = new Array<double>();
  vals_ = new Array<double>();
}

NonParametricDistribution::NonParametricDistribution(double bandwidth, Boolean decoy, Boolean varBW) 
: grid_(0),
  allDensities_(0),
  fwdDensities_(0),
  isdecoy_(0),
  decoyFit_(0),
  forwardFit_(0),
  posvarbws_(0),
  negvarbws_(0) {
  bandwidth_ = (bandwidth <= 0) ? 0.081 : bandwidth;
  pos_bw_ = bandwidth_;
  neg_bw_ = bandwidth_;
  decoy_ = decoy;
  numitrs_ = 0;
  pi_ = 1;
  varBW_ = varBW;
  wts_ = new Array<double>();
  vals_ = new Array<double>();
}

NonParametricDistribution::~NonParametricDistribution() {
  if (decoyFit_ != 0)
    delete decoyFit_;
  if (forwardFit_ != 0)
    delete forwardFit_;
  if (allDensities_ != 0)
    delete allDensities_;
  if (fwdDensities_ != 0)
    delete fwdDensities_;
  if (vals_ != 0)
    delete vals_;
  if (isdecoy_ != 0)
    delete isdecoy_;
  if (wts_ != 0)
    delete wts_;
  if (grid_ != 0)
    delete grid_;
  if (posvarbws_ != 0)
    delete posvarbws_;
  if (negvarbws_ != 0)
    delete negvarbws_;
}

void NonParametricDistribution::init(double* prior) {
  ContinuousDistribution::init(NULL);
}

void NonParametricDistribution::initUpdate(double* prior) {
  ContinuousDistribution::init(NULL);
}

Boolean NonParametricDistribution::zeroProb(double val) {
  return val < mean_;
}

Boolean NonParametricDistribution::oneProb(double val) {
  return False;
}

void NonParametricDistribution::addVal(double wt, double val) {
  wts_->insertAtEnd(wt);
  vals_->insertAtEnd(val);
  newtotwt_ += wt;
}

void NonParametricDistribution::addVal(double wt, int val) {}

Boolean NonParametricDistribution::update() {
  numitrs_++;
  if (decoy_) {
    if (numitrs_ == 1) {
      computeMeanStdev();
    }
    // count_ = wts_->size();
    wts_->clear();
    vals_->clear();
    // Don't ever change!!!
    return False;
  }
  else {
    Array<double>* forwarddensity ;
    if (varBW_) {
      forwarddensity     = varBWwtDensityFit(forwardFit_, grid_, vals_, bandwidth_, wts_, true);
    }
    else {
      forwarddensity     = wtDensityFit(grid_, vals_, bandwidth_, wts_);
    }
    delete forwardFit_->dens_;
    forwardFit_->dens_ = forwarddensity;
    fwdDensities_ = predictDensityFit(vals_, forwardFit_);
    computeMeanStdev();
    //count_ = wts_->size();
    wts_->clear();
    vals_->clear();
    if (numitrs_ > 5) {
      return False;
    }
    return True;
  }  
}


Boolean NonParametricDistribution::update(double wt) {
  numitrs_++;
  if (decoy_) {
    if (numitrs_ == 1) {
      computeMeanStdev();
    }
    //count_ = wts_->size();
    wts_->clear();
    vals_->clear();
    pi_ = wt;
    // Don't ever change!!!
    return False;
  }
  else {
   Array<double>* forwarddensity ;
    if (varBW_) {
      forwarddensity     = varBWwtDensityFit(forwardFit_, grid_, vals_, bandwidth_, wts_, true);
    }
    else {
      forwarddensity     = wtDensityFit(grid_, vals_, bandwidth_, wts_);
    }

    delete forwardFit_->dens_;
    forwardFit_->dens_ = forwarddensity;
    delete fwdDensities_;
    fwdDensities_ = predictDensityFit(vals_, forwardFit_);
    computeMeanStdev();
    //count_ = wts_->size();
    wts_->clear();
    vals_->clear();
    pi_ = wt;
    if (numitrs_ > 5) {
      return False;
    }
    return True;
  }
  
}


Boolean NonParametricDistribution::initWithDecoy(Array<Boolean>* isdecoy, Array<double>* val) {
  double maxval=-9999;
  double minval=9999;
  Array<double>* decoyvals = new Array<double>();
  Array<double>* forwardvals = new Array<double>();
  for (int i=0; i<isdecoy->size(); i++) {
    if ((*val)[i]<minval) {
      minval = (*val)[i];
    }
    if ((*val)[i]>maxval) {
      maxval = (*val)[i];
    }
    if ((*isdecoy)[i]) {
      decoyvals->insertAtEnd((*val)[i]);
    }
    else {
      forwardvals->insertAtEnd((*val)[i]);
    }
  }

  minval -= 0.01;
  maxval += 0.01;

  int numgrids = 500;
  grid_ = new Array<double>(500);
  for (int i=0; i<numgrids; i++) {
    (*grid_)[i] = minval + i*(maxval - minval) / (numgrids-1);
  }

  if (decoy_) {
    decoyFit_ = new Fit(grid_, NULL);
    //neg_bw_ = optimizeBandWidth(decoyvals, decoyFit_);
    //neg_bw_ = 0.081;

    Array<double>* tnegdens =  densityFit(grid_, decoyvals, neg_bw_);
    
    if (decoyFit_->dens_ != NULL) {
      delete  decoyFit_->dens_;
    }
    
    decoyFit_->dens_ = tnegdens;

    if (varBW_) {
       Array<double>* negdens  = varBWdensityFit(grid_, decoyvals, neg_bw_);    
         
       if (decoyFit_->dens_ != NULL) {
	 delete  decoyFit_->dens_;
       }
       decoyFit_->dens_ = negdens;
    }

    allDensities_ = predictDensityFit(decoyvals, decoyFit_);    
  }
  else {
    forwardFit_ = new Fit(grid_, NULL);    
    //pos_bw_ = optimizeBandWidth(forwardvals, forwardFit_);
    //pos_bw_ = 0.081;
    Array<double>* tposdens =  densityFit(grid_, forwardvals, pos_bw_);
    
    if (forwardFit_->dens_ != NULL) {
      delete  forwardFit_->dens_;
    }
    
    forwardFit_->dens_ = tposdens;

    if (varBW_) {
       Array<double>* posdens  = varBWdensityFit(grid_, forwardvals, pos_bw_);    
         
       if (forwardFit_->dens_ != NULL) {
	 delete  forwardFit_->dens_;
       }
       forwardFit_->dens_ = posdens;
    }

    fwdDensities_ = predictDensityFit(forwardvals, forwardFit_);
  }

  //  vals_ = val;
  //  isdecoy_ = isdecoy;

  delete decoyvals;
  delete forwardvals;

  return True;

}


double NonParametricDistribution::myRound(double x, double toNearest) {

  if (x >= 0.0) {
    return ((int)(x / toNearest + 0.49) * toNearest);
  } else {
    return ((int)(x / toNearest - 0.49) * toNearest);
  }

}


double NonParametricDistribution::calcFirstMoment(double x1, double x2, double y1, double y2) {

  double m = (y2 - y1) / (x2 - x1);
  double x1sq = x1 * x1;
  double x2sq = x2 * x2;
  return (m * (x2 * x2sq - x1 * x1sq) / 3.0 + (y1 - m * x1) * (x2sq - x1sq) / 2.0);
}

double NonParametricDistribution::calcSecondMoment(double x1, double x2, double y1, double y2) {

  double m = (y2 - y1) / (x2 - x1);
  double x1cb = x1 * x1 * x1;
  double x2cb = x2 * x2 * x2;
  return (m * (x2 * x2cb - x1 * x1cb) / 4.0 + (y1 - m * x1) * (x2cb - x1cb) / 3.0);
}

double NonParametricDistribution::getProb(int val) { return 0.0; }

//double NonParametricDistribution::getTot() {
//double tot = newtotwt_;
  //  if (decoy_) {
  // return count_ - tot;
  //}
  //return tot;
//}

double NonParametricDistribution::getProb(double val) {
  double wt =  1; //(pi_ > 0) ?  pi_ : 1; 
  if (decoy_) {
    return pi_ * predictDensityFit(val, decoyFit_);
  }
  else {
    return pi_ * predictDensityFit(val, forwardFit_);
  }
}

//approximate integral
double NonParametricDistribution::slice(double num, double left_val, double right_val) {
  Fit* fit = forwardFit_;
  if (decoy_) {
    fit = decoyFit_;
  }
 
  // if (right_val - left_val < fit->gridwid_ * 1.5) {
  //  double left = predictDensityFit(left_val, fit);
  //  double right = predictDensityFit(right_val, fit);
  //  return (num * (right_val - left_val)*(left+right)/2);
  //}
  
  if (fit->gridwid_ <= 0) 
    return 0;

  double itr = left_val + fit->gridwid_;
  double sum = 0;
  while (itr <= right_val) {
    double dens = predictDensityFit(itr, fit);
    sum += dens*fit->gridwid_;
    itr += fit->gridwid_;
  }

  if (itr > right_val) {
    double dens = predictDensityFit(itr, fit);
    sum += dens*(itr-right_val);;
  }
  sum *= num;
  
  //cerr << "The Nonparametric distribution slice() function is not implemented.  Please alert a TPP developer!" << endl;
  return sum;
}

// double NonParametricDistribution::slice(double num, double left_val, double right_val) {
//   Fit* fit = forwardFit_;
//   if (decoy_) {
//     fit = decoyFit_;
//   }
//   double left = predictDensityFit(left_val, fit);
//   double right = predictDensityFit(right_val, fit);
//   return (num * (right_val - left_val)*(left+right)/2);
// }


void NonParametricDistribution::printDistr() {
  printf("(non-parametric mean: %0.2f, stdev: %0.2f)\n", mean_, stdev_);

}
void NonParametricDistribution::writeDistr(FILE* fout) {
  fprintf(fout, "(non-parametric mean: %0.2f, stdev: %0.2f)\n", mean_, stdev_);

}

Array<Tag*>* NonParametricDistribution::getSummaryTags(Boolean pos) {
  const char* tagnames[] = {"posmodel_distribution", "negmodel_distribution"};
  int index = pos ? 0 : 1;
//  char next[500];
  Array<Tag*>* output = new Array<Tag*>;
  Tag* nexttag = new Tag(tagnames[index], True, False);
  nexttag->setAttributeValue("type", "non-parametric");
  output->insertAtEnd(nexttag);

  nexttag = new Tag("parameter", True, True);
  nexttag->setAttributeValue("name", "mean");
  stringstream mss;
  mss << mean_;
  nexttag->setAttributeValue("value", mss.str().c_str());
  output->insertAtEnd(nexttag);

  nexttag = new Tag("parameter", True, True);
  nexttag->setAttributeValue("name", "stdev");
  stringstream sss;
  sss << stdev_;
  nexttag->setAttributeValue("value", sss.str().c_str());
  output->insertAtEnd(nexttag);

  output->insertAtEnd(new Tag(tagnames[index], False, True));
  return output;
}

Array<double>* NonParametricDistribution::kernel(Array<double>* vals, double point, double bw) {
  Array<double>* rtn = new Array<double>(vals->size());
  GaussianDistribution*  dnorm = new GaussianDistribution(0.1);
  for (int i=0; i<vals->size(); i++) {
    (*rtn)[i] = dnorm->getGaussianProb(((*vals)[i]-point)/bw, 0, 1) / bw;
    if (isnan((*rtn)[i])) {
	cerr << "ERROR: nan detected in NonParametricDistribution ..." << endl;
	exit(1);
	break;
    }
    
  }
  delete dnorm;
  return rtn;
}

Array<double>* NonParametricDistribution::varBWkernel(Fit* fit, Array<double>* vals, double point) {
  Array<double>* rtn = new Array<double>(vals->size());
  GaussianDistribution*  dnorm = new GaussianDistribution(0.1);
  
  for (int i=0; i<vals->size(); i++) {
    double val = (*vals)[i];
    int l = 0;
    int r = fit->grid_->size()-1;

    int j = (int)((val -(*fit->grid_)[l])/ fit->gridwid_);
    
    while(l <= r) {
      
      double a = (*fit->grid_)[j];
      double b = (*fit->grid_)[j+1];
      double aa = (*fit->varbws_)[j];
      double bb = (*fit->varbws_)[j+1];
      
      if ( (val >= a && val < b ) || j == 0 || j+1 == fit->grid_->size()-1) {
	double pred = aa + ( bb - aa ) /  (b-a) * (val-a) ;
	if (val == a) {
	  pred = aa;
	}
	if (isnan(pred)) {
	  cerr << "ERROR: NAN probability density detected.  Please alert the developer !!!" << endl;
	  exit(1);
	}
	//	set = true;
	break;
      }
      
      if (val > a) {
	l = j;
      }
      if (val < a) {
	r = j;
      }
      j = (r + l) / 2;
      
    }
    
    (*rtn)[i] = dnorm->getGaussianProb(((*vals)[i]-point)/(*fit->varbws_)[j], 0, 1) / (*fit->varbws_)[j];
  }
  delete dnorm;
  return rtn;
}

Array<double>* NonParametricDistribution::varBWkernel(Fit* fit, Array<double>* vals, double point, double bw ) {
  //Fit* fit;
  //if (decoy_) {
  //  fit = decoyFit_;
  //}
  //else {
  //  fit = forwardFit_;
  //}

  Array<double>* rtn = new Array<double>(vals->size());
  double fe = 0;
  double w = 0;
  for (int i=0; i<vals->size(); i++) {
    fe += log(predictDensityFit((*vals)[i], fit));
  }

  fe /= vals->size();

  fe = exp(fe);

  GaussianDistribution*  dnorm = new GaussianDistribution(0.1);
  for (int i=0; i<vals->size(); i++) {
    w = sqrt(fe / (predictDensityFit((*vals)[i], fit)));       
    (*rtn)[i] = dnorm->getGaussianProb(((*vals)[i]-point)/(w*bw), 0, 1) / (w*bw);
  }

  delete dnorm;
  return rtn;
}



Array<double>* NonParametricDistribution::densityFit(Array<double>* grid, Array<double>* vals, double bw) {
  int len = grid->size();
  Array<double>* d = new Array<double>(len);
  int i = 0;
  for (i=0; i<len; i++) {
    (*d)[i] = 0;
    Array<double>* k = kernel(vals, (*grid)[i], bw) ;
    for (int j=0; j<k->size(); j++) {
      (*d)[i] += (*k)[j];
    }
    if (k->size() > 0)
      (*d)[i] = (*d)[i] / k->size();
    delete k;
  }
  
  return d;
}

Array<double>* NonParametricDistribution::varBWdensityFit(Array<double>* grid, Array<double>* vals, double bw) {
  Fit * fit;
  if (decoy_) {
    fit = decoyFit_;
  }
  else {
    fit = forwardFit_;
  }
  int len = grid->size();
  Array<double>* d = new Array<double>(len);
  int i = 0;
  for (i=0; i<len; i++) {
    (*d)[i] = 0;
    //Array<double>* k = varBWkernel(fit, vals, (*grid)[i], bw) ;
    Array<double>* k = varBWkernel(fit, vals, (*grid)[i]) ;
    for (int j=0; j<k->size(); j++) {
      (*d)[i] += (*k)[j];
    }
    if (k->size() > 0)
      (*d)[i] = (*d)[i] / k->size();
    delete k;
  }
  
  return d;
}

Array<double>* NonParametricDistribution::varBWdensityFit(Array<double>* grid, Array<double>* vals, double bw, Fit* fit) {
  int len = grid->size();
  Array<double>* d = new Array<double>(len);
  int i = 0;
  for (i=0; i<len; i++) {
    (*d)[i] = 0;
    //Array<double>* k = varBWkernel(fit, vals, (*grid)[i], bw) ;
    Array<double>* k = varBWkernel(fit, vals, (*grid)[i]) ;
    for (int j=0; j<k->size(); j++) {
      (*d)[i] += (*k)[j];
    }
    if (k->size() > 0)
      (*d)[i] = (*d)[i] / k->size();
    delete k;
  }
  
  return d;
}


Array<double>* NonParametricDistribution::wtDensityFit(Array<double>* grid, Array<double>* vals, double bw, Array<double>* wts) {
  return wtDensityFit(grid, vals, bw, wts, true);
}

Array<double>* NonParametricDistribution::wtDensityFit(Array<double>* grid, Array<double>* vals, double bw, Array<double>* wts, bool posprob) {
  int len = grid->size();
  Array<double>* d = new Array<double>(len);
  double wtsum = 0;
  double wt;
  int i;
  for (i=0; i<len; i++) {
    (*d)[i] = 0;
    wtsum = 0;
    Array<double>* k = kernel(vals, (*grid)[i], bw) ;
    for (int j=0; j<k->size(); j++) {
      if (posprob) {
	wt = (*wts)[j];
      }
      else {
	wt = 1-(*wts)[j];
      }
      (*d)[i] += (*k)[j] * wt;
      wtsum += wt;
      if (isnan((*d)[i])) {
		cerr << "ERROR: nan detected in NonParametricDistribution ..." << endl;
		exit(1);
		delete k;
		return d;
      }
    }
    if ( wtsum > 0 ) {
      (*d)[i] = (*d)[i] / wtsum;
    }
    delete k;
  }
  
  return d;
}
Array<double>* NonParametricDistribution::varBWwtDensityFit(Fit* fit,  Array<double>* grid, Array<double>* vals, double bw, Array<double>* wts, bool posprob) {
  int len = grid->size();
  Array<double>* d = new Array<double>(len);
  double wtsum = 0;
  double wt;
  int i;
  for (i=0; i<len; i++) {
    (*d)[i] = 0;
    wtsum = 0;
    //Array<double>* k = varBWkernel(fit, vals, (*grid)[i], bw) ;
    Array<double>* k = varBWkernel(fit, vals, (*grid)[i]) ;
    for (int j=0; j<k->size(); j++) {
      if (posprob) {
	wt = (*wts)[j];
      }
      else {
	wt = 1-(*wts)[j];
      }
       
      (*d)[i] += (*k)[j] * wt;
      wtsum += wt;
    }
    if ( wtsum > 0 ) {
      (*d)[i] = (*d)[i] / wtsum;
    }
    delete k;
  }
  
  return d;
}

Array<double>* NonParametricDistribution::wtDensityFit(Array<double>* grid, Array<double>* vals, double bw, Array<double>* wts, Array<Array<double>*>* kerns, bool posprob) {
  int len = grid->size();
  Array<double>* d = new Array<double>(len);
  double wtsum = 0;
  double wt;
  int i;
  for (i=0; i<len; i++) {
    (*d)[i] = 0;
    wtsum = 0;
    Array<double>* k = (*kerns)[i];
    
    for (int j=0; j<k->size(); j++) {
      if (posprob) {
	wt = (*wts)[j];
      }
      else {
	wt = 1-(*wts)[j];
      }
      
      (*d)[i] += (*k)[j] * wt;
      wtsum += wt;
    }
    if (wtsum > 0)
      (*d)[i] = (*d)[i] / wtsum;

  }
  
  return d;
}

Array<double>* NonParametricDistribution::predictDensityFit(Array<double>* newgrid, Fit* fit) {
  int len = newgrid->size();
  Array<double>* pred = new Array<double>(len);

  for (int j=0; j < newgrid->size(); j++) {
    (*pred)[j] = 0;
  }
  for (int i=0; i < fit->grid_->size()-1; i++) {
    double a = (*fit->grid_)[i];
    double b = (*fit->grid_)[i+1];
    double aa = (*fit->dens_)[i];
    double bb = (*fit->dens_)[i+1];
    
    for (int j=0; j < newgrid->size(); j++) {
      if ((*newgrid)[j] >= a && (*newgrid)[j] < b) {
	(*pred)[j] = aa + ( bb - aa ) /  (b-a) * ((*newgrid)[j]-a) ;
      }
    }
  }
  return pred;

}

double NonParametricDistribution::predictDensityFit(double val, Fit* fit) {
  double pred = 0;
  bool set = false;
  if (val < (*fit->grid_)[0] || val > (*fit->grid_)[fit->grid_->size()-1]) {
    return pred;
  }



  //  for (int i=0; i < fit->grid_->size()-1; i++) {

  int l = 0;
  int r = fit->grid_->size()-1;

  
  
  int i = (int)((val -(*fit->grid_)[l])/ fit->gridwid_);
  
  while(l <= r) {
    
    double a = (*fit->grid_)[i];
    int idx = (i+1) >= fit->grid_->size() ? fit->grid_->size()-1 : i+1;
    double b = (*fit->grid_)[idx];
    double aa = (*fit->dens_)[i];
    idx = i+1 >= fit->dens_->size() ? fit->dens_->size()-1 : i+1;
    double bb = (*fit->dens_)[idx];
    
   if ( (val >= a && val < b) || i == 0 || i+1 == fit->grid_->size()-1) {
      pred = aa + ( bb - aa ) /  (b-a) * (val-a) ;
      if (val == a) {
	pred = aa;
      }
      if (isnan(pred)) {
		cerr << "ERROR: NAN probability density detected.  Please alert the developer !!!" << endl;
		exit(1);
	  }
      set = true;
      break;
    }
    
    if (val > a) {
      l = i;
    }
    if (val < a) {
      r = i;
    }
    i = (r + l) / 2;

  }
    
  

  if (!set) {
    pred = (*fit->dens_)[fit->grid_->size()-1];
  }
  return pred;

}

void NonParametricDistribution::computeMeanStdev() {
  Fit* fit;
  if (decoy_) {
    fit = decoyFit_;
  }
  else {
    fit = forwardFit_;
  }

  double pred = 0;
  double tot = 0;
  stdev_ = 0;
  mean_ = 0;

  for (int i =0; i<vals_->size(); i++) {
    //    pred = predictDensityFit((*vals_)[i], fit);
    pred = (*wts_)[i];
    if (pred > 0) {
      mean_ += pred * (*vals_)[i];
      tot += pred;
      stdev_ += pred * (*vals_)[i]  * (*vals_)[i];
    }
  }
  if (tot > 0) {
    mean_ /= tot;
    stdev_ = sqrt( stdev_/tot - mean_*mean_);
  }
}

void NonParametricDistribution::initVarBWs(Fit* fit, Array<double>* grid, Array<double>* vals, double bw, Array<double>* probs, bool posprobs) {
  int len = grid->size();

  Array<double>* varbws = new Array<double>(len);
  if (fit->varbws_ != NULL)
    delete fit->varbws_;

  fit->varbws_ = varbws;

  double fe = 0;
  double w = 0;
  double probsum=0;
  for (int i=0; i<vals->size(); i++) {
    //    double prob = posprobs ? (*probs)[i] : 1 - (*probs)[i];
    //    probsum += prob;
    //    fe += prob*log(predictDensityFit((*vals)[i], fit));
    fe += log(predictDensityFit((*vals)[i], fit));
  }

  //  fe /= vals->size();
  fe /= vals->size();

  fe = exp(fe);

  //double pi_w = 0;
  //for (int i=0; i<vals->size(); i++) {
  //  double prob = posprobs ? (*probs)[i] : 1 - (*probs)[i];
  //  pi_w += prob*log(sqrt(fe / (predictDensityFit((*vals)[i], fit))));
  // }
  //pi_w = exp(pi_w);
  
  // GaussianDistribution*  dnorm = new GaussianDistribution(0.1);

  for (int i=0; i<grid->size(); i++) {
    if (predictDensityFit((*grid)[i], fit) <= 0) {
      w = 1;
    }
    else {
      w = sqrt(fe / (predictDensityFit((*grid)[i], fit)));
    }
    (*fit->varbws_)[i] =  w*bw;
  }

}


void NonParametricDistribution::initVarBWs(Fit* fit, Array<double>* grid, Array<double>* vals, double bw) {
  int len = grid->size();

  Array<double>* varbws = new Array<double>(len);
  if (fit->varbws_ != NULL)
    delete fit->varbws_;

  fit->varbws_ = varbws;

  double fe = 0;
  double w = 0;
  double probsum=0;
  for (int i=0; i<vals->size(); i++) {

    fe += log(predictDensityFit((*vals)[i], fit));
  }

  //  fe /= vals->size();
  fe /= vals->size();

  fe = exp(fe);



  for (int i=0; i<grid->size(); i++) {
    if (predictDensityFit((*grid)[i], fit) <= 0) {
      w = 1;
    }
    else {
      w = sqrt(fe / (predictDensityFit((*grid)[i], fit)));
    }
    (*fit->varbws_)[i] =  w*bw;
  }

}

double NonParametricDistribution::optimizeBandWidth(Array<double>* vals, Fit* fit) {
  double stddev = 0;
  double mean = 0;
  double bw_out = 0.1;
  


  for (int i=0; i<vals->size(); i++) {
    stddev += (*vals)[i]* (*vals)[i];
    mean += (*vals)[i];
    
  }

  if (stddev == 0 || mean == 0 || vals == NULL || vals->size() == 0) {
    return bw_out;
  }

  stddev /= vals->size();
  mean /= vals->size();
  stddev = sqrt(stddev - mean*mean);

  
  Array<double>* dens;
  Array<double>* tmpdens;
 

  //Optimize bandwidth
  double bw = 1;
  double best_tot = 0;
  double tot = 0;
  double count = 1;
  while (count <= 50) {
    tot = 0;
    bw = count / 50 * stddev;


    tmpdens = densityFit(fit->grid_, vals, bw);
    fit->dens_ = tmpdens; 
    dens = predictDensityFit(vals, fit);

    for (int i=0; i < dens->size(); i++) {
      tot += (*dens)[i];
    }

    if (tot > best_tot) {
      best_tot = tot;
      bw_out = bw;
    }

    delete dens;
    delete tmpdens;

    count ++;
  }
  fit->dens_ = NULL;
  return bw_out;
  // End bw optimization
}


