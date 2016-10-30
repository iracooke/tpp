/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#define IN_CWSHIFTWBM_CPP

#include "cwshwbm.hpp"

template <class t_alphabet,const int alphabetsize> CWShiftWBM<t_alphabet,alphabetsize>::CWShiftWBM( const kwset_t& P,
			const RTrie<t_alphabet,alphabetsize>& t,
			const CWOutput<t_alphabet,alphabetsize>& out ) :
		rep( t.size() ),
		charBM( t, P ) {
//	SPAREPARTS_ASSERT( P.c_inv() );
	SPAREPARTS_ASSERT( t.c_inv() );
	SPAREPARTS_ASSERT( out.c_inv() );
	// Now construct d_1 and d_2.
	auto RFail<t_alphabet,alphabetsize> f( t );
	auto D1<t_alphabet,alphabetsize> d1( t, f );
	auto D2<t_alphabet,alphabetsize> d2( P, t, f, out );

	// Now merge d1 and d2 into one.
	for( State q = FIRSTSTATE; q < rep.size(); q++ ) {
		rep.map( q ) = min( d1[q], d2[q] );
	}
	SPAREPARTS_ASSERT( c_inv() );
}

#if HAVE_OSTREAM_OVERLOAD
template <class t_alphabet,const int alphabetsize> std::ostream& operator<<( std::ostream& os, const CWShiftWBM<t_alphabet,alphabetsize>& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "CWShiftWBM = (\n" << r.rep << r.charBM << ")\n";
	SPAREPARTS_ASSERT( r.c_inv() );
	return( os );
}
#endif

// instantiate for the various supported alphabets
FASTAR_INSTANTIATE(CWShiftWBM);
