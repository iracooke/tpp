/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"
#include "com-io.hpp"

template<class MO,class t_alphabet, const int alphabetsize>
Char1<MO,t_alphabet, alphabetsize>::Char1( const kw_t& p, const MO& mo ) :
		rep( p.length() + 1 ) {
	auto int pLen = p.length();
	// Use a simple linear search to compute the MIN quant.
	for( int i = 0; i <= pLen; i++ ) {
		for( char a = 0; a < alphabetsize; a++ ) {
			// Do the linear search for this particular i.
			auto int k;
			for( k = 1; i < pLen && k <= mo.traverse( i )
					&& a != alphabetNormalize( p[mo.traverse( i ) - k] );
					k++ ) {
				// Intentionally empty.
			}
			// k is the shift distance.
			rep.at( i ).map( a ) = k;
		}
	}
	SPAREPARTS_ASSERT( c_inv() );
}

template<class MO,class t_alphabet, const int alphabetsize>
INLINE Char1<MO,t_alphabet, alphabetsize>::Char1( const Char1& r ) :
		rep( r.rep ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template<class MO,class t_alphabet, const int alphabetsize>
INLINE int Char1<MO,t_alphabet, alphabetsize>::shift( const int i, const char a ) const {
	SPAREPARTS_ASSERT( c_inv() );
	// expect a to be normalized.
	return( rep[i][a] );
}

template<class MO,class t_alphabet, const int alphabetsize>
INLINE bool Char1<MO,t_alphabet, alphabetsize>::c_inv() const {
//	return( rep.c_inv() );
	return( true );
}

#if HAVE_OSTREAM_OVERLOAD
template<class MO,class t_alphabet, const int alphabetsize>
std::ostream& operator<<( std::ostream& os, const Char1<MO,t_alphabet, alphabetsize>& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "Char1 = (\n" << r.rep << ")\n";
	return( os );
}
#endif

