/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#define IN_CWSHOPT_CPP

#include "cwshopt.hpp"

template <class t_alphabet,const int alphabetsize> CWShiftOpt<t_alphabet,alphabetsize>::CWShiftOpt( const kwset_t& P,
			const RTrie<t_alphabet,alphabetsize>& t,
			const CWOutput<t_alphabet,alphabetsize>& out ) :
		fr( new RFail<t_alphabet,alphabetsize>( t ) ),
		dopt( t, *fr ),
		d2( P, t, *fr, out ) {
//	SPAREPARTS_ASSERT( P.c_inv() );
	SPAREPARTS_ASSERT( t.c_inv() );
	SPAREPARTS_ASSERT( out.c_inv() );
	// Now destroy the intermediate structure.
	delete fr;
	fr = 0;
	SPAREPARTS_ASSERT( c_inv() );
}

#if HAVE_OSTREAM_OVERLOAD
template <class t_alphabet,const int alphabetsize> std::ostream& operator<<( std::ostream& os, const CWShiftOpt<t_alphabet,alphabetsize>& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "CWShiftOpt = (\n" << r.dopt << r.d2 << ")\n";
	SPAREPARTS_ASSERT( r.c_inv() );
	return( os );
}
#endif

// instantiate for the various supported alphabets
FASTAR_INSTANTIATE(CWShiftOpt);

