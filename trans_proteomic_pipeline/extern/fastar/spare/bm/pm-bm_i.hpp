/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

template<class MO, class SL, class MI,class t_alphabet, const int alphabetsize>
INLINE PMBM<MO,SL,MI,t_alphabet, alphabetsize>::PMBM( const kw_t &kw ) :
		keyword( kw ),
		mo( keyword ),
		sl( keyword ),
		shifter( keyword, mo ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template<class MO, class SL, class MI, class t_alphabet, const int alphabetsize>
void PMBM<MO,SL,MI,t_alphabet, alphabetsize>::match( const kw_t &S, bool callBack (int) ) {
	SPAREPARTS_ASSERT( c_inv() );
	// Use an index to denote the end of the current
	// match attempt. The name is chosen to go with v, in
	// the taxonomy.
	auto int pLen = keyword.length();
	// Keep track of the last possible place to be matching.
	auto int Slast = S.length() - pLen;
	auto int vBegin = 0;

	// Try matching, while we're not at the end of the input.
	while( vBegin <= Slast ) {
		// Do some skip looping.
		vBegin = sl.skip( S, vBegin, Slast );

		// Make sure that it worked out okay.
		SPAREPARTS_ASSERT( vBegin <= Slast );

		// Now time for a match attempt.
		auto int i;
		for( i = 0; i < pLen
				&& S[vBegin + mo.traverse( i )]
					== keyword[mo.traverse( i )]; i++ ) {
			// Intentionally empty.
		}

		if( i == pLen ) {
			// There was a match; call the client back.
			if( !callBack( vBegin + pLen ) ) {
				// The client wants to quit.
				return;
			}
		}

		// Now make the shift. Normalize the mismatched character
		// here since the shifter won't do it.
		// Just check that the i indexing into S is still valid though.
		vBegin += shifter.shift( i,
			alphabetNormalize( i >= pLen ? 0 :
				S[vBegin + mo.traverse( i )] ) );
	}
}

template<class MO, class SL, class MI, class t_alphabet, const int alphabetsize>
INLINE bool PMBM<MO,SL,MI,t_alphabet, alphabetsize>::c_inv() const {
//	return( keyword.c_inv() && mo.c_inv()
	return( mo.c_inv()
		&& sl.c_inv() && shifter.c_inv() );
}

#if HAVE_OSTREAM_OVERLOAD
template<class MO, class SL, class MI, class t_alphabet, const int alphabetsize>
std::ostream& operator<<( std::ostream& os, const PMBM<MO,SL,MI,t_alphabet, alphabetsize> r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "PMBM = (\n" << r.keyword << r.mo << r.sl << r.shifter << ")\n";
	return( os );
}
#endif

