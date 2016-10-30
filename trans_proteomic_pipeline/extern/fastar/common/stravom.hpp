/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#ifndef S_TRAV_OM_HPP
#define S_TRAV_OM_HPP
#define IN_S_TRAV_OM_HPP

#include "com-misc.hpp"
#include "string.hpp"
#include <assert.h>
#include <iostream>

// Generate indices to index into a kw_t.
// The OM stands for `Optimal Mismatch' (from the BM
// algorithms).

class STravOM {
public:
	STravOM( const kw_t s );

	// Traverse the kw_t.
	int traverse( int index ) const;
	int traverseInverse( int index ) const;
	bool c_inv() const;
	friend std::ostream& operator<<( std::ostream& os, const STravOM& t );
private:
	#include "stravom_p.hpp"
};

#include "stravom_i.hpp"

#undef IN_S_TRAV_OM_HPP
#endif

