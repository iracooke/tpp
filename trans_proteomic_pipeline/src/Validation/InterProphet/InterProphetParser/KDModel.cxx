/*

Program       : InterProphet                                                       
Author        : David Shteynberg <dshteynb  AT systemsbiology.org>                                                       
Date          : 12.12.07

Primary data object holding all mixture distributions for each precursor ion charge

Copyright (C) 2007 David Shteynberg

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

David Shteynberg
Institute for Systems Biology
401 Terry Avenue North 
Seattle, WA  98109  USA
akeller@systemsbiology.org

*/

#include "KDModel.h"

KDModel::KDModel(const char* name, int numgrids) 
: obs_kerns_(0) {
  posvarBW_ = false;
  negvarBW_ = false;
  numBreaks_ = 20;
  posnumBreaks_ = 20;
  negnumBreaks_ = 20;
  isready_ = false;
  postot_ = 0;
  negtot_ = 0;

  posprobs_ = new Array<double>();
  negprobs_ = NULL;//new Array<double>();
  vals_ = new Array<double>();
  zvals_ = NULL;//new Array<double>();
  ovals_ = NULL;//new Array<double>();

  pos_kerns_ = NULL;//new Array<Array<double>*>();
  neg_kerns_ = NULL;//new Array<Array<double>*>();
  
  highobsfit_ = NULL;
  lowobsfit_ = NULL;

  highvals_ = NULL;
  lowvals_ = NULL;
  posfit_ = NULL;
  negfit_ = NULL;

  name_ = name;

  d_ = new NonParametricDistribution();

  numgrids_ = numgrids;
  bw_ = 0.1;
  pos_bw_ = 0.1;
  neg_bw_ = 0.1;
  
  bw_opt_count_ = 0;
  grid_ = NULL;
  
}

bool KDModel::update(Array<double>* probs, int min_bw_opts, double posmin, double posmax) {
  
  double oldtot = postot_; 
  bool output = false;
  
  if (zvals_ != NULL) 
    zvals_->clear();
  if (ovals_ != NULL) 
    ovals_->clear();

  for (int i =0; i < probs->size(); i++) {
    if ((*vals_)[i] >= posmin && (*vals_)[i] <= posmax) {
      output = replaceProb(i, (*probs)[i]) || output;   //postot_ & negtot_ are updated
    }
  }
      
  if (fabs(oldtot - postot_) > 0.01) {
    output = true;
  }
  
//  Array<double>* posdens;
//  Array<double>* negdens;

  if (bw_opt_count_ < min_bw_opts) {
    pos_bw_ = d_->optimizeBandWidth(ovals_, posfit_);
    neg_bw_ = d_->optimizeBandWidth(zvals_, negfit_);
    bw_opt_count_++;
  }
  
  updateFits(posvarBW_, negvarBW_);
  
  return output;

}
bool KDModel::update(Array<double>* probs, int min_bw_opts) {

  double oldtot = postot_; 
  bool output = false;
  if (zvals_ != NULL) 
    zvals_->clear();
  if (ovals_ != NULL) 
    ovals_->clear();

  for (int i =0; i < probs->size(); i++) {
    output = replaceProb(i, (*probs)[i]) || output;   //postot_ & negtot_ are updated
  }
      
  if (fabs(oldtot - postot_) > 0.01) {
    output = true;
  }
  
//  Array<double>* posdens;
//  Array<double>* negdens;

  if (bw_opt_count_ < min_bw_opts) {
    pos_bw_ = d_->optimizeBandWidth(ovals_, posfit_);
    neg_bw_ = d_->optimizeBandWidth(zvals_, negfit_);
    bw_opt_count_++;
  }
  
  updateFits(posvarBW_, negvarBW_);
  
  return output;

}

bool KDModel::replaceProb(int idx, double prob) {
  assert(idx < posprobs_->size());
  bool ret = false;
  postot_ += prob - (*posprobs_)[idx];
  negtot_ += (1-prob) - (1-(*posprobs_)[idx]);
  
  if ( fabs((*posprobs_)[idx] - prob) > 0.01 ) {
    ret = true;
  }

  (*posprobs_)[idx] = prob;
  //(*negprobs_)[idx] = 1-prob;
  //if (prob <= 0.01) {
  //  zvals_->insertAtEnd((*vals_)[idx]);
  //}
  //if (prob >= 0.99) {
  //  ovals_->insertAtEnd((*vals_)[idx]);
  //}

  return ret;
}

