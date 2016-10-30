/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#define IN_CWCHARBM_CPP

#include "cwcharbm.hpp"

template <class t_alphabet,const int alphabetsize> CharBM<t_alphabet,alphabetsize>::CharBM( const RTrie<t_alphabet,alphabetsize>& t, const kwset_t& P ) {
	// Find out the length of the shortest keyword.
	auto int mP = PLUSINFINITY;
//	for( int i = 0; i < P.size(); i++ ) {
	for( kwset_t::const_iterator i = P.begin(); i != P.end(); i++ ) {
		mP = min( mP, i->length() );
	}
	// First set everything at the length of the shortest kw.
	for( char a = 0; a < alphabetsize; a++ ) {
		rep.map( a ) = mP;
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
template <class t_alphabet,const int alphabetsize> std::ostream& operator<<( std::ostream& os, const CharBM<t_alphabet,alphabetsize>& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "CharBM<t_alphabet,alphabetsize> = (\n" << r.rep << ")\n";
	SPAREPARTS_ASSERT( r.c_inv() );
	return( os );
}
#endif

// instantiate for the various supported alphabets
FASTAR_INSTANTIATE(CharBM);
