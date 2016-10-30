
#include "Distribution.h"

/*

Program       : Distribution for PeptideProphet                                                       
Author        : Andrew Keller <akeller@systemsbiology.org>                                                       
Date          : 11.27.02 

Distribution

This software will be available as open source in the future.  However, at this
time we are sharing it with you as a collaborator and ask that you do not
redistribute it.

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

Distribution::Distribution() { 
  newtot_ = NULL; // avoid wild ptrs
  tot_ = NULL; // avoid wild ptrs
}

Distribution::Distribution(double maxdiff) {
  maxdiff_ = maxdiff;
  set_ = False;
  newtot_ = NULL;
  tot_ = NULL;
  minval_ = 0;
}


Distribution::~Distribution() { 
  if(tot_ != NULL) {
    delete [] tot_;
  }
  if(newtot_ != NULL) {
    delete [] newtot_;
  }

}

void Distribution::setPriors(double *priors,double numpriors) {
	cerr << "error: Distribution::setPriors called" << endl;
}

void Distribution::setMinVal(double min) { 
  minval_ = min; 
}

int Distribution::getCount(int val) { 
   cerr << "error: Distribution::getCount called" << endl;
   return -1;
}

void Distribution::init(double* prior, Boolean alphabeta) {
  init(prior);
}