void KDModel::insert(double prob, double val, double posmin, double posmax) {
  if (prob < 0 || prob > 1) {
    return;
  }

  if (val < posmin || val > posmax) {
    prob = 0;
  }
 
  posprobs_->insertAtEnd(prob);
  //  negprobs_->insertAtEnd(1-prob);
  postot_ += prob;
  negtot_ += 1 - prob;

  if (isnan(val)) {
	cerr << "ERROR: nan detected in KDModel ..." << endl;
  }

  vals_->insertAtEnd(val);


  //if (prob <= 0.01) {
  //  zvals_->insertAtEnd(val);
  //}
  //if (prob >= 0.99) {
  //  ovals_->insertAtEnd(val);
  //}
}
void KDModel::clear() {
  if (posprobs_ != NULL)
    posprobs_->clear();

  if (negprobs_ != NULL)
    negprobs_->clear();

  //if (posfit_ != NULL && posfit_->dens_ != NULL)
  //posfit_->dens_->clear();

  //if (negfit_ != NULL && negfit_->dens_ != NULL)
  // negfit_->dens_->clear();

  if (vals_ != NULL)
    vals_->clear();

  if (zvals_ != NULL)
    zvals_->clear();

  if (ovals_ != NULL)
    ovals_->clear();

  //if (grid_ != NULL)
  //  grid_->clear();

  postot_ = 0;
  negtot_ = 0;

  isready_ = false;
}

void KDModel::clearObs() {
 if (highobsfit_ != NULL) {
   delete highobsfit_;
   highobsfit_=NULL;
 }

 if (lowobsfit_ != NULL) {
   delete lowobsfit_;
   lowobsfit_=NULL;
 }

}
void KDModel::insert(double prob, double val) {
  if (prob < 0 || prob > 1) {
    return;
  }

  postot_ += prob;
  negtot_ += 1 - prob;
  
  posprobs_->insertAtEnd(prob);

  vals_->insertAtEnd(val);

  if (isnan(val)) {
    cerr << "ERROR: nan detected in KDModel ..." << endl;
  }

  //negprobs_->insertAtEnd(1-prob);
  
  //if (prob <= 0.01) {
  //  zvals_->insertAtEnd(val);
  //}
  //if (prob >= 0.99) {
  //  ovals_->insertAtEnd(val);
  //}
}

bool KDModel::initGrid() {
  return initGrid(20);
}

bool KDModel::initGrid(double breaks) {
  return initGrid(breaks, breaks);
}

