/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

#if defined(IN_BMSLFST2_CPP) || defined(INLINING)

INLINE SLFast2::SLFast2( const SLFast2& r ) :
		lastIndex( r.lastIndex ),
		a( r.a ),
		distance( r.distance ) {
	// Nothing to do here.
	SPAREPARTS_ASSERT( c_inv() );
}

INLINE int SLFast2::skip( const kw_t& S, int j, const int last ) const {
	SPAREPARTS_ASSERT( c_inv() );
	SPAREPARTS_ASSERT( j <= last );
	// Do some skipping.
	while( j < last && S[j + lastIndex] != a ) {
		j += distance;
	}
	return( min( j, last ) );
}

INLINE bool SLFast2::c_inv() const {
	return( true );
}

#endif

