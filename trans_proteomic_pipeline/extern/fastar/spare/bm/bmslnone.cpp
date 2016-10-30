/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#define IN_BMSLNONE_CPP

#include "com-misc.hpp"
#include "bmslnone.hpp"
#include <assert.h>
#include <iostream>

#if HAVE_OSTREAM_OVERLOAD
std::ostream& operator<<( std::ostream& os, const SLNone& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "SLNone = ()\n";
	return( os );
}
#endif
