#include "AbbrevSequestDiscrimFunction.h"

/*

Program       : AbbrevSequestDiscrimFunction for PeptideProphet                                                       
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


AbbrevSequestDiscrimFunction::AbbrevSequestDiscrimFunction(int charge, bool use_expect) : SequestDiscrimFunction(charge, use_expect) {
  double consts[] = {-0.236, -1.498, -1.975, -0.774, -0.598, -0.598, -0.598};
  double xcorrs[] = {8.346, 9.3, 10.685, 1.465, 3.89, 3.89, 3.89};
  double deltas[] = {3.904, 7.317, 11.263, 8.704, 7.271, 7.271, 7.271};
  double ranks[] = {-0.536, -0.199, -0.207,  -0.331, -0.377, -0.377, -0.377};
  double massdiffs[] =  {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  const_ = consts[charge_];
  xcorr_p_wt_ = xcorrs[charge_];
  delta_wt_ = deltas[charge_];
  log_rank_wt_ = ranks[charge_];
  abs_massd_wt_ = massdiffs[charge_];
}
