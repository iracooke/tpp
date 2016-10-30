/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#ifndef BMSLNONE_HPP
#define BMSLNONE_HPP
#define IN_BMSLNONE_HPP

#include "string.hpp"
#include <assert.h>
#include <iostream>

// One of the `skip-loop' classes. This one provides no
// skip distance (a shift of 0).

class SLNone {
public:
	SLNone( const kw_t& kw );
	SLNone( const SLNone& r );

	// The skip itself. Return the next index in S after j.
	int skip( const kw_t& S, const int j, const int last ) const;
	bool c_inv() const;
	friend std::ostream& operator<<( std::ostream& os, const SLNone& r );
private:
};

#include "bmslnone_i.hpp"

#undef IN_BMSLNONE_HPP
#endif

