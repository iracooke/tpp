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


#ifndef _INCLUDED_PEPXUTILITY_H_
#define _INCLUDED_PEPXUTILITY_H_

#ifdef _MSC_VER // microsoft weirdness
#pragma warning(disable:4996) // don't bark about "unsafe" functions
#endif

#include <vector>
#include <string>
#include <fstream>
#include <boost/lexical_cast.hpp>

#include "common/util.h" // TPP helper funcs

#include "PepXDebug.h"

using std::string;
using std::ifstream;
using boost::lexical_cast;
using boost::bad_lexical_cast;



// namespace-wide function:
// wrap getline to throw exceptions

void safeReadLine(ifstream& stream, string& string);


// namespace-wide helper for boost smart-pointers
struct null_deleter
{
  void operator()(void const *) const
  {
  }
};



// singleton for logging/errors/messages
class PepXLog {
public:
  static PepXLog& Instance() {
    static PepXLog pepXLog;
    return pepXLog;
  }
  /* more (non-static) functions here */
  void append(const std::string& text) {
    log_.push_back(text);
  }

  // TODO: put this somewhere better
  std::vector<std::string> log_;
private:
  PepXLog(); // constructor hidden
  PepXLog(PepXLog const&); // copy constructor hidden
  PepXLog& operator=(PepXLog const&); // assignment operator hidden
  ~PepXLog(); // destructor hidden
};



// safe lexical casts; if exception (on bad cast), return the most reasonable blank value
// note: original output parameter value ("result") is erased/zeroed on failed cast
template <class inputType>
bool toString(const inputType& value, std::string& result) {
  bool success=true;
  result="";
  try {
    result=lexical_cast<std::string>(value);
  }
  catch (bad_lexical_cast&) {
    success=false;
  }
  catch (...) {
    std::cerr << "unhandled exception (toString)" << std::endl;
    throw;
  }
  return success;
}


bool toULLong(const string& value, unsigned long long& result);
bool toDouble(const string& value, double& result);
bool toLong(const string& value, long& result);
bool toBool(const string& value, bool& result); 
bool toInt(const string& value, int& result);





#endif // header guard
