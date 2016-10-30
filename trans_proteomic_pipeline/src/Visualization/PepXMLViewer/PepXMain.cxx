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


#include <cstdlib>
#include <cstring>
#include <exception>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/exception.hpp>

#include "PepXUtility.h"
#include "PepXViewer.h"

using namespace std;




int main(int argc, char** argv) {
  hooks_tpp(argc,argv); // handle install dir issues, etc
  // JLS: deprecated
  //boost::filesystem::path::default_name_check( boost::filesystem::native ); // default is portable_name, fails in win32

  PepXViewer pepxmlViewer;  
  try {


    /* determine invocation mode */

    char* envTest = getenv("REQUEST_METHOD");
    if (envTest == NULL || argc>1) {
      /* --command-line mode-- */
      pepxmlViewer.mode_ = PepXViewer::commandLine;
      debug(debugout << "cmdline mode");
      pepxmlViewer.parseCommandLineOptions(argc, argv);
    }


    else {
      /* --CGI mode-- */


      // no matter what happens, start with the correct header
      cout << "Content-type: text/html" << endl << endl;


      // check form mode
      if (! (strcmp(envTest, "GET")==0)) {
	pepxmlViewer.error(string("error: expected GET method, received ") + envTest);
      }
      else {
	// CGI looks good
	debug(debugout << "cgi mode");
	envTest = getenv("QUERY_STRING");
	if (envTest != NULL) {
	  // query data; override default settings
	  pepxmlViewer.parseCGIOptions(envTest);
	}
	// default options are used unless overrided by query string
      }
    }
    // either command-line or CGI;
    // either way, we've parsed any arguments and are ready to go
    pepxmlViewer.run();
  }
  catch (const std::exception& e) {
    pepxmlViewer.error(e.what());
  }
  catch (...) {
    pepxmlViewer.error("unhandled exception");
  }

  return 0;
}
