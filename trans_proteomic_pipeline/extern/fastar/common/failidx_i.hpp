/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.
//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

#if defined(IN_FAILIDX_CPP) || defined(INLINING)

INLINE FailIdx::FailIdx( const FailIdx& f ) :
		rep( f.rep ) {
	SPAREPARTS_ASSERT( c_inv() );
}

INLINE int FailIdx::operator[]( const int i ) const {
	SPAREPARTS_ASSERT( c_inv() );
	SPAREPARTS_ASSERT( 0 <= i && i < (int)rep.size() );
	return( rep[i] );
}

INLINE bool FailIdx::c_inv() const {
//	return( rep.c_inv() );
	return( true );
}

#endif

