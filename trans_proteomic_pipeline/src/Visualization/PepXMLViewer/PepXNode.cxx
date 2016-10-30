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



/** @file PepXNode.cxx
    @brief 
*/


#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include <boost/regex.hpp>

#include "PepXUtility.h"

#include "PepXNode.h"



void 
PepXNode::reset(void) {
  index =-1;
  runSummaryNumber = -1;
  startOffset = -1;
  endOffset = -1;
  sortData.clear();
}

  
void
PepXNode::print(std::ostream& outstream) {
  outstream 
    << index << "\t"
    << runSummaryNumber << "\t"
    << startOffset << "\t"
    << endOffset << "\t";
  //<< sortValue << "\t";

  for (unsigned int e=0;e<sortData.size(); e++) {
    if (e>0) {
      outstream << "\t";
    }
    outstream << sortData[e];
  }
  outstream << std::endl;    
}


bool
PepXNode::read(std::ifstream & in) {
  std::string line;
  safeReadLine(in, line);
  if (line == "") return false;
  //outstream << "input line: *" << line << "*" << std::endl;
  boost::regex reTab("\t");
  boost::sregex_token_iterator i(line.begin(), line.end(), reTab, -1);
  boost::sregex_token_iterator j;
  std::string t;

  t = *i;
  toInt(t, index);


  i++;
  t = *i;
  toInt(t,runSummaryNumber);
    
  i++;
  t = *i;
  toInt(t, startOffset);
  
    
  i++;
  t = *i;
  toInt(t, endOffset);

  //     i++;
  //     t = *i;
  //     sortValue = t;


  i++;
  while (i != j) {
    t = *i;
    sortData.push_back(t);
    i++;
  }

  //outstream << "read " << sortData.size() << std::endl  << std::endl  << std::endl;
  return true;
}
