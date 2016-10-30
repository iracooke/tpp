/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#include "com-opt.hpp"

#if defined(IN_ACEFTRIE_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE EFTrie<t_alphabet,alphabetsize>::EFTrie( const EFTrie<t_alphabet,alphabetsize>& r ) :
		rep( r.rep ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE State EFTrie<t_alphabet,alphabetsize>::image( const State q, const char a ) const {
	SPAREPARTS_ASSERT( c_inv() );
	return( rep[q][a] );
}

template <class t_alphabet,const int alphabetsize> INLINE int EFTrie<t_alphabet,alphabetsize>::size() const {
	SPAREPARTS_ASSERT( c_inv() );
	return( rep.size() );
}

template <class t_alphabet,const int alphabetsize> INLINE bool EFTrie<t_alphabet,alphabetsize>::c_inv() const {
	return( rep.c_inv() );
}

#endif

