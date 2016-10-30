#include "SpectraSTDiscrimFunction.h"

/*

Program       : SpectraSTDiscriminantFunction for discr_calc of PeptideProphet                                                       
Author        : Henry Lam <hlam@systemsbiology.org>                                                       
Date          : 04.10.06 


Copyright (C) 2006 Henry Lam

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
hlam@systemsbiology.org

*/

SpectraSTDiscrimFunction::SpectraSTDiscrimFunction(int charge) : DiscriminantFunction(charge) { 
 
  dot_wt_ = 0.6;
  normdeltadot_wt_ = 0.4;

  meanDot_ = 0.0;
  meanDelta_ = 0.0;

}




Boolean SpectraSTDiscrimFunction::isComputable(SearchResult* result) {
  return True;
}
  
double SpectraSTDiscrimFunction::getDiscriminantScore(SearchResult* result) {
  if(strcasecmp(result->getName(), "SpectraST") != 0) {
    cerr << "illegal type of SpectraST result: " << result->getName() << endl;
    exit(1);
  }
  SpectraSTResult* spectraSTResult = (SpectraSTResult*)(result);

  return (spectraSTResult->fval_);
  /*

  double score = 0.0;
  if (spectraSTResult->dot_ < 0.01) {
    score = 0.15; // put into neg without messing up with the distribution shape
  } else {
    //    score = dot_wt_ * spectraSTResult->dot_ + normdeltadot_wt_ * spectraSTResult->deltadot_ / spectraSTResult->dot_;
    score = 0.5 * spectraSTResult->dot_ + 0.5 * spectraSTResult->separation_;
  }  
  if ((spectraSTResult->dotbias_ < 0.1 || spectraSTResult->dotbias_ > 0.35) && score > 0.4) {
  	score -= 0.12;
	if (spectraSTResult->dotbias_ > 0.4) {
	  score -= 0.06;
	}
	if (spectraSTResult->dotbias_ > 0.45) {
	  score -= 0.06;
	}
  }	

  return (score);
  */
}

double SpectraSTDiscrimFunction::getDiscriminantScore(double dot, double delta) {

  if (dot < 0.00001) return (0.0);

  double normdelta = delta / dot;
  if (normdelta > 2.0 * dot) {
    // Put a cap on delta at 2 * dot. 
    normdelta = 2.0 * dot;
  }
  
  return (dot * dot_wt_ + normdelta * normdeltadot_wt_);

}

void SpectraSTDiscrimFunction::optimize(vector<double>& dots, vector<double>& deltas) {

  // calculate covariance matrix
  double sumDot = 0.0;
  double sumSqDot = 0.0;
  double sumDelta = 0.0;
  double sumSqDelta = 0.0;
  double sumDotDelta = 0.0;
  double count = 0;

  for (int i = 0; i < dots.size() && i < deltas.size(); i++) {
    double dot = dots[i];
    double delta = deltas[i];
    if (dot >= 0.00001) delta /= dot;

    if (delta > 2.0 * dot) delta = 2.0 * dot;
    //if (delta > dot) delta = dot; // require delta <= dot, such that fval <= dot

    sumDot += dot;
    sumSqDot += (dot * dot);
    sumDelta += delta;
    sumSqDelta += (delta * delta);
    sumDotDelta += (dot * delta);
    count++;
  }

  if (count < 50) {
    // sample size too small, probably not reliable
    //    cerr << "Data points (" << count << ") too few. Use default F-value = (0.6 x DOT) + (0.4 x DELTA/DOT)." << endl;
    dot_wt_ = 0.6;
    normdeltadot_wt_ = 0.4;
    return;
  }

  meanDot_ = sumDot / (double)count;
  meanDelta_ = sumDelta / (double)count;
  
  /*
  // correct problematic cases (dot below average, delta above average)
  for (int i = 0; i < dots.size() && i < deltas.size(); i++) {
    double dot = dots[i];
    double delta = deltas[i];
    if (dot >= 0.00001) delta /= dot; 
    if (delta > meanDelta_ && dot < meanDot_) {
      sumDelta -= (delta - meanDelta_);
      sumSqDelta -= (delta * delta - meanDelta_ * meanDelta_);
      sumDotDelta -= (dot * delta - dot * meanDelta_);
    }
  }
  meanDelta_ = sumDelta / (double)count;
  */
  
  double varDot = sumSqDot / (double)count - meanDot_ * meanDot_;
  double varDelta = sumSqDelta / (double)count - meanDelta_ * meanDelta_;
  double covar = sumDotDelta / (double)count - meanDot_ * meanDelta_;

  // calculate eigenvalue and eigenvector
  double largerEigenvalue = 0.5 * ((varDot + varDelta) + sqrt((varDot + varDelta) * (varDot + varDelta) - 4 * (varDot * varDelta - covar * covar)));

  normdeltadot_wt_ = (largerEigenvalue - varDot) / (covar + largerEigenvalue - varDot);
  dot_wt_ = 1.0 - normdeltadot_wt_;
  
  //  cerr << "Optimized F-value = (" << dot_wt_ << " x DOT) + (" << normdeltadot_wt_ << " x DELTA/DOT)" << endl;

  //  normdeltadot_wt_ = 0.4;
  //dot_wt_ = 0.6;
  //cerr << "Using default only" << endl;
  
}