bool KDModel::initGrid(double posbreaks, double negbreaks) {
  double minval=0;
  double maxval=0;

  double posminval=0;
  double posmaxval=0;
  double negminval=0;
  double negmaxval=0;
  // double zmin=0;
  // double zmax=0;
  // bool zset = false;

  double mean = 0;
  double sumsq = 0;
  double stddev = 0;

  double posmean = 0;
  double possumsq = 0;
  double posstddev = 0;
  
  double possum=0;

  double negsum=0;

  double negmean = 0;
  double negsumsq = 0;
  double negstddev = 0;

  
  posnumBreaks_ = posbreaks;
  negnumBreaks_ = negbreaks;

  for (int i=0; i<vals_->size(); i++) {
 
    if (i==0 || (*vals_)[i]<minval) {
      minval = (*vals_)[i];
    }
    if (i==0 || (*vals_)[i]>maxval) {
      maxval = (*vals_)[i];
    }

 
    if (i==0 || (*posprobs_)[i]*(*vals_)[i]<posminval) {
      posminval = (*posprobs_)[i]*(*vals_)[i];
    }
    if (i==0 || (*posprobs_)[i]*(*vals_)[i]>posmaxval) {
      posmaxval = (*posprobs_)[i]*(*vals_)[i];
    }

    if (i==0 || (1-(*posprobs_)[i])*(*vals_)[i]<negminval) {
      negminval = (1-(*posprobs_)[i])*(*vals_)[i];
    }
    if (i==0 || (1-(*posprobs_)[i])*(*vals_)[i]>negmaxval) {
      negmaxval = (1-(*posprobs_)[i])*(*vals_)[i];
    }

    mean += (*vals_)[i];
    sumsq += (*vals_)[i]*(*vals_)[i];

    posmean += (*posprobs_)[i]*(*vals_)[i];
    possum += (*posprobs_)[i];
    possumsq += (*posprobs_)[i]*(*posprobs_)[i]*(*vals_)[i]*(*vals_)[i];

    negmean += (1-(*posprobs_)[i])*(*vals_)[i];
    negsum += 1-(*posprobs_)[i];
    negsumsq += (1-(*posprobs_)[i])*(1-(*posprobs_)[i])*(*vals_)[i]*(*vals_)[i];
    
// if (!zset) {
//        if ((*posprobs_)[i] < 0.002) {
//  	zmin = (*vals_)[i];
//       	zmax = (*vals_)[i];
//        }
//        zset = true;
//      }
//      else {
//        if ((*vals_)[i] < zmin && (*posprobs_)[i] < 0.002) {
//  	zmin = (*vals_)[i];
//        }
//        if ((*vals_)[i] > zmax && (*posprobs_)[i] < 0.002) {
//  	zmax = (*vals_)[i];
//        }    
//     }

  }

  if (vals_->size() > 0) {
    mean  /= vals_->size();
    sumsq /= vals_->size();
    sumsq -= (mean * mean);
  }

  if (sumsq > 0) {
    stddev = sqrt(sumsq);
  }
  

  if (possum > 0) {
    posmean  /= possum;
    possumsq /= possum;
    possumsq -= (posmean * posmean);
  }

  if (possumsq > 0) {
    posstddev = sqrt(possumsq);
  }
  

  if (negsum > 0) {
    negmean  /= negsum;
    negsumsq /= negsum;
    negsumsq -= (negmean * negmean);
  }

  if (negsumsq > 0) {
    negstddev = sqrt(negsumsq);
  }
  

  //Compute Quantiles
  /*
  std::sort(sort_vals->begin(),sort_vals->end(),valcompare);
  if (vals_->size() % 2 == 0) {
    q2 = ( (*sort_vals_)[(vals_->size() / 2) - 1] + (*sort_vals_)[(vals_->size() / 2)] ) / 2;
    
    if ((vals_->size() / 2) % 2 == 0) {
      q1 = ( (*sort_vals_)[(vals_->size() / 4)-1] + (*sort_vals_)[(vals_->size() / 4)] ) / 2;
      q3 = ( (*sort_vals_)[3 * (vals_->size() / 4) - 1] + (*sort_vals_)[(3 * vals_->size() / 4)] ) / 2;
    }
    else {
      q1 = (*sort_vals_)[vals_->size() / 4];
      q3 = (*sort_vals_)[vals_->size() / 2 + vals_->size() / 4 ];
    }

  }
  else {
    q2 = (*sort_vals_)[vals_->size() / 2];

    if (((vals_->size() - 1) / 2) % 2 == 0) {
      q1 = ( (*sort_vals_)[((vals_->size() - 1) / 4) - 1] + (*sort_vals_)[((vals_->size()-1) / 4)] )/ 2;

      q3 = ( (*sort_vals_)[(vals_->size() / 2) + ((vals_->size()-1) / 4)] + (*sort_vals_)[(vals_->size() / 2) + ((vals_->size()-1) / 4) + 1]) / 2;
    }
    else {
      q1 = (*sort_vals_)[vals_->size() / 4];
      q3 = (*sort_vals_)[vals_->size() - (vals_->size() / 4) - 1];
      
    }
  }
  */

 


  if (minval == maxval || vals_->size() == 0) {
    return false;
  }
  

  minval -= 0.01;
  maxval += 0.01;


  //Set initial bandwidth to range / numBreaks the range ???

  //posmaxval = posmean + 4 * posstddev;
  //posminval = posmean - 4 * posstddev;

  //negmaxval = negmean + 4 * negstddev;
  //negminval = negmean - 4 * negstddev;
  
  //pos_bw_ = (posmaxval - posminval) / posnumBreaks_;
  //   neg_bw_ = (negmaxval - negminval) / negnumBreaks_;

  
  //pos_bw_ = posstddev;
  
  //neg_bw_ = negstddev;

  //pos_bw_ = neg_bw_ = negstddev < posstddev ? negstddev/negbreaks : posstddev/posbreaks;


  

  pos_bw_  = (maxval - minval) / posnumBreaks_;
  
  neg_bw_  = (maxval - minval) / negnumBreaks_;

   //neg_bw_ = pos_bw_ = ( 6 * stddev ) / numBreaks_;

  //if (zset && zmax > zmin) {
  //  neg_bw_ = (zmax - zmin) / numBreaks_;
  //}


  grid_ = new Array<double>();
  for (int i=0; i<numgrids_; i++) {
    grid_->insertAtEnd(minval + i*(maxval - minval) / (numgrids_-1));
  }
  return true;
  


}


