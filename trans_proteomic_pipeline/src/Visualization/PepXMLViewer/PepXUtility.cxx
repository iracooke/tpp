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


#include <stdexcept>

#include "PepXUtility.h"
bool toBool(const string& value, bool& result) {
  bool success=true;
  result=false;
  try {

    result = atoi(value.c_str()) ? true : false ;//lexical_cast<bool>(value);
  }
  catch (bad_lexical_cast&) {
    success = false;
  }
  catch (...) {
    std::cerr << "unhandled exception (toBool)" << std::endl;
    throw;
  }

  return success;
}



bool toInt(const string& value, int& result) {
  bool success=true;
  result=0;
  try {
    result=atoi(value.c_str());//lexical_cast<int>(value);
  }
  catch (bad_lexical_cast&) {
    success = false;
  }
  catch (...) {
    std::cerr << "unhandled exception (toInt)" << std::endl;
    throw;
  }


  return success;
}



bool toLong(const string& value, long& result) {
  bool success=true;
  char* end;
  result=0;
  try {
    result=strtol(value.c_str(),&end, 10);// lexical_cast<long>(value);
  }
  catch (bad_lexical_cast&) {
    success = false;
  }
  catch (...) {
    std::cerr << "unhandled exception (toLong)" << std::endl;
    throw;
  }

  return success;
}



bool toULLong(const string& value, unsigned long long& result) {
  bool success=true;
  char *end;
  result=0;
  try {
	  result=strtoull(value.c_str(),&end, 10);//lexical_cast<unsigned long long>(value);
  }
  catch (bad_lexical_cast&) {
    success = false;
  }
  catch (...) {
    std::cerr << "unhandled exception (toLong)" << std::endl;
    throw;
  }

  return success;
}


bool toDouble(const string& value, double& result) {
  bool success=true;
  result=0;

  try {
    result=atof(value.c_str());//lexical_cast<double>(value);
  }
  catch (bad_lexical_cast&) {
    success = false;
	result=0;
  }
  catch (...) {
    std::cerr << "unhandled exception (toDouble)" << std::endl;
    throw;
  }

  return success;
}


using namespace std;



// have an example of typical usage


void 
safeReadLine(ifstream& stream, string& string) {
 try {
    if (stream.fail() || stream.bad() ) {
      cerr << "bad input stream" << endl;
      throw runtime_error("bad input stream");
    }
    getline(stream, string);
  }
  catch(const exception & e) {
    cerr << "error reading line: " << e.what() << endl;
    // rethrow
    throw runtime_error("error reading from input stream");
  }
  catch(...) {
    cerr << "unhandled exception (safeReadLine)" << endl;
    throw;
  }
}





