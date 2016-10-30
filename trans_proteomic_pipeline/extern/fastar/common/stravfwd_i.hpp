/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

#if defined(IN_S_TRAV_FWD_CPP) || defined(INLINING)

INLINE STravFWD::STravFWD( const kw_t &s ) {
	SPAREPARTS_ASSERT( c_inv() );
}

INLINE int STravFWD::traverse( const int index ) const {
	return( index );
}

INLINE int STravFWD::traverseInverse( const int index ) const {
	// The forward one happens to be its own inverse.
	return( traverse( index ) );
}

INLINE bool STravFWD::c_inv() const {
	return( true );
}

#endif

