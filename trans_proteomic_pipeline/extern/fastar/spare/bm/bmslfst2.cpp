/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#define IN_BMSLFST2_CPP

#include "com-misc.hpp"
#include "bmslfst2.hpp"
#include <assert.h>
#include <iostream>

SLFast2::SLFast2( const kw_t& kw ) :
		lastIndex( kw.length() - 1 ),
		a( kw[lastIndex] ) {
	// Now compute the skip distance.
	auto int k;
	for( k = 1; k <= lastIndex
			&& kw[lastIndex] == kw[lastIndex - k]; k++ ) {
		// Intentionally empty.
	}
	distance = k;

	SPAREPARTS_ASSERT( c_inv() );
}

#if HAVE_OSTREAM_OVERLOAD
std::ostream& operator<<( std::ostream& os, const SLFast2& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "SLFast2 = (\na = " << r.a << "\ndistance = "
		<< r.distance << "\n)\n";
	return( os );
}
#endif
