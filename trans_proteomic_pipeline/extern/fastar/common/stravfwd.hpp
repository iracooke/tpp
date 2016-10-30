/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#ifndef S_TRAV_FWD_HPP
#define S_TRAV_FWD_HPP
#define IN_S_TRAV_FWD_HPP

#include "com-misc.hpp"
#include "string.hpp"
#include <assert.h>
#include <iostream>

// Generate indices to index into a kw_t.
// The FWD stands for `forward'.

class STravFWD {
public:
	STravFWD( const kw_t &s );
	int traverse( const int index ) const;
	int traverseInverse( const int index ) const;
	bool c_inv() const;
	friend std::ostream& operator<<( std::ostream& os, const STravFWD& t );
};

#include "stravfwd_i.hpp"

#undef IN_S_TRAV_FWD_HPP
#endif

