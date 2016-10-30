/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#define IN_S_TRAV_FWD_CPP
#define ALPHABET_HOME // placed here more or less at random, allows static consts to instantiate
#include "alphabet.hpp"

#include "stravfwd.hpp"

std::ostream& operator<<( std::ostream& os, const STravFWD& t ) {
	SPAREPARTS_ASSERT( t.c_inv() );
	os << "STravFWD = ()\n";
	SPAREPARTS_ASSERT( t.c_inv() );
	return( os );
}

