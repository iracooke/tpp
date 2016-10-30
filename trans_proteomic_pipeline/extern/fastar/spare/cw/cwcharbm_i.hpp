/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#include "com-opt.hpp"

#if defined(IN_CWCHARBM_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE CharBM<t_alphabet,alphabetsize>::CharBM( const CharBM<t_alphabet,alphabetsize>& r ) :
		rep( r.rep ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE int CharBM<t_alphabet,alphabetsize>::operator[]( const char a ) const {
	SPAREPARTS_ASSERT( c_inv() );
	return( rep[a] );
}

template <class t_alphabet,const int alphabetsize> INLINE bool CharBM<t_alphabet,alphabetsize>::c_inv() const {
	return( rep.c_inv() );
}

#endif

