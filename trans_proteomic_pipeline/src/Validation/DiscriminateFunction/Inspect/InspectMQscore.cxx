#include "InspectDiscrimValMixtureDistr.h"

/*

Program       : InspectMQscoreDF for discr_calc of PeptideProphet                                                       
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

class InspectMQscoreDF : public InspectDiscrimFunction
{
public:
    InspectMQscoreDF(int charge) : InspectDiscrimFunction(charge) { 
      
      const_ = 0.25 ;
      mq_wt_ = 1.75 ;
      massError_wt_ = 0;
      mzSSE_wt_ = 0;
      min_val_ = -5;
      len_wt_ = 0;
        //if (charge == 0)
        //    min_val_ = -1.0;
    }
};

class InspectMQscoreDFFactory : public InspectDiscrimFunctionFactory
{
public:
    InspectMQscoreDFFactory() : InspectDiscrimFunctionFactory("zscore") {}

    InspectDiscrimFunction* createDiscrimFunction(int charge) {
        return new InspectMQscoreDF(charge);
    }
};

static InspectMQscoreDFFactory factory;

void linkInspectMQscore() {}
