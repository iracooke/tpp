#include "CruxDiscrimFunction.h"

/*

Program       : DiscriminantFunction for discr_calc of PeptideProphet                                                       
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


CruxDiscrimFunction::CruxDiscrimFunction(int charge, Boolean use_xcorr) : DiscriminantFunction(charge) {

  double consts[] = {0.646, -0.959, -1.460, -0.774, -0.598, -0.598, -0.598};
  double xcorrs[] = {5.49, 8.362, 9.933, 1.465, 3.89, 3.89, 3.89};
  double deltas[] = {4.643, 7.386, 11.149, 8.704, 7.271, 7.271, 7.271};
  double ranks[] = {-0.455, -0.194, -0.201, -0.331, -0.377, -0.377, -0.377};
  double massdiffs[] =  {-0.84, -0.314, -0.277, -0.277, -0.84, -0.84, -0.84};
  int max_pep_lens[] = {100, 15, 25, 50, 100, 100, 100};
  int num_frags[] = {2, 2, 3, 4, 6, 6, 6};

  const_ = consts[charge_];
  xcorr_p_wt_ = xcorrs[charge_];
  delta_wt_ = deltas[charge_];
  log_rank_wt_ = ranks[charge_];
  abs_massd_wt_ = massdiffs[charge_];
  max_pep_len_ = max_pep_lens[charge_];
  num_frags_ = num_frags[charge_];

  use_xcorr_ = use_xcorr;

  if (use_xcorr_) {
    delta_wt_ = 0;
  }

  min_val_ = -100;

}



Boolean CruxDiscrimFunction::isComputable(SearchResult* result) {
  return (result != NULL && ((CruxResult*)(result))->xcorr_ > 0.0);
}

double CruxDiscrimFunction::getDiscriminantScore(SearchResult* result) {
  if(strcasecmp(result->getName(), "Crux") != 0) {
    cerr << "illegal type of Crux result: " << result->getName() << endl;
    exit(1);
  }
  CruxResult* cruxresult = (CruxResult*)(result);
  double tot = const_;
  tot += xcorr_p_wt_ * getXcorrP(cruxresult->xcorr_, getPepLen(cruxresult->peptide_));
  if(cruxresult->delta_ < 1.0) // exclude special case of deltaCn equal to 1.00 (i.e. override by setting to 0)
    tot += delta_wt_ * cruxresult->delta_;
  //tot += log_rank_wt_ * log((double)cruxresult->rank_);
  //tot += abs_massd_wt_ * myfabs(cruxresult->massdiff_);
  Boolean writeMultivariate = False;

  if(writeMultivariate) {
    ofstream fMulti("multivariate.txt", ios::app);
    if(! fMulti) {
      cerr << "cannot append multivariate info for " << cruxresult->spectrum_ << endl;
      exit(1);
    }
    fMulti << cruxresult->spectrum_ << "\t" << getXcorrP(cruxresult->xcorr_, getPepLen(cruxresult->peptide_)) << "\t" << cruxresult->delta_ << "\t" << myfabs(cruxresult->massdiff_) << "\t" << tot << endl;
    fMulti.close();
  }

   return tot;
}


int CruxDiscrimFunction::getPepLen(char* pep) {
  if(pep == NULL)
    return 0;
  int start = 0;
  int end = (int)strlen(pep);
  if(strlen(pep) > 4 && pep[1] == '.' && pep[strlen(pep)-2] == '.') {
    start = 2;
    end = (int)strlen(pep) - 2;
  }
  int tot = 0;
  for(int k = start; k < end; k++) 
    if(pep[k] >= 'A' && pep[k] <= 'Z')
      tot++;

  return tot;
}

double CruxDiscrimFunction::getXcorrP(double xcorr, int peplen) {
  int eff_pep_len = peplen;
  if(eff_pep_len > max_pep_len_)
    eff_pep_len = max_pep_len_;

  return (log(xcorr)) / (log((double)(eff_pep_len * num_frags_)));

}
