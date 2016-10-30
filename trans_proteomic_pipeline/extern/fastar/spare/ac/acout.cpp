/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#define IN_ACOUT_CPP

#include "alphabet.hpp"
#include "acout.hpp"

template <class t_alphabet,const int alphabetsize> ACOutput<t_alphabet,alphabetsize>::ACOutput( const kwset_t& P,
			const FTrie<t_alphabet,alphabetsize>& t,
			const FFail<t_alphabet,alphabetsize>& f ) :
		rep( t.size() ) {
//	SPAREPARTS_ASSERT( P.c_inv() );
	SPAREPARTS_ASSERT( t.c_inv() );
	SPAREPARTS_ASSERT( rep.size() == t.size() );
	// Check that all of the sets are empty.
	auto State q;
	for( q = FIRSTSTATE; q < rep.size(); q++ ) {
		SPAREPARTS_ASSERT( rep[q].empty() );
	}

	// Do this in two phases: first the keywords, then work
	// on the trie.
//	for( int i = 0; i < P.size(); i++ ) {
	for( kwset_t::const_iterator i = P.begin(); i != P.end(); i++ ) {
		// Iterate over keyword i.
		auto State u = FIRSTSTATE;
//		auto int j;
		const char * j;
//		for( j = 0; P.iterSelect( i )[j]; j++ ) {
		for( j = i->c_str(); *j; j++ ) {
			// Note that the chars in P_i must be normalized to correspond
			// to the normalization already done in the ctor of trie t.
//			u = t.image( u, alphabetNormalize( P.iterSelect( i )[j] ) );
			u = t.image( u, alphabetNormalize( *j ) );
			// Since P and t should correspond, this can't be an
			// INVALIDSTATE.
			SPAREPARTS_ASSERT( u != INVALIDSTATE );
		}
		// u should correspond to keyword P_i.
//		SPAREPARTS_ASSERT( t.BFTdepth( u ) == j
		SPAREPARTS_ASSERT( t.BFTdepth( u ) == j-i->c_str()
//			&& j == P.iterSelect( i ).length() );
			&& *j == 0 );
//		rep.map( u ).insert( P.iterSelect( i ) );
		rep.map( u ).insert( i->c_str() );
	}
	// Now do a BFT over the trie, computing the ACOutput function.
	// There's no need to do the first level of the trie.
	for( q = t.BFTsuccessor( t.BFTfirst() );
			q != INVALIDSTATE; q = t.BFTsuccessor( q ) ) {
		rep.map( q ).insert( rep[f[q]].begin(), rep[f[q]].end() );
	}
	SPAREPARTS_ASSERT( c_inv() );
}

#if HAVE_OSTREAM_OVERLOAD
template <class t_alphabet,const int alphabetsize> std::ostream& operator<<( std::ostream& os, const ACOutput<t_alphabet,alphabetsize>& t ) {
	SPAREPARTS_ASSERT( t.c_inv() );
	os << "ACOutput = (\n" << t.rep << ")\n";
	SPAREPARTS_ASSERT( t.c_inv() );
	return( os );
}
#endif

// instantiate for the various supported alphabets
FASTAR_INSTANTIATE(ACOutput);