void KDModel::initFit() {
  if (posfit_ == NULL) {
    posfit_ = new Fit(grid_, NULL); 

  }
  if (negfit_ == NULL) {
    negfit_ = new Fit(grid_, NULL); 
  }
}


void KDModel::initBandwidths() {
 // double q1, q2, q3;
  double pstddev = 0;
  double pmean = 0;
  double ptot = 0;
  double psum_sq = 0;
  double nstddev = 0;
  double nmean = 0;
  double ntot = 0;
  double nsum_sq = 0;
  
  std::vector<double>* sort_vals = new std::vector<double>();
  
  for (int i=0; i<vals_->size(); i++) {
    pmean +=  (*posprobs_)[i]*(*vals_)[i];
    ptot += (*posprobs_)[i];
    psum_sq +=  (*posprobs_)[i]*(*posprobs_)[i]*(*vals_)[i]*(*vals_)[i];

    nmean +=  (1-(*posprobs_)[i])*(*vals_)[i];
    nsum_sq +=  (1-(*posprobs_)[i])*(1-(*posprobs_)[i])*(*vals_)[i]*(*vals_)[i];
    ntot += 1-(*posprobs_)[i];
  }
  
  pmean /= ptot;
  pstddev = sqrt(psum_sq/ptot - pmean * pmean);
    
  nmean /= ntot;
  nstddev = sqrt(nsum_sq/ntot - nmean * nmean);

  pos_bw_ = 0.79*1.34*pstddev*pow(ptot, -0.20);
  neg_bw_ = 0.79*1.34*nstddev*pow(ntot, -0.20);

}

void KDModel::initKernels() {


  for (int i=0; i<grid_->size(); i++) {
    if (i >= pos_kerns_->size()) {
      obs_kerns_->insertAtEnd(d_->kernel(vals_, (*grid_)[i], pos_bw_));
      pos_kerns_->insertAtEnd(d_->kernel(vals_, (*grid_)[i], pos_bw_));
      neg_kerns_->insertAtEnd(d_->kernel(vals_, (*grid_)[i], neg_bw_));
    }
    else {
      delete (*obs_kerns_)[i];
      delete (*pos_kerns_)[i];
      delete (*neg_kerns_)[i];
      (*obs_kerns_)[i] = d_->kernel(vals_, (*grid_)[i], pos_bw_);
      (*pos_kerns_)[i] = d_->kernel(vals_, (*grid_)[i], pos_bw_);
      (*neg_kerns_)[i] = d_->kernel(vals_, (*grid_)[i], neg_bw_);
    }

  }
  


}

void KDModel::clearKernels() {

  if (pos_kerns_ != NULL) {
    for (int i=0; i<pos_kerns_->size(); i++) {
      (*pos_kerns_)[i]->clear();
    }
    pos_kerns_->clear();
  }
  if (neg_kerns_ != NULL) {
    for (int i=0; i<neg_kerns_->size(); i++) {
      (*neg_kerns_)[i]->clear();
    }
    neg_kerns_->clear();
  }
}

/*
bool KDModel::makeReady() {
  if (!initGrid()) return false;
  initFit();

  if (ovals_ == NULL || ovals_->size() <= 0) {
    pos_bw_ = d_->optimizeBandWidth(vals_,posfit_);
  }
  else {
    pos_bw_ = d_->optimizeBandWidth(ovals_,posfit_);
  }  

  if (zvals_ == NULL || zvals_->size() <= 0) {
  }
  else {
    neg_bw_ = d_->optimizeBandWidth(zvals_,negfit_);

  }
  //  if (!isready_) {
  //  initKernels();
  //}
  
  //  Array<double>* posdens = d_->wtDensityFit(grid_, vals_, pos_bw_, posprobs_, pos_kerns_, true);
  Array<double>* posdens = d_->wtDensityFit(grid_, vals_, pos_bw_, posprobs_, true);
  posfit_->dens_ = posdens;

  //  Array<double>* negdens = d_->wtDensityFit(grid_, vals_, neg_bw_, posprobs_, neg_kerns_, false);
  Array<double>* negdens = d_->wtDensityFit(grid_, vals_, neg_bw_, posprobs_, false);
  negfit_->dens_ = negdens;    

  isready_ = true;
  return isready_;
}
*/


