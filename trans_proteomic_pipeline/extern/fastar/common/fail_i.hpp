/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.
#include "com-opt.hpp"

template<class T, class t_alphabet, const int alphabetsize>
Fail<T,t_alphabet, alphabetsize>::Fail( const Trie<T,t_alphabet, alphabetsize>& r ) :
		rep( r.size() ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	// Do the first BFT level of the failure function separately
	// from the other levels.
	rep.map( FIRSTSTATE ) = INVALIDSTATE;
	// Iterate over the alphabet, and do symbols that have out-trans
	// from the FIRSTSTATE.
	for( char a = 0; a < alphabetsize; a++ ) {
		if( r.image( FIRSTSTATE, a ) != INVALIDSTATE ) {
			rep.map( r.image( FIRSTSTATE, a ) ) = FIRSTSTATE;
		}
	}
	// Now perform a BFT over the Trie to get the failure function
	// by doing the remaining layers.
	for( State q = r.BFTsuccessor( r.BFTfirst() ); q != INVALIDSTATE;
			q = r.BFTsuccessor( q ) ) {
		// Go over all the out-transitions of q and setup the next
		// level of the failure function.
		for( char a = 0; a < alphabetsize; a++ ) {
			if( r.image( q, a ) != INVALIDSTATE ) {
				// Now go through the successive failures of q, to
				// find one with an out-transition on a.
				auto State qfail = rep[q];
				while( r.image( qfail, a ) == INVALIDSTATE
						&& qfail != FIRSTSTATE ) {
					qfail = rep[qfail];
				}
				// qfail is it, or we've gotten to qfail == FIRSTSTATE
				// and there's still no out-transition on a.
				if( r.image( qfail, a ) != INVALIDSTATE ) {
					rep.map( r.image( q, a ) ) = r.image( qfail, a );
				} else {
					SPAREPARTS_ASSERT( qfail == FIRSTSTATE
						&& r.image( qfail, a ) == INVALIDSTATE );
					rep.map( r.image( q, a ) ) = FIRSTSTATE;
				}
			}
		}
	}
	SPAREPARTS_ASSERT( c_inv() );
}

template<class T, class t_alphabet, const int alphabetsize>
INLINE Fail<T,t_alphabet, alphabetsize>::Fail( const Fail<T,t_alphabet, alphabetsize>& r ) :
		rep( r.rep ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template<class T, class t_alphabet, const int alphabetsize>
INLINE State Fail<T,t_alphabet, alphabetsize>::operator[]( const State q ) const {
	SPAREPARTS_ASSERT( c_inv() );
	SPAREPARTS_ASSERT( FIRSTSTATE <= q && q < size() );
	return( rep[q] );
}

template<class T, class t_alphabet, const int alphabetsize>
INLINE int Fail<T,t_alphabet, alphabetsize>::size() const {
	SPAREPARTS_ASSERT( c_inv() );
	return( rep.size() );
}

template<class T, class t_alphabet, const int alphabetsize>
INLINE bool Fail<T,t_alphabet, alphabetsize>::c_inv() const {
	return( rep.c_inv() );
}

template<class T, class t_alphabet, const int alphabetsize>
std::ostream& operator<<( std::ostream& os, const Fail<T,t_alphabet, alphabetsize>& t ) {
	SPAREPARTS_ASSERT( t.c_inv() );
	os << "Fail<T,t_alphabet, alphabetsize> = (\n" << t.rep << ")\n";
	SPAREPARTS_ASSERT( t.c_inv() );
	return( os );
}
