/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

template<class T,class t_alphabet, const int alphabetsize>
INLINE PMCW<T,t_alphabet, alphabetsize>::PMCW( const kwset_t& P ) :
		trie( P ),
		out( P, trie ),
		shift( P, trie, out ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template<class T,class t_alphabet, const int alphabetsize>
INLINE PMCW<T,t_alphabet, alphabetsize>::PMCW( const PMCW<T,t_alphabet, alphabetsize>& r ) :
		trie( r.trie ),
		out( r.out ),
		shift( r.shift ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template<class T,class t_alphabet, const int alphabetsize>
INLINE void PMCW<T,t_alphabet, alphabetsize>::match( const kw_t &S,
		bool callBack (int, const kwset_t&) ) {
	SPAREPARTS_ASSERT( c_inv() );
	// The indexes for matching. No need to maintain u, it's just r-1.
	// u is encoded as follows:
	// the abstract l is represented as integer l such that
	// S[0..l] = abstract l.
	auto int r = 0, l = -1;
	auto State v = FIRSTSTATE;
	// Maintain a set of current matches.
	auto kwset_t O;
	if( out.isKeyword( v ) ) {
		O.insert( out[v] );
		// There was a match, so call the client back with it.
		if( !callBack( 0, O ) ) {
			return;
		}
		// Wipe out the match information.
		O.clear();
	}
	// Make an initial shift, thereby phase-shifting the following
	// loop structure.
	// First make a shift. Take care of the case where l has
	// run off the left end of S.
	r += shift.shift( trie, alphabetNormalize( (l < 0 ?
		0 : S[l]) ), v, alphabetNormalize( S[r] ) );

	// Continue until we've passed the right end of the input kw_t.
	auto int Slen = S.length();
	while( r <= Slen ) {
		// Match from right to left.
		l = r - 1;
		v = FIRSTSTATE;
		// Preliminary check for a match.
		if( out.isKeyword( v ) ) {
			O.insert( out[v] );
			// Don't call back the client yet.
		}
		while( l >= 0 && trie.image( v,
				alphabetNormalize( S[l] ) ) != INVALIDSTATE ) {
			v = trie.image( v, alphabetNormalize( S[l] ) );
			l--;
			// Now check if there's a match.
			if( out.isKeyword( v ) ) {
				O.insert( out[v] );
				// Don't call back the client yet.
			}
		}
		// Report any matches.
		if( !O.empty() ) {
			if( !callBack( r, O ) ) {
				// Client wants to quit.
				return;
			}
			O.clear();
		}

		// Make a shift. Take care of the case where l has
		// run off the left end of S.
		SPAREPARTS_ASSERT( shift.shift( trie, alphabetNormalize( (l < 0 ?
			0 : S[l]) ), v, alphabetNormalize( S[r] ) ) >= 1 );

		r += shift.shift( trie, alphabetNormalize( (l < 0 ?
			0 : S[l]) ), v, alphabetNormalize( S[r] ) );
	}
}

template<class T,class t_alphabet, const int alphabetsize>
INLINE bool PMCW<T,t_alphabet, alphabetsize>::c_inv() const {
	return( trie.c_inv() && out.c_inv() && shift.c_inv() );
}

#if HAVE_OSTREAM_OVERLOAD
template<class T,class t_alphabet, const int alphabetsize>
std::ostream& operator<<( std::ostream& os, const PMCW<T,t_alphabet, alphabetsize> r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "PMCW<T,t_alphabet, alphabetsize> = (\n" << r.trie << r.out << r.shift << ")\n";
	return( os );
}
#endif

