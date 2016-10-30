#include "CometDiscrimFunction.h"

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

CometDiscrimFunction::CometDiscrimFunction(int charge, Boolean use_expect) : DiscriminantFunction(charge) {

  double consts[] = {0.646, -0.959, -1.460, -0.774, -0.598, -0.598, -0.598};
  double xcorrs[] = {5.49, 8.362, 9.933, 1.465, 3.89, 3.89, 3.89};
  double deltas[] = {4.643, 7.386, 11.149, 8.704, 7.271, 7.271, 7.271};
  double ranks[] = {-0.455, -0.194, -0.201, -0.331, -0.377, -0.377, -0.377};
  double massdiffs[] =  {-0.84, -0.314, -0.277, -0.277, -0.84, -0.84, -0.84};
  int max_pep_lens[] = {100, 15, 25, 50, 100, 100, 100};
  int num_frags[] = {2, 2, 3, 4, 6, 6, 6};
  use_expect_ = use_expect;
  const_ = consts[charge_];
  xcorr_p_wt_ = xcorrs[charge_];
  delta_wt_ = deltas[charge_];
  log_rank_wt_ = ranks[charge_];
  abs_massd_wt_ = massdiffs[charge_];
  max_pep_len_ = max_pep_lens[charge_];
  num_frags_ = num_frags[charge_];
}



Boolean CometDiscrimFunction::isComputable(SearchResult* result) {
  return (result != NULL && ((CometResult*)(result))->xcorr_ > 0.0);
}

double CometDiscrimFunction::getDiscriminantScore(SearchResult* result) {
  if(strcasecmp(result->getName(), "Comet") != 0) {
    cerr << "illegal type of Comet result: " << result->getName() << endl;
    exit(1);
  }
 
  CometResult* seqresult = (CometResult*)(result);
  if (use_expect_) {
    return -5-log((double)seqresult->expect_);
  }
  double tot = const_;
  tot += xcorr_p_wt_ * getXcorrP(seqresult->xcorr_, getPepLen(seqresult->peptide_));
  if(seqresult->delta_ < 1.0) // exclude special case of deltaCn equal to 1.00 (i.e. override by setting to 0)
    tot += delta_wt_ * seqresult->delta_;
  tot += log_rank_wt_ * log((double)seqresult->rank_);
  tot += abs_massd_wt_ * myfabs(seqresult->massdiff_);
  Boolean writeMultivariate = False;

  if(writeMultivariate) {
    ofstream fMulti("multivariate.txt", ios::app);
    if(! fMulti) {
      cerr << "cannot append multivariate info for " << seqresult->spectrum_ << endl;
      exit(1);
    }
    fMulti << seqresult->spectrum_ << "\t" << getXcorrP(seqresult->xcorr_, getPepLen(seqresult->peptide_)) << "\t" << seqresult->delta_ << "\t" << log((double)seqresult->rank_) << "\t" << myfabs(seqresult->massdiff_) << "\t" << tot << endl;
    fMulti.close();
  }

   return tot;
}


int CometDiscrimFunction::getPepLen(char* pep) {
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

double CometDiscrimFunction::getXcorrP(double xcorr, int peplen) {
  int eff_pep_len = peplen;
  if(eff_pep_len > max_pep_len_)
    eff_pep_len = max_pep_len_;

  return (log(xcorr)) / (log((double)(eff_pep_len * num_frags_)));

}
