/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

#if defined(IN_BMSLSFC_CPP) || defined(INLINING)

INLINE SLSFC::SLSFC( const kw_t& kw ) :
		a( kw[0] ) {
	// Really nothing to do here.
	SPAREPARTS_ASSERT( c_inv() );
}

INLINE SLSFC::SLSFC( const SLSFC& r ) :
		a( r.a ) {
	// Nothing to do here.
	SPAREPARTS_ASSERT( c_inv() );
}

INLINE int SLSFC::skip( const kw_t& S, int j, const int last ) const {
	SPAREPARTS_ASSERT( c_inv() );
	SPAREPARTS_ASSERT( j <= last );
	// Do some skipping.
	while( j < last && S[j] != a ) {
		j++;
	}
	SPAREPARTS_ASSERT( j <= last );
	return( j );
}

INLINE bool SLSFC::c_inv() const {
	return( true );
}

#endif

