/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.
//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#define IN_FAILIDX_CPP

#include "failidx.hpp"
#include "com-misc.hpp"
#include "com-io.hpp"

FailIdx::FailIdx( const kw_t &kw ) :
		rep( kw.length() + 1 ) {
	// rep is one larger than the length of the keyword so
	// that it can encode all of pref( kw ).
//	rep.at( 0 ) = -1;
	rep[0] = -1;
	// The rest of the failure function is only constructed
	// if the keyword is not the empty kw_t.
	if( kw.length() ) {
		// Set up the first letter.
		auto int v = 1;

//		rep.at( v ) = 0;
		rep[v] = 0;

		// Do the remaining letters.
		while( v < (int)kw.length() ) {
			// Now do the linear search.
			auto int u = rep[v];
			
			while( kw[u] != kw[v] && u != 0 ) {
				u = rep[u];
			}
			if( kw[u] == kw[v++] ) {
//				rep.at( v ) = u + 1;
				rep[v] = u + 1;
			} else {
//				rep.at( v ) = 0;
				rep[v] = 0;
			}
		}
	}
	SPAREPARTS_ASSERT( c_inv() );
}

std::ostream& operator<<( std::ostream& os, const FailIdx& t ) {
	SPAREPARTS_ASSERT( t.c_inv() );
	os << "FailIdx = (\n" << t.rep << ")\n";
	SPAREPARTS_ASSERT( t.c_inv() );
	return( os );
}

