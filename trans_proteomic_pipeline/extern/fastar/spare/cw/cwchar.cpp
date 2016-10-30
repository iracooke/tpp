/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#define IN_CWCHAR_CPP

#include "cwchar.hpp"

template <class t_alphabet,const int alphabetsize> CharCW<t_alphabet,alphabetsize>::CharCW( const RTrie<t_alphabet,alphabetsize>& t ) {
	// First set everything at infinity.
	for( char a = 0; a < alphabetsize; a++ ) {
		rep.map( a ) = PLUSINFINITY;
	}
	// Now a BFT of the trie; start on second level.
	for( State q = t.BFTsuccessor( t.BFTfirst() );
			q != INVALIDSTATE; q = t.BFTsuccessor( q ) ) {
		for( char a = 0; a < alphabetsize; a++ ) {
			if( t.image( q, a ) != INVALIDSTATE ) {
				rep.map( a ) = min( rep[a], t.BFTdepth( q ) );
			}
		}
	}
	SPAREPARTS_ASSERT( c_inv() );
}

#if HAVE_OSTREAM_OVERLOAD
template <class t_alphabet,const int alphabetsize> std::ostream& operator<<( std::ostream& os, const CharCW<t_alphabet,alphabetsize>& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "CharCW<t_alphabet,alphabetsize> = (\n" << r.rep << ")\n";
	SPAREPARTS_ASSERT( r.c_inv() );
	return( os );
}
#endif

// instantiate for the various supported alphabets
FASTAR_INSTANTIATE(CharCW);

