/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

#if defined(IN_ACMOPT_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE ACMachineOpt<t_alphabet,alphabetsize>::ACMachineOpt( const kwset_t& P ) :
		// Make some assumptions about order of construction.
		trie( new FTrie<t_alphabet,alphabetsize>( P ) ),
		fail( new FFail<t_alphabet,alphabetsize>( *trie ) ),
		gf( *trie, *fail ),
		out( P, *trie, *fail ) {
//	SPAREPARTS_ASSERT( P.c_inv() );
	SPAREPARTS_ASSERT( !P.empty() );
	SPAREPARTS_ASSERT( trie->c_inv() );
	SPAREPARTS_ASSERT( fail->c_inv() );
	// Now get rid of the intermediates.
	delete trie;
	delete fail;
	trie = 0;
	fail = 0;
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE ACMachineOpt<t_alphabet,alphabetsize>::ACMachineOpt( const ACMachineOpt<t_alphabet,alphabetsize>& M ) :
		gf( M.gf ),
		out( M.out ),
		trie( M.trie ),
		fail( M.fail ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE ACMachineOpt<t_alphabet,alphabetsize>::~ACMachineOpt() {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE State ACMachineOpt<t_alphabet,alphabetsize>::transition( const State q, const char a ) const {
	SPAREPARTS_ASSERT( c_inv() );
	// The char a will not have been normalized yet, so do it.
	return( gf.image( q, alphabetNormalize( a ) ) );
}

template <class t_alphabet,const int alphabetsize> INLINE const kwset_t& ACMachineOpt<t_alphabet,alphabetsize>::output( const State q ) const {
	SPAREPARTS_ASSERT( c_inv() );
	return( out[q] );
}

template <class t_alphabet,const int alphabetsize> INLINE bool ACMachineOpt<t_alphabet,alphabetsize>::c_inv() const {
	return( !trie && !fail && gf.c_inv() && out.c_inv()
			&& gf.size() == out.size() );
}

#endif

