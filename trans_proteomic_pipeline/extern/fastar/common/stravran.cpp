/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#define IN_S_TRAV_RAN_CPP

#include "stravran.hpp"

std::ostream& operator<<( std::ostream& os, const STravRAN& t ) {
	SPAREPARTS_ASSERT( t.c_inv() );
	os << "STravRAN = (\n" << ")\n";
	SPAREPARTS_ASSERT( t.c_inv() );
	return( os );
}

