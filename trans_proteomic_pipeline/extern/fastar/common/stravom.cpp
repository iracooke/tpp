/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#define IN_S_TRAV_OM_CPP

#include "stravom.hpp"

std::ostream& operator<<( std::ostream& os, const STravOM& t ) {
	SPAREPARTS_ASSERT( t.c_inv() );
	os << "STravOM = (\n" << ")\n";
	SPAREPARTS_ASSERT( t.c_inv() );
	return( os );
}

