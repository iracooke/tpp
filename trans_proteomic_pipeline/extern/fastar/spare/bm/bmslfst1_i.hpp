/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

#if defined(IN_BMSLFST1_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE SLFast1<t_alphabet,alphabetsize>::SLFast1( const SLFast1<t_alphabet,alphabetsize>& r ) :
		lastIndex( r.lastIndex ),
		a( r.a ),
		distance( r.distance ) {
	// Nothing to do here.
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE int SLFast1<t_alphabet,alphabetsize>::skip( const kw_t& S, int j, const int last ) const {
	SPAREPARTS_ASSERT( c_inv() );
	SPAREPARTS_ASSERT( j <= last );
	// Do some skipping.
	// a is not normalized, for speed.
	while( j < last && S[j + lastIndex] != a ) {
		j += distance[alphabetNormalize( S[j + lastIndex] )];
	}
	return( min( j, last ) );
}

template <class t_alphabet,const int alphabetsize> INLINE bool SLFast1<t_alphabet,alphabetsize>::c_inv() const {
	return( true );
}

#endif

