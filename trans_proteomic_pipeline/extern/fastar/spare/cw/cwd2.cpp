/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#define IN_CWD2_CPP

#include "cwd2.hpp"

// For details on the following, see [Watson-Watson94] or
// [Watson-Zwaan95].

template <class t_alphabet,const int alphabetsize> D2<t_alphabet,alphabetsize>::D2( const kwset_t& P,
		const RTrie<t_alphabet,alphabetsize>& t,
		const RFail<t_alphabet,alphabetsize>& fr,
		const CWOutput<t_alphabet,alphabetsize>& out ) :
		rep( t.size() ) {
	SPAREPARTS_ASSERT( t.c_inv() );
	SPAREPARTS_ASSERT( fr.c_inv() );
	SPAREPARTS_ASSERT( t.size() == fr.size() );
	auto State u;
	for( u = FIRSTSTATE; u < rep.size(); u++ ) {
		rep.map( u ) = PLUSINFINITY;
	}
	// Check if the empty kw_t is a keyword.
//	for( int i = 0; i < P.size(); i++ ) {
	for( kwset_t::const_iterator i = P.begin(); i != P.end(); i++ ) {
		if( !i->length() ) {
			rep.map( FIRSTSTATE ) = 1;
			break;
		}
	}
	for( u = FIRSTSTATE; u < rep.size(); u++ ) {
		if( out.isKeyword( u ) ) {
			auto State v = u;
			while( v != FIRSTSTATE ) {
				v = fr[v];
				if( t.BFTdepth( u ) - t.BFTdepth( v )
						< rep[v] ) {
					rep.map( v ) = t.BFTdepth( u )
						- t.BFTdepth( v );
				} else {
					break;
				}
			}
		}
	}
	// One last BFT for cleanup.
	for( u = t.BFTfirst(); u != INVALIDSTATE;
			u = t.BFTsuccessor( u ) ) {
		for( char a = 0; a < alphabetsize; a++ ) {
			if( t.image( u, a ) != INVALIDSTATE ) {
				// u = (au \leftdrop 1)
				rep.map( t.image( u, a ) ) = min( rep[u],
					rep[t.image( u, a )] );
			}
		}
	}
	SPAREPARTS_ASSERT( c_inv() );
}

#if HAVE_OSTREAM_OVERLOAD
template <class t_alphabet,const int alphabetsize> std::ostream& operator<<( std::ostream& os, const D2<t_alphabet,alphabetsize>& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "D2 = (\n" << r.rep << ")\n";
	SPAREPARTS_ASSERT( r.c_inv() );
	return( os );
}
#endif

// instantiate for the various supported alphabets
FASTAR_INSTANTIATE(D2);
