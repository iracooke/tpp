#include "TandemDiscrimValMixtureDistr.h"

/*

Program       : TandemKscoreDF for discr_calc of PeptideProphet                                                       
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

class TandemHRKscoreDF : public TandemDiscrimFunction
{
public:
  TandemHRKscoreDF(int charge, Boolean use_expect = False) : TandemDiscrimFunction(charge, use_expect) { 
      

      //HL: static double consts[] = {-24.2340, -21.5331, -22.0414, -22.0414, -22.0414};
      //HL: static double score_wts[] = {4.3843, 3.8073, 3.8092, 3.8092, 3.8092};
      //HL: static double delta_wts[] = {9.8113, 6.1410, 6.7053, 6.7053, 6.7053};

      //ORIG:
      static double consts[] = {-13.287, -28.708, -31.083, -31.083, -31.083};
      static double score_wts[] = {2.256, 4.91, 4.983, 4.983, 4.983};
      static double delta_wts[] = {14.346, 10.882, 18.091, 18.091, 18.091};

      //DDS: static double consts[] = {-14.3, -18.6, -16, -16, -16};
      //DDS: static double score_wts[] = {2.224, 3.176, 2.529,  2.529, 2.529};
      //DDS: static double delta_wts[] = {7.311, 7.646, 10.863, 10.863, 10.863};
      
      //ORIG 1+ , DDS: 2+, 3+
      //static double consts[] = {-13.287, -18.6, -16, -16, -16};
      //static double score_wts[] = {2.256, 3.176, 2.529,  2.529, 2.529};
      //static double delta_wts[] = {14.346, 7.646, 10.863, 10.863, 10.863};
      
      if (!use_expect) {
	const_ = consts[charge];
	score_wt_ = score_wts[charge];
	delta_wt_ = delta_wts[charge];

      }
      else {
	const_ = 3.0;
	score_wt_ = 0;
	delta_wt_ = 0;
	len_wt_ = 0;
	expect_wt_ = 0.25;
	  
      }

        //if (charge == 0)
        //    min_val_ = -1.0;
    }
};

class TandemHRKscoreDFFactory : public TandemDiscrimFunctionFactory
{
public:
  TandemHRKscoreDFFactory() : TandemDiscrimFunctionFactory("hrk-score") {}
  
  TandemDiscrimFunction* createDiscrimFunction(int charge) {
    return new TandemHRKscoreDF(charge, False);
  }
  
  TandemDiscrimFunction* createDiscrimFunction(int charge, Boolean use_expect) {
    return new TandemHRKscoreDF(charge, use_expect);
  }
};

static TandemHRKscoreDFFactory factory;

void linkTandemHRKscore() {}
