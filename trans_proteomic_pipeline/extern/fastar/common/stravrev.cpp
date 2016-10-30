/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#define IN_S_TRAV_REV_CPP

#include "stravrev.hpp"

std::ostream& operator<<( std::ostream& os, const STravREV& t ) {
	SPAREPARTS_ASSERT( t.c_inv() );
	os << "STravREV = (\n" << "len = " << t.len << "\n)\n";
	SPAREPARTS_ASSERT( t.c_inv() );
	return( os );
}

