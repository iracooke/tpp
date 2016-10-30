/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

#if defined(IN_BMSLNONE_CPP) || defined(INLINING)

INLINE SLNone::SLNone( const kw_t& kw ) {
	// Really nothing to do here.
	SPAREPARTS_ASSERT( c_inv() );
}

INLINE SLNone::SLNone( const SLNone& r ) {
	// Nothing to do here.
	SPAREPARTS_ASSERT( c_inv() );
}

INLINE int SLNone::skip( const kw_t& S, const int j, const int last ) const {
	SPAREPARTS_ASSERT( c_inv() );
	SPAREPARTS_ASSERT( j <= last );
	// Don't skip at all.
	return( j );
}

INLINE bool SLNone::c_inv() const {
	return( true );
}

#endif