bool KDModel::makeReady(double pos_bw, double neg_bw) {
  if (grid_ == NULL && !initGrid()) return false;
  initFit();

  pos_bw_ = pos_bw;
  neg_bw_ = neg_bw;

  //if (!isready_) {
  //  initKernels();
  //}
  
   //  Array<double>* posdens = d_->wtDensityFit(grid_, vals_, pos_bw_, posprobs_, pos_kerns_, true);
  Array<double>* posdens = d_->wtDensityFit(grid_, vals_, pos_bw_, posprobs_, true);
  posfit_->dens_ = posdens;

  //  Array<double>* negdens = d_->wtDensityFit(grid_, vals_, neg_bw_, posprobs_, neg_kerns_, false);
  Array<double>* negdens = d_->wtDensityFit(grid_, vals_, neg_bw_, posprobs_, false);
  negfit_->dens_ = negdens;    

  isready_ = true;
  return isready_;
}
 
void KDModel::updateObsFits(bool varBW, double highPr, double lowPr) {
  return updateObsFits(varBW, varBW,  highPr,  lowPr);
}

void KDModel::updateObsFits(bool posvarBW, bool negvarBW, double highPr, double lowPr) {
 
  if (highobsfit_ == NULL) {
    highobsfit_ = new Fit(grid_, NULL); 
  }

  if (lowobsfit_ == NULL) {
    lowobsfit_ = new Fit(grid_, NULL); 
  }


  Array<double>* lowobsdens;
  Array<double>* highobsdens;


  if (lowvals_ == NULL && highvals_ == NULL) {
    lowvals_ = new Array<double>();
    highvals_ = new Array<double>();
  }
  else {
    lowvals_->clear();
    highvals_->clear();
  }
  
  for(int k = 0; k < vals_->size(); k++) {
    if((*posprobs_)[k] >= highPr) {
      highvals_->insertAtEnd((*vals_)[k]);
    }
    if((*posprobs_)[k] <= lowPr) {
      lowvals_->insertAtEnd((*vals_)[k]);
    }
    
  }
  

  if (negvarBW) {

    lowobsfit_->varbws_ = negfit_->varbws_;
    
    
    lowobsdens = d_->varBWdensityFit(grid_, lowvals_, pos_bw_, lowobsfit_);
    
    if (lowobsfit_->dens_ != NULL) {
      delete  lowobsfit_->dens_;
    }
    lowobsfit_->dens_ = lowobsdens;
  }
  else if (grid_ != NULL) {

    lowobsdens = d_->densityFit(grid_, lowvals_, pos_bw_);
    
    if (lowobsfit_->dens_ != NULL) {
      delete  lowobsfit_->dens_;
    }
    
    lowobsfit_->dens_ = lowobsdens;

   
  }

  if (posvarBW) {
 
    highobsfit_->varbws_ = posfit_->varbws_;
    
  
    highobsdens = d_->varBWdensityFit(grid_, highvals_, pos_bw_, highobsfit_);
   
    if (highobsfit_->dens_ != NULL) {
      delete  highobsfit_->dens_;
    }
    highobsfit_->dens_ = highobsdens;
   


  }
  else if (grid_ != NULL) {

    highobsdens = d_->densityFit(grid_, highvals_, pos_bw_);
    
    if (highobsfit_->dens_ != NULL) {
      delete  highobsfit_->dens_;
    }
    
    highobsfit_->dens_ = highobsdens;

 
  }

}

void KDModel::updateFits(bool varBW) {
  return updateFits(varBW, varBW);
}

