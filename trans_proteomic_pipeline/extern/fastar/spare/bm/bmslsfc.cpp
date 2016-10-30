/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#define IN_BMSLSFC_CPP

#include "bmslsfc.hpp"
#include <assert.h>
#include <iostream>

#if HAVE_OSTREAM_OVERLOAD
std::ostream& operator<<( std::ostream& os, const SLSFC& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "SLSFC = ()\n";
	return( os );
}
#endif
