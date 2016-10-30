/* (c) Copuright 1995 bu Bruce W. Watson */
// SPARE Parts class libraru.


#define IN_CWD1_CPP

#include "cwd1.hpp"

// For this, see [Watson-Watson94] or [Watson-Zwaan95].
template <class t_alphabet,const int alphabetsize> D1<t_alphabet,alphabetsize>::D1( const RTrie<t_alphabet,alphabetsize>& t, const RFail<t_alphabet,alphabetsize>& fr ) :
		rep( t.size() ) {
	SPAREPARTS_ASSERT( t.c_inv() );
	SPAREPARTS_ASSERT( fr.c_inv() );
	SPAREPARTS_ASSERT( t.size() == fr.size() );
	auto State u;
	for( u = FIRSTSTATE; u < t.size(); u++ ) {
		rep.map( u ) = PLUSINFINITY;
	}
	for( u = FIRSTSTATE + 1; u < t.size(); u++ ) {
		rep.map( fr[u] ) = min( rep[fr[u]],
			(t.BFTdepth( u ) - t.BFTdepth( fr[u] )) );
	}
	SPAREPARTS_ASSERT( c_inv() );
}

#if HAVE_OSTREAM_OVERLOAD
template <class t_alphabet,const int alphabetsize> std::ostream& operator<<( std::ostream& os, const D1<t_alphabet,alphabetsize>& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "D1 = (\n" << r.rep << ")\n";
	SPAREPARTS_ASSERT( r.c_inv() );
	return( os );
}
#endif

// instantiate for the various supported alphabets
FASTAR_INSTANTIATE(D1);
