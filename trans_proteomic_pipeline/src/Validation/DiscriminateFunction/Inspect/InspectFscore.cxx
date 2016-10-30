#include "InspectDiscrimValMixtureDistr.h"

/*

Program       : InspectFscoreDF for discr_calc of PeptideProphet                                                       
Author        : David Shteynberg <dshteynb%at%systemsbiology.org>                                                       
Date          : 05.16.07 

Copyright (C) 2007 David Shteynberg and Andrew Keller

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

class InspectFscoreDF : public InspectDiscrimFunction
{
public:
    InspectFscoreDF(int charge) : InspectDiscrimFunction(charge) { 
      
      const_ = -3. ;
      mq_wt_ = 0. ;
      fscore_wt_ = 1.3 ;
      massError_wt_ = 0;
      mzSSE_wt_ = 0;
      min_val_ = -5;
      len_wt_ = 0;
        //if (charge == 0)
        //    min_val_ = -1.0;
    }
};

class InspectFscoreDFFactory : public InspectDiscrimFunctionFactory
{
public:
    InspectFscoreDFFactory() : InspectDiscrimFunctionFactory("zscore") {}

    InspectDiscrimFunction* createDiscrimFunction(int charge) {
        return new InspectFscoreDF(charge);
    }
};

static InspectFscoreDFFactory factory;

void linkInspectFscore() {}
