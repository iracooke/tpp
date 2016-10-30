/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"
#include "com-misc.hpp"
#include <algorithm>

//
// stuff for nearly-zero-copy trie building 
// (formerly it deep-copied the entire keyword set in order to sort it)
// 7-17-06 Brian Pratt Insilicos LLC 
//


template<class T,class t_alphabet,const int alphabetsize>
Trie<T,t_alphabet, alphabetsize>::Trie( const kwset_t& P ) :
		rep( 1 ),
		depth( 1 ) {
//	SPAREPARTS_ASSERT( P.c_inv() );
	SPAREPARTS_ASSERT( !P.empty() );
	// Copy P to vector P2 and sort in ascending order of length
	std::vector<kw_t> P2; // zero copy work 7-17-06 BSP Insilicos LLC
	for(kwset_t::const_iterator ii = P.begin(); ii != P.end(); ii++) {
		P2.push_back(*ii);
	}
   std::sort(P2.begin(), P2.end(), shorter);

	// The trie now has at least the epsilon (empty kw_t) in it.
	// Set all of the epsilon out-transitions to INVALIDSTATE.
   //for( int a = 0; a < alphabetsize; a++ ) {
	//	rep.map( FIRSTSTATE ).map( a ) = INVALIDSTATE;
	//}
   rep.map( FIRSTSTATE ).invalidate();

	// Make sure that the FIRSTSTATE is at depth 0.
	depth.map( FIRSTSTATE ) = 0;

	// Iterate over all of the keywords.
//	for( int i = 0; i < P.size(); i++ ) {
	for( std::vector<kw_t>::const_iterator i = P2.begin(); i != P2.end(); i++ ) {
		auto State q = FIRSTSTATE;
		// Create a traverser for the current keyword.
//		auto T trav( P.iterSelect( i ) );
		auto T trav( i->c_str() );

		// Iterate over the letters of each keyword.
//		for( int j = 0; P.iterSelect( i )[j]; j++ ) {
		for( const char * j = i->c_str(); *j; j++) {
			// If there's already an out-transition, take it,
			// otherwise construct a new one.
//			if( image( q, alphabetNormalize( P.iterSelect( i )
//					[trav.traverse( j )] ) ) == INVALIDSTATE ) {
			if( image( q, alphabetNormalize( (i->c_str())
					[trav.traverse( j-i->c_str() )] ) )
					== INVALIDSTATE ) {
				// Create a new State and adjust everything.
				auto State latest = rep.size();
				rep.setSize( latest + 1 );
				depth.setSize( latest + 1 );

				// Set the out-transitions of the new State
				// to INVALIDSTATE.
				//for( int a = 0; a < alphabetsize; a++ ) {
				//	rep.map( latest ).map( a ) = INVALIDSTATE;
				//}
            rep.map( latest ).invalidate();

				// latest will be at one level below q, depth-wise.
				depth.map( latest ) = depth[q] + 1;

//				rep.map( q ).map( alphabetNormalize(
//						P.iterSelect( i )[trav.traverse( j )] ) )
//						= latest;
				rep.map( q ).map( alphabetNormalize( ( i->c_str() )
						[trav.traverse( j - i->c_str() )] ) )
						= latest;

			}
//			SPAREPARTS_ASSERT( image( q, alphabetNormalize( P.iterSelect( i )
//					[trav.traverse( j )] ) ) != INVALIDSTATE );
			SPAREPARTS_ASSERT( image( q, alphabetNormalize( ( i->c_str() )
					[trav.traverse( j - i->c_str() )] ) )
					!= INVALIDSTATE );
//			q = image( q, alphabetNormalize( P.iterSelect( i )
//					[trav.traverse( j )] ) );
			q = image( q, alphabetNormalize( ( i->c_str() )
					[trav.traverse( j - i->c_str() )] ) );
		}
	}
	SPAREPARTS_ASSERT( c_inv() );
}

template<class T,class t_alphabet,const int alphabetsize>
INLINE Trie<T,t_alphabet, alphabetsize>::Trie( const Trie<T,t_alphabet, alphabetsize>& r ) :
		rep( r.rep ),
		depth( r.depth ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template<class T,class t_alphabet,const int alphabetsize>
INLINE State Trie<T,t_alphabet, alphabetsize>::image( const State q, const char a ) const {
	SPAREPARTS_ASSERT( c_inv() );
	SPAREPARTS_ASSERT( FIRSTSTATE <= q && q < size() );
	// Assume that a is already normalized by the caller.
	SPAREPARTS_ASSERT( 0 <= a && a < alphabetsize );
	return( rep[q][a] );
}

template<class T,class t_alphabet,const int alphabetsize>
INLINE State Trie<T,t_alphabet, alphabetsize>::BFTfirst() const {
	SPAREPARTS_ASSERT( c_inv() );
	SPAREPARTS_ASSERT( depth[FIRSTSTATE] == 0 );
	// Assume that we always start allocating States at 0.
	return( FIRSTSTATE );
}

template<class T,class t_alphabet,const int alphabetsize>
State Trie<T,t_alphabet, alphabetsize>::BFTsuccessor( const State q ) const {
	SPAREPARTS_ASSERT( c_inv() );
	// Short-cut for the last state in a BFT.
	// (Assumes that size() was the last state allocated.)
	if( q == size() - 1 ) {
		return( INVALIDSTATE );
	}
	// Search for the next state at this BFT level.
	auto State r;
	for( r = q + 1; r < size(); r++ ) {
		if( depth[r] == depth[q] ) {
			return( r );
		}
	}
	// There wasn't one at this level, check the next BFT level.
	for( r = FIRSTSTATE; r < size(); r++ ) {
		if( depth[r] == depth[q] + 1 ) {
			return( r );
		}
	}
	// There is nothing at a deeper level.
	// This shouldn't even be happening.
	SPAREPARTS_ASSERT( !"I shouldn't be here" );
	return( INVALIDSTATE );
}

template<class T,class t_alphabet,const int alphabetsize>
INLINE int Trie<T,t_alphabet, alphabetsize>::BFTdepth( const State q ) const {
	SPAREPARTS_ASSERT( c_inv() );
	return( depth[q] );
}

template<class T,class t_alphabet,const int alphabetsize>
INLINE int Trie<T,t_alphabet, alphabetsize>::size() const {
	SPAREPARTS_ASSERT( c_inv() );
	return( rep.size() );
}

template<class T,class t_alphabet,const int alphabetsize>
INLINE bool Trie<T,t_alphabet, alphabetsize>::c_inv() const {
	return( rep.c_inv() 
 		&& depth.c_inv()
   	&& !depth[FIRSTSTATE]
		&& rep.size() == depth.size() );
}

#if HAVE_OSTREAM_OVERLOAD
template<class T,class t_alphabet,const int alphabetsize>
std::ostream& operator<<( std::ostream& os, const Trie<T,t_alphabet, alphabetsize>& t ) {
	SPAREPARTS_ASSERT( t.c_inv() );
	os << "Trie<T,t_alphabet, alphabetsize> = (\n" << t.rep << t.depth << ")\n";
	SPAREPARTS_ASSERT( t.c_inv() );
	return( os );
}
#endif

