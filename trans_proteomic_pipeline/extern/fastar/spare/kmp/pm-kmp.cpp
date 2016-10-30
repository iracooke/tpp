/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#define IN_PM_KMP_CPP

#include "pm-kmp.hpp"

template <class t_alphabet,const int alphabetsize> void PMKMP<t_alphabet,alphabetsize>::match( const kw_t &S, bool callBack (int) ) {
	SPAREPARTS_ASSERT( c_inv() );
	// This is the classical KMP algorithm.
	auto int Slen = S.length();
	auto int pLen = keyword.length();

	auto int i = 0, j = 0;
	if( i == pLen ) {
		if( !callBack( j ) ) {
			// The client wants to quit the matching.
			return;
		}
	}
	while( j < Slen ) {
		while( 0 <= i && S[j] != keyword[i] ) {
			i = ff[i];
		}
		i++;
		j++;
		if( i == pLen ) {
			if( !callBack( j ) ) {
				// The client wants to quit the matching.
				return;
			}
		}
	}
	SPAREPARTS_ASSERT( c_inv() );
}

#if HAVE_OSTREAM_OVERLOAD
template <class t_alphabet,const int alphabetsize> std::ostream& operator<<( std::ostream& os, const PMKMP<t_alphabet,alphabetsize>& t ) {
	SPAREPARTS_ASSERT( t.c_inv() );
	os << "PMKMP = (\n" << "keyword = " << t.keyword << '\n'
		<< t.ff << ")\n";
	SPAREPARTS_ASSERT( t.c_inv() );
	return( os );
}
#endif

// instantiate for the various supported alphabets
FASTAR_INSTANTIATE(PMKMP);

