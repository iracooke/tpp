/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.
//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC
//
#ifndef PM_MULTI_HPP
#define PM_MULTI_HPP
#define IN_PM_MULTIPLE_HPP

#include "com-misc.hpp"
//#include "set.hpp"
#include <set>
#include <assert.h>

// A (pure virtual, abstract) multiple-keyword pattern matching class.

template <class t_alphabet,const int alphabetsize> class PMMultiple {
public:
	// In order to perform matching, pass an input kw_t and a
	// pointer to a function (which takes an int and a const
	// kwset_t& and returns an int) to member function
	// match(). It will call the function back with the index
	// of the character to the right of the match, and the set
	// of keywords that matched. To abort matching, the call
	// back function must return FALSE to match().
	virtual void match( const kw_t &S,
		bool callBack (int, const kwset_t&) ) = 0;

   static int Alphabetsize() {
      return alphabetsize;
   }
   static char Normalize(char c) {
      return t_alphabet::Normalize(c);
   }

};

#undef IN_PM_MULTIPLE_HPP
#endif

