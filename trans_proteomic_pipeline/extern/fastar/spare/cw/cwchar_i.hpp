/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#include "com-opt.hpp"

#if defined(IN_CWCHAR_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE CharCW<t_alphabet,alphabetsize>::CharCW( const CharCW<t_alphabet,alphabetsize>& r ) :
		rep( r.rep ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE int CharCW<t_alphabet,alphabetsize>::shift( const char a, const int z ) const {
	SPAREPARTS_ASSERT( c_inv() );
	return( rep[a] - z );
}

template <class t_alphabet,const int alphabetsize> INLINE bool CharCW<t_alphabet,alphabetsize>::c_inv() const {
	return( rep.c_inv() );
}

#endif

