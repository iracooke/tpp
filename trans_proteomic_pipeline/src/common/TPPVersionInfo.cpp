/*

Copyright (C) 2014 Institute for Systems Biology

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

Institute for Systems Biology
401 Terry Avenue North
Seattle, WA  98109  USA

*/

/*
   Version and build information are compiled into TPP programs using
   this source file.  It expects that the macros TPP_BUILD_DATE, 
   TPP_BUILD_SVNREV, and TPP_BUILD_ARCH are defined at compile time.
   (Usually as -DTPP_BUILD_ARCH=linux).  The major, minor, and revision
   come from the header.
*/
#include "TPPVersion.h"

#ifndef TPP_BUILD_ID
#error "TPP_BUILD_ID was not defined"
#endif
#ifndef TPP_BUILD_ARCH
#error "TPP_BUILD_ARCH must be defined"
#endif

// Two step macro mechansim for stringification of macros
#define STR(macro) QUOTE(macro)
#define QUOTE(name) #name

const char *szTPPVersionInfo = "TPP v"
                               STR(TPP_MAJOR_VERSION_NUMBER) "." 
                               STR(TPP_MINOR_VERSION_NUMBER) "."
                               STR(TPP_REV_VERSION_NUMBER)   ""
                               TPP_RELEASE_TYPE              " "
                               TPP_RELEASE_NAME              ", Build "
                               STR(TPP_BUILD_ID)             " "
                               "(" STR(TPP_BUILD_ARCH) ")";
