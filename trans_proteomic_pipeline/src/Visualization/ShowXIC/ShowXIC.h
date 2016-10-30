/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

/*
 * ShowXIC by David Shteynberg (c) Institute for Systems Biology, 12/30/2013
 *
 * Purpose:  Program to plot Extracted Ion Currents
 */

#include <string>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "common/Array.h"
#include <pwiz/data/msdata/MSDataFile.hpp>
#include <pwiz/data/msdata/SpectrumInfo.hpp>

using namespace std;
bool compare_mz (  pwiz::msdata::MZIntensityPair mz1,  pwiz::msdata::MZIntensityPair mz2) {
  return (mz1.mz < mz2.mz);
}


class ShowXIC {

 public:

  ShowXIC(string* file, double mz) ;
  
  Array<double>* extractXIC() ;
  void printXICd3(ostream& out);
 

  ~ShowXIC();
   
 private:
  string* file_;
  double  mz_;
  Array<double>* xic_;
};
