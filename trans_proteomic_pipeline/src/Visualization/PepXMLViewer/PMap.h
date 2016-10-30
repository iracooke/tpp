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


#ifndef _INCLUDED_PMAP_H_
#define _INCLUDED_PMAP_H_


#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

using std::string;
using std::vector;
using boost::shared_ptr;


/** @file 
    @brief 
*/


struct AAModInfo {
  enum {
    static_mod=1,
    variable_mod
  }; // mod types
  string aa_;
  double modValue_; // delta, so iodoacet. is 57.
  int modType_; // static or variable
  AAModInfo() : aa_(""), modValue_(0), modType_(0) {
  }
  AAModInfo(const string& aa,
	    double modValue,
	    int modType) : aa_(aa), modValue_(modValue), modType_(modType) {
  }
	  
};

typedef shared_ptr<AAModInfo> AAModInfoPtr;
typedef vector<AAModInfoPtr> AAModInfoVec;




class PMap {
 public:
  virtual string operator[](const string& from) = 0;
  virtual AAModInfoVec* getModVec(void) {
    return 0;
  }
  virtual ~PMap() { }
};

#endif // header guards


