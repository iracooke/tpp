/* This is a SWIG interface file for exposing TPP library function points to perl via
   SWIG (could also be used for Python, Java, etc).

   There are two sections - declarations of things you want literally included in
   the wrapper code, and things you want magically turned into perl (or python etc).
   These lists tend to be the same, but its possible that you'd need to include some
   declarations of private stuff in the first list but not the second.
   
   Note that if there are things in your headers that you don't want SWIG to deal
   with, just surround them #ifndef SWIG / #endif .
   
   by Brian Pratt 2008 Insilicos LLC (see the TPP code being wrapped for license info)
*/  


/* set the module name - this needs to match the name of the DLL containing the
   the code that the perl module will look to for its inner workings */
%module tpplib_perl


/* STL support - generates smarter wrappers for functions that return std::string etc */
%include "std_deque.i" 
%include "std_list.i" 
%include "std_map.i" 
%include "std_pair.i"
%include "std_string.i" 
%include "std_vector.i" 

%{
  /* everything in this section (including this comment) appears in the generated
     wrapper code, so here's where you declare the C/C++ functions that the wrapper
     will find in the DLL */

#include "../common/util.h"
#include "../Parsers/ramp/ramp.h"

%}

/* and here put declarations of things you want SWIG to express in the target language 
  (tends to be the same list as above) */
   
%include "../common/util.h"
%include "../Parsers/ramp/ramp.h"
