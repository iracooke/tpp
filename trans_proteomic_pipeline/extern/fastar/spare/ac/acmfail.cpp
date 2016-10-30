/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#define IN_ACMFAIL_CPP

#include "acmfail.hpp"
#include "alphabet.hpp"

template <class t_alphabet,const int alphabetsize> ACMachineFail<t_alphabet,alphabetsize>::ACMachineFail( const kwset_t& P ) :
		trie( new FTrie<t_alphabet,alphabetsize>( P ) ),
		tauef( *trie ),
		ff( *trie ),
		out( P, *trie, ff ) {
//	SPAREPARTS_ASSERT( P.c_inv() );
	SPAREPARTS_ASSERT( !P.empty() );
	SPAREPARTS_ASSERT( trie->c_inv() );
	// Now destroy the intermediate stuff.
	delete trie;
	trie = 0;
	SPAREPARTS_ASSERT( c_inv() );
}

#if HAVE_OSTREAM_OVERLOAD
template <class t_alphabet,const int alphabetsize> std::ostream& operator<<( std::ostream& os, const ACMachineFail<t_alphabet,alphabetsize>& t ) {
	SPAREPARTS_ASSERT( t.c_inv() );
	os << "ACMachineFail = (\n" << t.tauef << t.ff
		<< t.out << ")\n";
	SPAREPARTS_ASSERT( t.c_inv() );
	return( os );
}
#endif

// instantiate for the various supported alphabets
FASTAR_INSTANTIATE(ACMachineFail);

