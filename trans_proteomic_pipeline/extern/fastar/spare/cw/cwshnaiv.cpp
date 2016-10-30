/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#define IN_CWSHNAIV_CPP

#include "cwshnaiv.hpp"

#if HAVE_OSTREAM_OVERLOAD
std::ostream& operator<<( std::ostream& os, const CWShiftNaive& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "CWShiftNaive = ()\n";
	SPAREPARTS_ASSERT( r.c_inv() );
	return( os );
}
#endif

// instantiate for the various supported alphabets
FASTAR_INSTANTIATE(CWShiftNaive);
