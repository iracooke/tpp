/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#define IN_PM_BFMUL_CPP

#include "pm-bfmul.hpp"

template <class t_alphabet,const int alphabetsize> void PMBFMulti<t_alphabet,alphabetsize>::match( const kw_t &S,
					  bool callBack (int, const kwset_t&) ) {
	SPAREPARTS_ASSERT( c_inv() );
	auto int Slen = S.length();

	// Store the keywords matched.
	auto kwset_t Matched;
	SPAREPARTS_ASSERT( Matched.empty() );

	// Do simple Brute-Force matching.
	for( int i = 0; i < Slen; i++ ) {
//		for( int k = 0; k < P.size(); k++ ) {
		for( kwset_t::const_iterator k = P.begin(); k != P.end(); k++ ) {
			// Do keyword k.
//			auto int pLen = P.iterSelect( k ).length();
			auto int pLen = k->length();
			auto int j;
			for( j = 0;
					j < pLen
					&& i >= pLen
//					&& P.iterSelect( k )[P.iterSelect( k ).length() - j - 1]
					&& (*k)[pLen - j - 1]
						== S[i - j];
					j++ ) {
				// Intentionally empty.
			}
			if( j == pLen ) {
				// There was a match.
				// Add it to the match set.
//				Matched.insert( P.iterSelect( k ) );
				Matched.insert( k->c_str() );
			}
		}
		// If there are any matches, then report them to the client.
		if( !Matched.empty() ) {
			if( !callBack( i, Matched ) ) {
				return;
			}
			Matched.clear();
		}
	}
	SPAREPARTS_ASSERT( c_inv() );
}

#if HAVE_OSTREAM_OVERLOAD
template <class t_alphabet,const int alphabetsize> std::ostream& operator<<( std::ostream& os, const PMBFMulti<t_alphabet,alphabetsize>& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "PMBFMulti = (\n" << r.P << ")\n";
	return( os );
}
#endif
