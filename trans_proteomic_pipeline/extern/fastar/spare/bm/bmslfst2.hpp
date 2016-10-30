/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#ifndef BMSLFST2_HPP
#define BMSLFST2_HPP
#define IN_BMSLFST2_HPP

#include "string.hpp"
#include <assert.h>
#include <iostream>

// One of the `skip-loop' classes. This one is the `Fast
// skip' by Hume & Sunday. This implements the sl_2 skip
// distance for the J_2 skip predicate.

class SLFast2 {
public:
	SLFast2( const kw_t& kw );
	SLFast2( const SLFast2& r );

	// The skip itself. Return the next index in S after j.
	int skip( const kw_t& S, int j, const int last ) const;
	bool c_inv() const;
	friend std::ostream& operator<<( std::ostream& os, const SLFast2& r );
private:
	#include "bmslfst2_p.hpp"
};

#include "bmslfst2_i.hpp"

#undef IN_BMSLFST2_HPP
#endif

