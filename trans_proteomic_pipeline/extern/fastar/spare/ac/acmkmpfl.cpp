/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#define IN_ACMKMPFAIL_CPP

#include "alphabet.hpp"
#include "acmkmpfl.hpp"
#include "tries.hpp"
#include "fails.hpp"

#if HAVE_OSTREAM_OVERLOAD
std::ostream& operator<<( std::ostream& os, const ACMachineKMPFail& t ) {
	SPAREPARTS_ASSERT( t.c_inv() );
	os << "ACMachineKMPFail = (\n" 
      << t.tau 
      << t.ff
		<< t.out 
      << ")\n";
	SPAREPARTS_ASSERT( t.c_inv() );
	return( os );
}
#endif

// instantiate for the various supported alphabets
FASTAR_INSTANTIATE(ACMachineKMPFail);
