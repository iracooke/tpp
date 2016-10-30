/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#define IN_ACEFTRIE_CPP

#include "aceftrie.hpp"
#include "alphabet.hpp"

template <class t_alphabet,const int alphabetsize> EFTrie<t_alphabet,alphabetsize>::EFTrie( const FTrie<t_alphabet,alphabetsize>& t ) :
		rep( t.size() ) {
	SPAREPARTS_ASSERT( t.c_inv() );
	// Construct the image of the forward trie t.
	for( State q = FIRSTSTATE; q < rep.size(); q++ ) {
		for( char a = 0; a < alphabetsize; a++ ) {
			rep.map( q ).map( a ) = t.image( q, a );
		}
	}
	// Construct the FIRSTSTATE auto-cycle.
	for( char a = 0; a < alphabetsize; a++ ) {
		if( rep[FIRSTSTATE][a] == INVALIDSTATE ) {
			rep.map( FIRSTSTATE ).map( a ) = FIRSTSTATE;
		}
	}
	SPAREPARTS_ASSERT( c_inv() );
}

#if HAVE_OSTREAM_OVERLOAD
template <class t_alphabet,const int alphabetsize> std::ostream& operator<<( std::ostream& os, const EFTrie<t_alphabet,alphabetsize>& t ) {
	SPAREPARTS_ASSERT( t.c_inv() );
	os << "EFTrie<t_alphabet,alphabetsize> = (\n"  << t.rep << ")\n";
	SPAREPARTS_ASSERT( t.c_inv() );
	return( os );
}
#endif

// instantiate for the various supported alphabets
FASTAR_INSTANTIATE(EFTrie);
