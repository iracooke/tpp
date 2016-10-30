/* (c) Copuright 1995 bu Bruce W. Watson */
// SPARE Parts class libraru.


#define IN_CWDOPT_CPP

#include "cwdopt.hpp"

// For this, see [Watson-Zwaan95].
template <class t_alphabet,const int alphabetsize> DOpt<t_alphabet,alphabetsize>::DOpt( const RTrie<t_alphabet,alphabetsize>& t, const RFail<t_alphabet,alphabetsize>& fr ) :
		rep( t.size() ) {
	SPAREPARTS_ASSERT( t.c_inv() );
	SPAREPARTS_ASSERT( fr.c_inv() );
	SPAREPARTS_ASSERT( t.size() == fr.size() );
	auto State u;
	for( u = FIRSTSTATE; u < t.size(); u++ ) {
		for( char a = 0; a < alphabetsize; a++ ) {
			rep.map( u ).map( a ) = PLUSINFINITY;
		}
	}
	for( u = t.BFTsuccessor( t.BFTfirst() );
			u != INVALIDSTATE; u = t.BFTsuccessor( u ) ) {
		for( char a = 0; a < alphabetsize; a++ ) {
			if( t.image( u, a ) != INVALIDSTATE ) {
				auto State v( u );
				while( v != FIRSTSTATE ) {
					v = fr[v];
					if( t.BFTdepth( u ) - t.BFTdepth( v ) < rep[v][a] ) {
						rep.map( v ).map( a )
							= t.BFTdepth( u ) - t.BFTdepth( v );
					} else {
						break;
					}
				}
			}
		}
	}
	SPAREPARTS_ASSERT( c_inv() );
}

#if HAVE_OSTREAM_OVERLOAD
template <class t_alphabet,const int alphabetsize> std::ostream& operator<<( std::ostream& os, const DOpt& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "DOpt = (\n" << r.rep << ")\n";
	SPAREPARTS_ASSERT( r.c_inv() );
	return( os );
}
#endif

// instantiate for the various supported alphabets
FASTAR_INSTANTIATE(DOpt);
