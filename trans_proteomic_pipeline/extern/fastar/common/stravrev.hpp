/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#ifndef S_TRAV_REV_HPP
#define S_TRAV_REV_HPP
#define IN_S_TRAV_REV_HPP

#include "com-misc.hpp"
#include <assert.h>
#include <iostream>

// Generate indices to index into a kw_t.
// The REV stands for `reverse'.

class STravREV {
public:
	STravREV( const kw_t &s );

	// Traverse the kw_t.
	int traverse( int const index ) const;
	int traverseInverse( const int index ) const;
	bool c_inv() const;
	friend std::ostream& operator<<( std::ostream& os, const STravREV& t );
private:
	#include "stravrev_p.hpp"
};

#include "stravrev_i.hpp"

#undef IN_S_TRAV_REV_HPP
#endif

