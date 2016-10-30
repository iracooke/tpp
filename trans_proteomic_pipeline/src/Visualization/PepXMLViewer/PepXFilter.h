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



#ifndef _INCLUDED_PEPXFILTER_H_
#define _INCLUDED_PEPXFILTER_H_

/** @file filter.h
    @brief 
*/




#include <vector>
#include <string>
#include <iostream>
#include <sstream>

// include headers that implement a archive in simple text format
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

#include "PepXField.h"

using std::string;
using std::vector;
using std::ostream;





/**
   numeric tests: min, max, equals
 */


class Filter {
  // <nodeName_ attrName_="double to compare" ... />
 public:
  enum {
    unknown = 0,
    min, // pass if data >= testValue_
    max, // pass if data <= testValue_
    equals // pass if data == testValue_
    //string_equals, // pass if data == testString_
    //string_regex, // pass if data =~ m/ $regex /;
  };
  int filterType_;
  
  Field field_; // stores the field name, field code, and associated
		// node name

  string attrName_; // name of node name's attribute to test: eg
			 // index, xcorr

  double testValue_; // ex 3425


  bool chargeSpecfication_; // is this a charge-specific filter?
  int charge_; // 0 implies all charges apply

  Filter();
  const Filter& operator=(const Filter & rhs);

  Filter(const Filter & rhs);


  Filter(const string& encodedName, const string& value); // from encoded form, eg "mSxcorr"

  string getFilterParamName(void);
  void print(ostream &out);
  void print(void); // useful for gdb enthusiasts
  string getSummary(void);

  bool operator==(const Filter& rhs) {
    if (filterType_ != rhs.filterType_) {
      return false;
    }
    
    if (!(field_ == rhs.field_)) {
      return false;
    }

    if (attrName_ != rhs.attrName_) 
      return false;

    if (testValue_ != rhs.testValue_)
      return false;

    if (chargeSpecfication_ != rhs.chargeSpecfication_)
      return false;
    
    if (charge_ != rhs.charge_)
      return false;

    return true;
  }



  
  friend class boost::serialization::access;
  // When the class Archive corresponds to an output archive, the
  // & operator is defined similar to <<.  Likewise, when the class Archive
  // is a type of input archive the & operator is defined similar to >>.
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & filterType_;
    ar & field_;
    ar & attrName_;
    ar & testValue_;
    ar & chargeSpecfication_;
    ar & charge_;
  }
  
   void write(std::ostream& out) {
 	  out << filterType_ << " ";
 	  field_.write(out);
 	  out << attrName_  << " ";
 	  out << testValue_ << " ";
 	  out << chargeSpecfication_  << " ";
 	  out << charge_ << " ";
   }

   void read(std::istream& in) {
 	  in >> filterType_;
 	  field_.read(in);
 	  in >> attrName_;
 	  in >> testValue_;
 	  in >> chargeSpecfication_;
 	  in >> charge_;
   }

  
};






/**
   FilterVec
   
   A list of filters for the same xml node.


   A simple wrapper around a vector of Filter objects; the only reason

   we do this is for speed, so we can check if the vector is empty
   without a call to size().
   

 */
class FilterVec {
 public:
  bool isEmpty_;
  vector<Filter*> filterVec_;

  FilterVec() {
    isEmpty_ = true;
  }


  FilterVec(const FilterVec & rhs);


  const FilterVec& operator=(const FilterVec & rhs);




  bool operator==(const FilterVec& rhs) const;




  friend class boost::serialization::access;
  // When the class Archive corresponds to an output archive, the
  // & operator is defined similar to <<.  Likewise, when the class Archive
  // is a type of input archive the & operator is defined similar to >>.
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & isEmpty_;
    ar & filterVec_;
  }

  void write(std::ostream& out);
 
 
  void read(std::istream& in);
  
};

#endif // header guards
