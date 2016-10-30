/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#include "com-opt.hpp"

#if defined(IN_CWD2_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE D2<t_alphabet,alphabetsize>::D2( const D2<t_alphabet,alphabetsize>& r ) :
		rep( r.rep ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE int D2<t_alphabet,alphabetsize>::operator[]( const State u ) const {
	SPAREPARTS_ASSERT( c_inv() );
	return( rep[u] );
}

template <class t_alphabet,const int alphabetsize> INLINE bool D2<t_alphabet,alphabetsize>::c_inv() const {
	return( rep.c_inv() );
}

#endif

