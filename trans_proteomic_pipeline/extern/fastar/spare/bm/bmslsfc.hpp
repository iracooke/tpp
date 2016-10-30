/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#ifndef BMSLSFC_HPP
#define BMSLSFC_HPP
#define IN_BMSLSFC_HPP

#include "com-misc.hpp"
#include "string.hpp"
#include <assert.h>
#include <iostream>

// One of the `skip-loop' classes. This one is the `Search
// First Character' skip-loop. Implements the sl_1 (equiv.
// sl_2) skip distances for the J_1 skip predicate.

class SLSFC {
public:
	SLSFC( const kw_t& kw );
	SLSFC( const SLSFC& r );

	// The skip itself. Return the next index in S after j.
	int skip( const kw_t& S, int j, const int last ) const;
	bool c_inv() const;
	friend std::ostream& operator<<( std::ostream& os, const SLSFC& r );
private:
	#include "bmslsfc_p.hpp"
};

#include "bmslsfc_i.hpp"

#undef IN_BMSLSFC_HPP
#endif

