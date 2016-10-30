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



#ifndef _INCLUDED_PEPXNODE_H_
#define _INCLUDED_PEPXNODE_H_

/** @file PepXNode.h
    @brief 
*/

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include <boost/shared_ptr.hpp>

#include "PepXUtility.h"

using std::string;
using std::vector;
using std::ostream;
using std::fstream;

using boost::shared_ptr;



struct PepXNode;
typedef shared_ptr<PepXNode> PepXNodePtr;
typedef vector<PepXNodePtr> PepXNodeVec;


struct PepXNode {
  int index;
  int runSummaryNumber;
  int startOffset;
  int endOffset;
  string sortValue;
  string sortDataString; // all sortable fields (allColumns)
  vector<string> sortData;
  PepXNode() : 
    index(-1),
    runSummaryNumber(-1),
    startOffset(-1),
    endOffset(-1)
  {
  }

  void reset(void);
  void print(ostream& outstream);
  bool read(ifstream & in);

};

#endif // header guards
