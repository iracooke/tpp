/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

#if defined(IN_S_TRAV_REV_CPP) || defined(INLINING)

INLINE STravREV::STravREV( const kw_t &s ) :
		len( s.length() ) {
	SPAREPARTS_ASSERT( c_inv() );
}

INLINE int STravREV::traverse( int const index ) const {
	SPAREPARTS_ASSERT( 0 <= index && index < len );
	return( len - index - 1 );
}

INLINE int STravREV::traverseInverse( const int index ) const {
	// The reverse one happens to be its own inverse.
	return( traverse( index ) );
}

INLINE bool STravREV::c_inv() const {
	return( 0 <= len );
}

#endif

