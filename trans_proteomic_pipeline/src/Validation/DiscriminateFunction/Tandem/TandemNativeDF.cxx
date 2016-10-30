#include "TandemDiscrimValMixtureDistr.h"

/*

Program       : TandemNativeDF for discr_calc of PeptideProphet                                                       
Author        : Brendan MacLean <bmaclean%at%fhcrc.org>                                                       
Date          : 06.20.06 

Copyright (C) 2006 Brendan MacLean and Andrew Keller

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

class TandemNativeDF : public TandemDiscrimFunction
{
public:
  TandemNativeDF(int charge, Boolean use_expect = False) : TandemDiscrimFunction(charge, use_expect) { 
        static double consts[] = {0.72, 0.4, 0.2, 0.2, 0.2};
        static double expect_wts[] = {0.43, 0.4, 0.5, 0.5, 0.5};
        static double delta_wts[] = {4.3, 4.0, 5.0, 5.0, 5.0};
        static double len_wts[] = {0.25, 0.25, 0.25, 0.25, 0.25};
        static double min_vals[] = {-1.0, -1.8, -1.1, -1.1, -1.1};
	if (!use_expect) {
	  const_ = consts[charge];
	  expect_wt_ = expect_wts[charge];
	  len_wt_ = len_wts[charge];
	  delta_wt_ = delta_wts[charge];
	  min_val_ = min_vals[charge];
	}
	else {
	  const_ = 3.0;
	  score_wt_ = 0;
	  delta_wt_ = 0;
	  len_wt_ = 0;
	  expect_wt_ = 0.25;
	}
    }

    double getDiscriminantScore(SearchResult* result) {
        double tot = TandemDiscrimFunction::getDiscriminantScore(result);

        // TODO: Understand why this has an impact on whether a model
        //       can be computed, when min value is set to > -2.0 in all cases.
        if (tot < -2.0)
            return -2.0;
        return tot;
    }
};

class TandemNativeDFFactory : public TandemDiscrimFunctionFactory
{
public:
  TandemNativeDFFactory() : TandemDiscrimFunctionFactory("native") {}

  TandemDiscrimFunction* createDiscrimFunction(int charge) {
    return new TandemNativeDF(charge, False);
  }
  TandemDiscrimFunction* createDiscrimFunction(int charge, Boolean use_expect) {
    return new TandemNativeDF(charge, use_expect);
  }
};

static TandemNativeDFFactory factory;

void linkTandemNative() {}
