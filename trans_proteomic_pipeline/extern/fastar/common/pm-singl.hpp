/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.
//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC
//
#ifndef PM_SINGLE_HPP
#define PM_SINGLE_HPP
#define IN_PM_SINGLE_HPP

#include "com-misc.hpp"

// A (pure virtual, abstract) single-keyword pattern matching class.

template <class t_alphabet,const int alphabetsize> class PMSingle {
public:
	// To do some matching, pass an input kw_t and a pointer
	// to a function, which takes an int, and returns an int, to
	// the member function match(). The member function will
	// then call back its parameter with the index of the
	// character to the right of the match. Matching can be
	// aborted by returning FALSE in the call back function.
	virtual void match( const kw_t &S,
			bool callBack (int) ) = 0;

   static int Alphabetsize() {
      return alphabetsize;
   }
   static char Normalize(char c) {
      return t_alphabet::Normalize(c);
   }

};

#undef IN_PM_SINGLE_HPP
#endif

