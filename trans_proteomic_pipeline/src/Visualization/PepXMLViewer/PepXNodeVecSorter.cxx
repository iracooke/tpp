// -*- mode: c++ -*-



/*
    Program: PepXMLViewer.cgi
    Description: CGI program to display pepxml files in a web browser
    Date: July 31 2006

    Copyright (C) 2006  Natalie Tasman (original author), ISB Seattle

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    Natalie Tasman
    Institute for Systems Biology
    401 Terry Avenue North
    Seattle, WA  98109  USA
    
    email (remove underscores):
    n_tasman at systems_biology dot org
*/



/** @file .cxx
    @brief 
*/


#include <math.h> // ceil
#include <stdexcept>
#include <algorithm>

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include "PepXUtility.h"
#include "XMLNode.h"
#include "XMLTree.h"

#include "PepXNodeVecSorter.h"

using namespace std;
using namespace boost;




PepXNodeVecSorter::PepXNodeVecSorter(Field field,
			     int direction,
			     int colNum) : 
  field_(field),
  sortDir_(direction),
  colNum_(colNum)
{  
}


bool PepXNodeVecSorter::operator()(const PepXNodePtr& A, const PepXNodePtr& B) {


  static double valA, valB;
  static string strA, strB;
   

  strA=A->sortData[colNum_];
  strB=B->sortData[colNum_];
  valA = -999;
  valB = -999;



  // text fields
  if (field_.fieldName_ == "peptide" ||
      field_.fieldName_ == "protein" ||
      field_.fieldName_ == "spectrum" ||
      field_.fieldName_ == "ions") {


    // compare text
    if (sortDir_) {
      return (strA>=strB);
    }
    else {
      return (strA<strB);
    }
  } 

  // convert strings to numeric
  if (strA == "") {
    valA = 0;
  }
  else {
    toDouble(strA, valA);
  }

  if (strB == "") {
    valB = 0;
  }
  else {
    toDouble(strB, valB);
  }

  // compare numeric
  if (sortDir_) {
    //cout << valA << " > " << valB << "?";
    //cout << (valA > valB) << endl;
    return (valA > valB);
  }
  else {
    //cout << valA << " < " << valB << "?";
    //cout << (valA < valB) << endl;
    return (valA < valB);
  }
}