void KDModel::updateFits(bool posvarBW, bool negvarBW) {
  if (posvarBW) {

    if (!isready_) {
      Array<double>* tposdens = d_->wtDensityFit(grid_, vals_, pos_bw_, posprobs_, true);
      //Array<double>* tposdens = d_->densityFit(grid_, vals_, pos_bw_);
      if (posfit_->dens_ != NULL) {
	delete  posfit_->dens_;
      }
      
      posfit_->dens_ = tposdens;

    }
    d_->initVarBWs(posfit_, grid_, vals_, pos_bw_, posprobs_, true);    


    Array<double>* posdens = d_->varBWwtDensityFit(posfit_, grid_, vals_, pos_bw_, posprobs_, true);
    
    if (posfit_->dens_ != NULL) {
      delete  posfit_->dens_;
    }
    posfit_->dens_ = posdens;
    
  }
 else {
    Array<double>* tposdens = d_->wtDensityFit(grid_, vals_, pos_bw_, posprobs_, true);
    
    if (posfit_->dens_ != NULL) {
      delete  posfit_->dens_;
    }
    
    posfit_->dens_ = tposdens;

  }
    
  if (negvarBW) {
     if (!isready_) {
       Array<double>* tnegdens = d_->wtDensityFit(grid_, vals_, neg_bw_, posprobs_, false);
       //Array<double>* tnegdens = d_->densityFit(grid_, vals_, neg_bw_);
       if (negfit_->dens_ != NULL) {
	 delete  negfit_->dens_;
       }
       
       negfit_->dens_ = tnegdens;
     }
     d_->initVarBWs(negfit_, grid_, vals_, neg_bw_, posprobs_, false);


     Array<double>* negdens = d_->varBWwtDensityFit(negfit_, grid_, vals_, neg_bw_, posprobs_, false);
     
     if (negfit_->dens_ != NULL) {
       delete  negfit_->dens_;
     }
     
     negfit_->dens_ = negdens;    
     

  }
  else {
 
    Array<double>* tnegdens = d_->wtDensityFit(grid_, vals_, neg_bw_, posprobs_, false);
   
    if (negfit_->dens_ != NULL) {
      delete  negfit_->dens_;
    }
    
    negfit_->dens_ = tnegdens;    
  }
}

bool KDModel::makeReady() {
  return makeReady(false, 20);
}

bool KDModel::makeReady(bool varBW) {
  return makeReady(varBW, 20);
}
bool KDModel::makeReady(bool varBW, double posbreaks, double negbreaks) {
  return makeReady( varBW,  varBW,   posbreaks,  negbreaks);
}
bool KDModel::makeReady(bool posvarBW, bool negvarBW,  double posbreaks, double negbreaks) {
  posvarBW_ = posvarBW;
  negvarBW_ = negvarBW;
  if (grid_ == NULL && !initGrid(posbreaks, negbreaks)) return false;
  //  initBandwidths();


  initFit();
  
 
  //pos_bw_ = pos_bw;
  //neg_bw_ = neg_bw;
  
  //if (!isready_) {
  //  initKernels();
  //}
  
  updateFits(posvarBW_, negvarBW_);

 if (posvarBW_ || negvarBW_ || highobsfit_ == NULL || lowobsfit_ == NULL) {
    //Do only on the first iteration 
   updateObsFits(posvarBW_, negvarBW_, 0.95, 0.05);
    //        updateObsFits(varBW_, 0.9, 0.05);
  }

  isready_ = true;
  return isready_;
}

bool KDModel::makeReady(bool varBW, double breaks) {
  return makeReady(varBW, breaks, breaks);
}



bool KDModel::isReady() {
  return isready_;
}

double KDModel::getValAtIndex(int i) {
  if (i < 0 || i >= vals_->size()) {
    cerr << "Trying to get value of nonexistent index: " << i << endl;
    return -999;
  }
  return (*vals_)[i];

}

long KDModel::getValSize() {
  return (*vals_).size();

}

double KDModel::getPosProb(double val) {
  if (isready_) {
    return d_->predictDensityFit(val, posfit_); 
  }
  else {
    return 1;
  }
}

double KDModel::getNegProb(double val) {
  if (isready_) {
    return d_->predictDensityFit(val, negfit_);
  }
  else {
    return 1;
  }

}

