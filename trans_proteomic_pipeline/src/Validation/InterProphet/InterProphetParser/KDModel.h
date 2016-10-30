#ifndef _KD_MODEL_
#define _KD_MODEL_
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

#include <ostream>
#include <assert.h>
#include "Parsers/Parser/Tag.h"
#include "common/util.h"
#include "common/sysdepend.h"
#include "common/Array.h"
#include "Validation/Distribution/NonParametricDistribution.h"
#include "Validation/Distribution/Fit.h"

//bool valcompare(const double &d1, const double &d2) {
//  return (d1> d2)?1:0;
//}

class KDModel {
 public:
  KDModel(const char* name, int numgrids = 500);

  void insert(double prob, double val, double posmin, double posmax);

  void insert(double prob, double val);

  void initKernels();

  void report(ostream& out);
  Array<Tag*>* reportTags();
  
  bool update(Array<double>* probs, int min_bw_opts);

  bool update(Array<double>* probs, int min_bw_opts, double posmin, double posmax);

  bool replaceProb(int idx, double newprob);

  void initFit();
  void updateFits(bool varBW);
  void updateObsFits(bool varBW, double highPr, double lowPr);

  void updateFits(bool posvarBW, bool negvarBW);
  void updateObsFits(bool posvarBW, bool negvarBW, double highPr, double lowPr);

  void clear();
  void clearObs();

  void clearKernels();
  
  bool initGrid();

  double slice(double low_val, double high_val);

  bool initGrid(double breaks);
  bool initGrid(double posbreaks, double negbreaks);

  void initBandwidths();
  
  bool makeReady();

  bool makeReady(bool varBW, double posbreaks, double negbreaks);

  bool makeReady(bool varBW, double breaks);

  bool makeReady(bool varBW);

  bool makeReady(double pos_bw, double neg_bw);

  bool makeReady(bool posvarBW, bool negvarBW, double posbreaks, double negbreaks);

  bool isReady();

  double getPosProb(double val);
  
  double getValAtIndex(int i) ;
  long getValSize();

  double getNegProb(double val);
  ~KDModel() ;

  const char *getName() {
	  return name_.c_str();
  }

 private:
  bool isready_;

  double postot_;
  double negtot_;
  
  Array<Array<double>*>* obs_kerns_;
  Array<Array<double>*>* pos_kerns_;
  Array<Array<double>*>* neg_kerns_;

  Array<double>* posprobs_;
  Array<double>* negprobs_;

  Array<double>* vals_;
  Array<double>* zvals_;
  Array<double>* ovals_;

  Array<double>* lowvals_;
  Array<double>* highvals_; 

  Array<double>* grid_;
  
  int numgrids_;

  Fit* highobsfit_;
  Fit* lowobsfit_;
  Fit* posfit_;
  Fit* negfit_;
  
  double bw_;
  
  double pos_bw_;
  double neg_bw_;

  int bw_opt_count_;
  string name_;

  bool posvarBW_;

  bool negvarBW_;

  double posnumBreaks_;

  double negnumBreaks_;

  int numBreaks_;

  NonParametricDistribution* d_;
  
};

#endif
