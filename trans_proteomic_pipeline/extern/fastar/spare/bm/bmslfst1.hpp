/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC
//

#ifndef BMSLFST1_HPP
#define BMSLFST1_HPP
#define IN_BMSLFST1_HPP

#include "com-misc.hpp"
#include "string.hpp"
#include "symbolto.hpp"
#include <assert.h>
#include <iostream>

// One of the `skip-loop' classes. This one is the `Fast
// skip' by Hume & Sunday. This implements the sl_1 skip
// distance for the J_2 skip predicate.

template <class t_alphabet,const int alphabetsize> class SLFast1 {
public:
	SLFast1( const kw_t& kw );
	SLFast1( const SLFast1<t_alphabet,alphabetsize>& r );

	// The skip itself. Return the next index in S after j.
	int skip( const kw_t& S, int j, const int last ) const;
	bool c_inv() const;
   static char Normalize(char c) {
      return t_alphabet::Normalize(c);
   }
	template <class t_alphabet2,const int alphabetsize2> friend std::ostream& operator<<( std::ostream& os, const SLFast1<t_alphabet,alphabetsize>& r );
private:
	#include "bmslfst1_p.hpp"
};

#include "bmslfst1_i.hpp"

#undef IN_BMSLFST1_HPP
#endif