Array<Tag*>*  KDModel::reportTags() {
  if (!isready_ || grid_ == NULL || grid_->size() == 0) {
    return NULL;
  }
  double step = (*grid_)[1]-(*grid_)[0];
  Array<Tag*>* output = new Array<Tag*>;
  Tag* tag = new Tag("mixturemodel", True, False);
  char text[500];
  sprintf(text, "%s", getName());
  tag->setAttributeValue("name", text);
  sprintf(text, "%f", pos_bw_);
  tag->setAttributeValue("pos_bandwidth", text);
  sprintf(text, "%f", neg_bw_);
  tag->setAttributeValue("neg_bandwidth", text);
  output->insertAtEnd(tag);
  for (int i=0; i<grid_->size()-1; i++) {
    tag = new Tag("point", True, True);
    sprintf(text, "%f", (*grid_)[i]);
    tag->setAttributeValue("value", text);
    sprintf(text, "%f", d_->predictDensityFit((*grid_)[i], posfit_));
    tag->setAttributeValue("pos_dens", text);
    sprintf(text, "%f", d_->predictDensityFit((*grid_)[i], negfit_));
    tag->setAttributeValue("neg_dens", text);
    output->insertAtEnd(tag);
    //    sprintf(text, "%f", slice((*grid_)[i], (*grid_)[i]+step));
    //tag->setAttributeValue("obs_dens", text);
    //output->insertAtEnd(tag);
  }
  tag = new Tag("mixturemodel", False, True);
  output->insertAtEnd(tag);
  return output;
}

double KDModel::slice(double low_val, double high_val) {
  int tot = 0;
  for(int k = 0; k < vals_->size(); k++) {
    if((*vals_)[k] > low_val && (*vals_)[k] < high_val) {
      tot++;
    }
  }
  return (double)tot/(double)vals_->size();
}

void KDModel::report(ostream& out) {
  

  out << "<mixturemodel name=\"" << name_ << "\" pos_bandwidth=\"" << pos_bw_ << "\" neg_bandwidth=\"" << neg_bw_ << "\">" << endl;
  if (isready_) {
    double step = (*grid_)[1]-(*grid_)[0];
    for (int i=0; i<grid_->size(); i++) {
      
      out << "<point value=\"" << (*grid_)[i] 
	  << "\" pos_dens=\"" <<  d_->predictDensityFit((*grid_)[i], posfit_) 
	  << "\" neg_dens=\"" <<  d_->predictDensityFit((*grid_)[i], negfit_)
      
	  << "\" neg_obs_dens=\"" <<  d_->predictDensityFit((*grid_)[i], lowobsfit_)
	  << "\" pos_obs_dens=\"" <<  d_->predictDensityFit((*grid_)[i], highobsfit_);
      

      out << "\"/>" << endl;
      
    }
  }
  out << "</mixturemodel>" << endl;
}


// void KDModel::adjust(double origin, bool left_inc, bool right_inc) {
  
//   if (origin < grid_[0] || origin > grid_[fit->grid_->size()-1]) {
//     return pred;
//   }
  


//   //  for (int i=0; i < fit->grid_->size()-1; i++) {

//   int l = 0;
//   int r = fit->grid_->size()-1;

  
  
//   int i = (int)((origin -grid_[l])/ fit->gridwid_);
  
//   while(l <= r) {
    
//     double a = grid_[i];
//     double b = grid_[i+1];

    
//     if (origin >= a && origin < b || i == 0 || i+1 == fit->grid_->size()-1) {
//       break;
//     }
    
//     if (origin > a) {
//       l = i;
//     }
//     if (origin < a) {
//       r = i;
//     }
//     i = (r + l) / 2;

//   }
    

// }

KDModel::~KDModel() {
  if (posprobs_ != NULL)
    delete posprobs_;
  if (negprobs_ != NULL)
    delete negprobs_;
  if (vals_ != NULL)
    delete vals_;

  if (highvals_ != NULL)
    delete highvals_;
  if (lowvals_ != NULL)
    delete lowvals_;

  if (posfit_ && posfit_->varbws_ != NULL)

    delete posfit_->varbws_;
  if (negfit_ && negfit_->varbws_ != NULL)
    delete negfit_->varbws_;
  if (posfit_ != NULL)
    delete posfit_;
  if (negfit_ != NULL)
    delete negfit_;

   if (obs_kerns_ != NULL)
    delete obs_kerns_;
   if (pos_kerns_ != NULL)
    delete pos_kerns_;
   if (neg_kerns_ != NULL)
    delete neg_kerns_;

   if (zvals_ != NULL)
    delete zvals_;
   if (ovals_ != NULL)
    delete ovals_;

  if (grid_ != NULL)
   delete grid_;

  if (highobsfit_ != NULL)
   delete highobsfit_;
  if (lowobsfit_ != NULL)
   delete lowobsfit_;
  if (d_ != NULL)
   delete d_;
  
}
