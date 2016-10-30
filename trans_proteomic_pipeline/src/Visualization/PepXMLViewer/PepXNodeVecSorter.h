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



#ifndef _INCLUDED_PEPXNODEVECSORTER_H_
#define _INCLUDED_PEPXNODEVECSORTER_H_

/** @file 
    @brief 
*/


#include "XMLNode.h"
#include "PepXField.h"
#include "PepXNode.h"


/**

   A function object for sorting PepXNode objects.  

   initialized with a coded fieldname, it determines which field to
   compare, and does any logic for numeric vs string, etc.

   

*/

class PepXNodeVecSorter {
protected:
  Field field_;
  int sortDir_;
  int colNum_;
public:
  PepXNodeVecSorter(Field field,
		    const int direction,
		    int colNum_);
  ~PepXNodeVecSorter() {}

  bool operator()(const PepXNodePtr& A, const PepXNodePtr& B);
};



#endif // header guards

