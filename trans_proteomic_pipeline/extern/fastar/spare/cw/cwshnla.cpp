/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#define IN_CWSHIFTNLA_CPP

#include "cwshnla.hpp"

template <class t_alphabet,const int alphabetsize> CWShiftNLA<t_alphabet,alphabetsize>::CWShiftNLA( const kwset_t& P,
			const RTrie<t_alphabet,alphabetsize>& t,
			const CWOutput<t_alphabet,alphabetsize>& out ) :
		rep( t.size() ) {
//	SPAREPARTS_ASSERT( P.c_inv() );
	SPAREPARTS_ASSERT( t.c_inv() );
	SPAREPARTS_ASSERT( out.c_inv() );
	// First construct the d_1 and d_2;
	auto RFail<t_alphabet,alphabetsize> f( t );
	auto D1<t_alphabet,alphabetsize> d1( t, f );
	auto D2<t_alphabet,alphabetsize> d2( P, t, f, out );

	// Now merge them into one.
	for( State q = FIRSTSTATE; q < rep.size(); q++ ) {
		rep.map( q ) = min( d1[q], d2[q] );
	}
	SPAREPARTS_ASSERT( c_inv() );
}

#if HAVE_OSTREAM_OVERLOAD
template <class t_alphabet,const int alphabetsize> std::ostream& operator<<( std::ostream& os, const CWShiftNLA<t_alphabet,alphabetsize>& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "CWShiftNLA = (\n" << r.rep << ")\n";
	SPAREPARTS_ASSERT( r.c_inv() );
	return( os );
}
#endif

// instantiate for the various supported alphabets
FASTAR_INSTANTIATE(CWShiftNLA);
