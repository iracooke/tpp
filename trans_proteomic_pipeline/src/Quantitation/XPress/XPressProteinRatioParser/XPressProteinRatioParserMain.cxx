/*

Program       : XPressProteinRatioParser                                                   
Author        : J.Eng and Andrew Keller <akeller@systemsbiology.org>, Robert Hubley, and 
                open source code                                                       
Date          : 11.27.02 

Primary data object holding all mixture distributions for each precursor ion charge

Copyright (C) 2003 Andrew Keller

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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Andrew Keller
Institute for Systems Biology
401 Terry Avenue North 
Seattle, WA  98109  USA
akeller@systemsbiology.org

*/

#include "XPressProteinRatioParser.h"

#include "common/TPPVersion.h" // contains version number, name, revision

#include <fstream>

int main(int argc, char** argv) {
  hooks_tpp handler(argc,argv); // set up install paths etc

   cout << " " << argv[0] << " (" << szTPPVersionInfo << ")" << std::endl;
   if (argc < 2)
   {
      cout << "usage:  XPressProteinRatioParser <protxml_file>" << endl;
      exit(1);
   }

  XPressProteinRatioParser *p = new XPressProteinRatioParser(argv[1],(argc>2)?argv[2]:NULL);
  delete p;


  return 0;
}
