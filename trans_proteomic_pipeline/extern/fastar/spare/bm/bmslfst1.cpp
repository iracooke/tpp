/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#define IN_BMSLFST1_CPP

#include "alphabet.hpp"
#include "bmslfst1.hpp"
#include <assert.h>
#include <iostream>

template <class t_alphabet,const int alphabetsize> SLFast1<t_alphabet,alphabetsize>::SLFast1( const kw_t& kw ) :
		lastIndex( kw.length() - 1 ),
		a( kw[lastIndex] ) {
	// Now compute the skip distance.
	for( char b = 0; b < alphabetsize; b++ ) {
		// Just do a linear search to find the appropriate skip.
		auto int k;
		for( k = 1; k <= lastIndex
				&& b != alphabetNormalize( kw[lastIndex - k] );
				k++ ) {
			// Intentionally empty.
		}
		// We've now found the appropriate skip.
		distance.map( b ) = k;
	}
	SPAREPARTS_ASSERT( c_inv() );
}

#if HAVE_OSTREAM_OVERLOAD
template <class t_alphabet,const int alphabetsize> std::ostream& operator<<( std::ostream& os, const SLFast1<t_alphabet,alphabetsize>& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "SLFast1<t_alphabet,alphabetsize> = (\na = " << r.a << "\ndistance = "
		<< r.distance << "\n)\n";
	return( os );
}
#endif

// instantiate for the various supported alphabets
FASTAR_INSTANTIATE(SLFast1);
