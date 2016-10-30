/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#define IN_PM_BFSIN_CPP

#include "pm-bfsin.hpp"

template <class t_alphabet,const int alphabetsize> void PMBFSingle<t_alphabet,alphabetsize>::match( const kw_t &S, bool callBack (int) ) {
	SPAREPARTS_ASSERT( c_inv() );
	auto int pLen = kw.length();
	auto int Slast = S.length() - pLen;
	// Do simple Brute-Force matching.
	for( int i = 0; i <= Slast; i++ ) {
		auto int j;
		for( j = 0; j < pLen && kw[j] == S[i + j]; j++ ) {
			// Intentionally empty.
		}
		if( j == pLen ) {
			// There was a match.
			if( !callBack( i + pLen ) ) {
				// The client wants to quit.
				return;
			}
		}
	}
	SPAREPARTS_ASSERT( c_inv() );
}

#if HAVE_OSTREAM_OVERLOAD
template <class t_alphabet,const int alphabetsize> std::ostream& operator<<( std::ostream& os, const PMBFSingle<t_alphabet,alphabetsize>& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "PMBFSingle = (\n" << r.kw << ")\n";
	return( os );
}
#endif
