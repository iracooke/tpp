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



#ifndef _INCLUDED_PEPXVIEWER_H_
#define _INCLUDED_PEPXVIEWER_H_


#include <string>

#include "PepXOptions.h"
#include "PepXParser.h"

using std::string;



/** @file 
    @brief
*/
class PepXViewer {


public:
  enum {
    commandLine,
    CGI,
  }; // modes
  int mode_;

protected:
  



  Options options_; // options, from CGI or command-line

  PepXParser pepxmlParser_; // the actual parsing object


  // html rewriting
  bool inBlock_;
  bool evalBlock_;

  

protected:
  void usage(void);  
  void prepareParameters(void);

  void printHTMLColumnHeaders(void);
  void printHTMLDataRows(void);

  bool processHTMLTemplate(const string& templateFileName);


  void writeSpreadSheet(const string& spreadsheetFileName);


  

public:
  PepXViewer();
  ~PepXViewer();

  void reset(void);
  void error(const string& msg);


  void parseCommandLineOptions(int argc, char** argv);
  void parseCGIOptions(const string & queryString);
  
  
  // the main routine
  void run(void);


};





#endif // header guards
