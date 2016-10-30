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



/** @file PepXFilter.cxx
    @brief see PepXFilter.h
*/

#include <stdexcept>

#include "PepXUtility.h"

#include "PepXFilter.h"

using std::runtime_error;
using std::cout;
using std::endl;




Filter::Filter() :
  filterType_(Filter::unknown),
  testValue_(0),
  chargeSpecfication_(false) 
{
}




// from encoded form, eg "FmSbays_score", "f2MSxcorr"
Filter::Filter(const string& encodedName, const string& value) {

  // TODO: what to do if passed bad value?
  toDouble(value, testValue_);

  // TODO: check minimum length of input string

  int i = 0;
  char chargeType=encodedName[i]; // 'f' or 'F'
  
  // checking charge?
  if (chargeType == 'f') {
    chargeSpecfication_ = true;
  }
  else {
    chargeSpecfication_ = false;
    charge_ = 0;
  }

  char filterCodeChar=encodedName[++i]; // 'm' or 'M', etc
  switch(filterCodeChar) {
  case 'm':
    filterType_ = Filter::min;
    break;
  case 'M':
    filterType_ = Filter::max;
    break;
  case 'E':
    filterType_ = Filter::equals;
    break;
  default :
    filterType_ = Filter::unknown;
    break;
  }

  if (chargeSpecfication_) {
    toInt(encodedName.substr(++i),charge_);
  }



  // set the field(type, name, nodename, isValueNode) from the encoded
  // fieldname (eg 'S', "xcorr")
  char fieldCode = encodedName[++i];
  field_.set(encodedName.substr(++i), fieldCode);


  attrName_ = encodedName.substr(i);


  // modify special case: asap ratio
  if (attrName_ == "asapratio") {
    attrName_ = "mean";
  }


  // modify special case: xpress ratio
  if (attrName_ == "xpress") {
    attrName_ = "decimal_ratio";
  }


}





string
Filter::getFilterParamName(void) {
  string name;
  switch (filterType_) {
  case Filter::min:
    name += 'm';
    break;
  case Filter::max:
    name += 'M';
    break;

  case Filter::equals:
    name += 'E';
    break;

  case Filter::unknown:
    name += '?';
    break;

  default:
    cout << "unknown filter type" << endl;
    throw runtime_error("unknown filter type");
    break;
  }
  

  string t;
  if (chargeSpecfication_ ) {
    switch (charge_){
    case 1:
      name += "1";
      break;
    case 2:
      name += "2";
      break;
    case 3:
      name += "3";
      break;
    case 0:
      name += "A";
    default:
      cout << "unknown filter charge " << charge_ << endl;
      toString(charge_,t);
      throw runtime_error(string("unknown filter charge ") + t);
    }
  }


  name += field_.getFieldParamName();
  
  return name;
}

void Filter::print(void) { // useful for gdb enthusiasts
	print(cout);
}

void
Filter::print(ostream &out) {
  out << "filter for node " << field_.nodeName_ << ",";

  if (chargeSpecfication_) {
    out << "+" << charge_ << "only, " << endl;
  }


  if (field_.isValueNode_) {
    out << "name=" << attrName_ << ": ";
    out << "value ";
  } else {
    out << attrName_ << " ";
  }


  switch (filterType_) {
  case Filter::min:
    out << ">=";
    break;
  case Filter::max:
    out << "<=";
    break;
  case Filter::equals:
    out << "==";
    break;
  }
  out << testValue_;
  out << endl;
}



string
Filter::getSummary(void) {
  string summary;

 
  summary += field_.fieldName_ + " ";

  if (chargeSpecfication_) {
    summary +=  "( +";
    string t;
    toString(charge_, t);
    summary += t;
    summary += " ions only) ";
  }




  switch (filterType_) {
  case Filter::min:
    summary +=  ">=";
    break;
  case Filter::max:
    summary  += "<=";
    break;
  case Filter::equals:
    summary +=  "==";
    break;
  }
  string t;
  toString(testValue_, t);
  summary += t;
  
  return summary;

}








const Filter& 
Filter::operator=(const Filter & rhs) {
  filterType_ = rhs.filterType_;
  field_ = rhs.field_;
  attrName_ = rhs.attrName_;
  testValue_ = rhs.testValue_;
  chargeSpecfication_ = rhs.chargeSpecfication_;
  charge_ = rhs.charge_;
  return *this;
}


Filter::Filter(const Filter & rhs) {
  filterType_ = rhs.filterType_;
  field_ = rhs.field_;
  attrName_ = rhs.attrName_;
  testValue_ = rhs.testValue_;
  chargeSpecfication_ = rhs.chargeSpecfication_;
  charge_ = rhs.charge_;
}






// TODO:
// currently, we assume the same order for both FilterVec filter lists--
// this is not necessarily the case!

// ignores stored nodes with 0 filters
bool FilterVec::operator==(const FilterVec& rhs) const {
  if (isEmpty_ != rhs.isEmpty_)
    return false;

    
  if (filterVec_.size() != rhs.filterVec_.size())
    return false;
    
  bool result = true;
  for (unsigned int i = 0 ; i<filterVec_.size(); i++) {
    if (! (*(filterVec_[i]) == *(rhs.filterVec_[i]))) {
      result = false;
      break;
    }
  }
  if (result == false) 
    return false;

  return true;

}




FilterVec::FilterVec(const FilterVec & rhs) {
  filterVec_.clear();
  isEmpty_ = rhs.isEmpty_;
  if (!rhs.isEmpty_) {
    for (unsigned int i = 0 ; i<rhs.filterVec_.size(); i++) {
      Filter* fp;
      fp = rhs.filterVec_[i];
      filterVec_.push_back(new Filter(*fp));
    }
  }
}


const FilterVec& 
FilterVec::operator=(const FilterVec & rhs) {
  filterVec_.clear();
  isEmpty_ = rhs.isEmpty_;
  if (!rhs.isEmpty_) {
    for (unsigned int i = 0 ; i<rhs.filterVec_.size(); i++) {
      Filter* fp;
      fp = rhs.filterVec_[i];
      filterVec_.push_back(new Filter(*fp));
    }
  }
  return *this;
}

void FilterVec::read(std::istream& in) {
	  in >> isEmpty_;
	  size_t size;
	  in >> size;
	  for (int j=0; j < (int)size; j++) {
		  Filter* fil = new Filter();
		  (*fil).read(in);
		  filterVec_.push_back(fil);
	}
}

void FilterVec::write(std::ostream& out) {
	out << isEmpty_ << " ";
	out << filterVec_.size() << " ";
	for (int i =0; i < (int)filterVec_.size(); i++)
		(*filterVec_[i]).write(out);

}