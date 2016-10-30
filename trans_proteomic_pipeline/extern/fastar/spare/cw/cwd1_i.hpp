/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#include "com-opt.hpp"

#if defined(IN_CWD1_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE D1<t_alphabet,alphabetsize>::D1( const D1<t_alphabet,alphabetsize>& r ) :
		rep( r.rep ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE int D1<t_alphabet,alphabetsize>::operator[]( const State q ) const {
	SPAREPARTS_ASSERT( c_inv() );
	return( rep[q] );
}

template <class t_alphabet,const int alphabetsize> INLINE bool D1<t_alphabet,alphabetsize>::c_inv() const {
	return( rep.c_inv() );
}

#endif

