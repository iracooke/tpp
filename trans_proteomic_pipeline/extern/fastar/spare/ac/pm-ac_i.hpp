/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

template<class T,class t_alphabet, const int alphabetsize>
INLINE PMAC<T,t_alphabet, alphabetsize>::PMAC( const kwset_t& P ) :
		machine( P ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template<class T,class t_alphabet, const int alphabetsize>
INLINE PMAC<T,t_alphabet, alphabetsize>::PMAC( const PMAC<T,t_alphabet, alphabetsize>& M ) :
		machine( M.machine ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template<class T,class t_alphabet, const int alphabetsize>
void PMAC<T,t_alphabet, alphabetsize>::match( const kw_t &S,
		bool callBack (int, const kwset_t&) ) {
	SPAREPARTS_ASSERT( c_inv() );
	// Assume that the machine has the same idea of what the first
	// State is.
	auto State q = FIRSTSTATE;
	// Got to keep track of the index into the input kw_t.
	auto int j = 0;

	if( !machine.output( q ).empty() ) {
		// There's something to output.
		if( !callBack( j, machine.output( q ) ) ) {
			SPAREPARTS_ASSERT( c_inv() );
			return;
		}
	}
	auto int Slen = S.length();
	while( j < Slen ) {
//		q = machine.transition( q, alphabetNormalize( S[j] ) );
//		this was incorrect, 2 out of 3 machines do the normalization
//		so for now, removed the alphabetNormalize call here and added it to
//		the other machine
		q = machine.transition( q, S[j] );
		// The transition function must be total.
		SPAREPARTS_ASSERT( q != INVALIDSTATE );
		j++;
		if( !machine.output( q ).empty() ) {
			if( !callBack( j, machine.output( q ) ) ) {
				SPAREPARTS_ASSERT( c_inv() );
				return;
			}
		}
	}
	SPAREPARTS_ASSERT( c_inv() );
}

template<class T,class t_alphabet, const int alphabetsize>
INLINE bool PMAC<T,t_alphabet, alphabetsize>::c_inv() const {
	return( machine.c_inv() );
}

#if HAVE_OSTREAM_OVERLOAD
template<class T,class t_alphabet, const int alphabetsize>
std::ostream& operator<<( std::ostream& os, const PMAC<T,t_alphabet, alphabetsize>& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "PMAC<T,t_alphabet, alphabetsize> = (\n" << r.machine << ")\n";
	return( os );
}
#endif

