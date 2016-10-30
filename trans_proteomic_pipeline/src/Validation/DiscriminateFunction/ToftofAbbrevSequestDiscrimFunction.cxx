#include "ToftofAbbrevSequestDiscrimFunction.h"

ToftofAbbrevSequestDiscrimFunction::ToftofAbbrevSequestDiscrimFunction(bool use_expect) : SequestDiscrimFunction(0, use_expect) {

  double consts[] = {-1.244};
  double xcorrs[] = {5.375};
  double deltas[] = {5.078};
  double ranks[] = {-0.166};
  double massdiffs[] =  {0.0};

  const_ = consts[charge_];
  xcorr_p_wt_ = xcorrs[charge_];
  delta_wt_ = deltas[charge_];
  log_rank_wt_ = ranks[charge_];
  abs_massd_wt_ = massdiffs[charge_];
  use_expect_ = use_expect;

}
