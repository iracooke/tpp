/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#include "com-opt.hpp"

#if defined(IN_CWDOPT_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE DOpt<t_alphabet,alphabetsize>::DOpt( const DOpt& r ) :
		rep( r.rep ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE int DOpt<t_alphabet,alphabetsize>::shift( const State q, const char a ) const {
	SPAREPARTS_ASSERT( c_inv() );
	return( rep[q][a] );
}

template <class t_alphabet,const int alphabetsize> INLINE bool DOpt<t_alphabet,alphabetsize>::c_inv() const {
	return( rep.c_inv() );
}

#endif

