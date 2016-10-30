/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#define IN_ACGAMMA_CPP

#include "alphabet.hpp"
#include "com-misc.hpp"
#include "acgamma.hpp"
#include <vector>

template <class t_alphabet,const int alphabetsize> Gamma<t_alphabet,alphabetsize>::Gamma( const FTrie<t_alphabet,alphabetsize>& t, const FFail<t_alphabet,alphabetsize>& f ) :
		rep( t.size() ) {
	SPAREPARTS_ASSERT( t.c_inv() );
	SPAREPARTS_ASSERT( f.c_inv() );
	// The failure function must be related to the trie.
	SPAREPARTS_ASSERT( t.size() == f.size() );
	SPAREPARTS_ASSERT( rep.size() == t.size() );

	// Perform a (more or less random) traversal of trie t,
	// constructing the rep along the way. (Whether it's BFT or
	// DFT is not relevant.)
	for( State q = FIRSTSTATE; q < rep.size(); q++ ) {
		// Iterate over the alphabet, constructing the transitions.
		for( char a = 0; a < alphabetsize; a++ ) {
			// Do the linear search.
			auto State qfail = q;
			while( t.image( qfail, a ) == INVALIDSTATE && qfail
					!= FIRSTSTATE) {
				// Use the failure function as the predecessor function.
				qfail = f[qfail];
			}
			SPAREPARTS_ASSERT( t.image( qfail, a ) != INVALIDSTATE
				|| qfail == FIRSTSTATE );
			if( t.image( qfail, a ) != INVALIDSTATE ) {
				rep.
    			map( q ).
    			map( a ) = t.image( qfail, a );
			} else {
				SPAREPARTS_ASSERT( qfail == FIRSTSTATE );
				rep.map( q ).map( a ) = FIRSTSTATE;
			}
		}
	}
	SPAREPARTS_ASSERT( c_inv() );
}

#if HAVE_OSTREAM_OVERLOAD
std::ostream& operator<<( std::ostream& os, const Gamma& t ) {
	SPAREPARTS_ASSERT( t.c_inv() );
	os <<  "Gamma = (\n"  <<   t.rep    <<      ")\n";
	SPAREPARTS_ASSERT( t.c_inv() );
	return( os );
}
#endif

// instantiate for the various supported alphabets
FASTAR_INSTANTIATE(Gamma);

