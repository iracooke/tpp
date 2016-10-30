/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

#if defined(IN_S_TRAV_OM_CPP) || defined(INLINING)

INLINE STravOM::STravOM( const kw_t s ) {
	SPAREPARTS_ASSERT( c_inv() );
}

INLINE bool STravOM::c_inv() const {
	return( true );
}

#endif

