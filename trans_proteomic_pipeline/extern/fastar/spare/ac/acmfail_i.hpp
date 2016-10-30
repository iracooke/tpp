/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

#if defined(IN_ACMFAIL_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE ACMachineFail<t_alphabet,alphabetsize>::ACMachineFail( const ACMachineFail& M ) :
		tauef( M.tauef ),
		ff( M.ff ),
		out( M.out ),
		trie( 0 ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE ACMachineFail<t_alphabet,alphabetsize>::~ACMachineFail() {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE State ACMachineFail<t_alphabet,alphabetsize>::transition( State q, const char a ) const {
	SPAREPARTS_ASSERT( c_inv() );
	// The char a will not have been normalized yet, so do it.
	// Do the linear search.
	while( tauef.image( q, alphabetNormalize( a ) ) == INVALIDSTATE ) {
		q = ff[q];
	}
	SPAREPARTS_ASSERT( c_inv() );
	return( tauef.image( q, alphabetNormalize( a ) ) );
}

template <class t_alphabet,const int alphabetsize> INLINE const kwset_t& ACMachineFail<t_alphabet,alphabetsize>::output( const State q ) const {
	SPAREPARTS_ASSERT( c_inv() );
	return( out[q] );
}

template <class t_alphabet,const int alphabetsize> INLINE bool ACMachineFail<t_alphabet,alphabetsize>::c_inv() const {
	return( !trie && tauef.c_inv() && ff.c_inv()
		&& out.c_inv() && tauef.size() == ff.size()
		&& tauef.size() == out.size() );
}

#endif

