/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#include "com-opt.hpp"

#if defined(IN_ACGAMMA_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE Gamma<t_alphabet,alphabetsize>::Gamma( const Gamma<t_alphabet,alphabetsize>& r ) :
		rep( r.rep ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE State Gamma<t_alphabet,alphabetsize>::image( const State q, const char a ) const {
	SPAREPARTS_ASSERT( c_inv() );
	SPAREPARTS_ASSERT( FIRSTSTATE <= q && q < rep.size() );
	return( rep[q][a] );
}

template <class t_alphabet,const int alphabetsize> INLINE int Gamma<t_alphabet,alphabetsize>::size() const {
	SPAREPARTS_ASSERT( c_inv() );
	return( rep.size() );
}

template <class t_alphabet,const int alphabetsize> INLINE bool Gamma<t_alphabet,alphabetsize>::c_inv() const {
	return( rep.c_inv() );
}

#endif

