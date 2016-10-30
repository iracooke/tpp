/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"
#include "com-io.hpp"

template<class MO>
Char2<MO>::Char2( const kw_t& p, const MO& mo ) :
		rep( p.length() + 1 ) {
	auto int pLen = p.length();
	// Use a simple linear search to compute the MIN quant.
	for( int i = 0; i <= pLen; i++ ) {
		// Do the linear search for this particular i.
		auto int k;
		for( k = 1; i < pLen && k <= mo.traverse( i )
				&& p[mo.traverse( i )] == p[mo.traverse( i ) - k];
				k++ ) {
			// Intentionally empty.
		}
		// k is the shift distance.
		rep.at( i ) = k;
	}
	SPAREPARTS_ASSERT( c_inv() );
}

template<class MO>
INLINE Char2<MO>::Char2( const Char2& r ) :
		rep( r.rep ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template<class MO>
INLINE int Char2<MO>::shift( const int i ) const {
	SPAREPARTS_ASSERT( c_inv() );
	return( rep[i] );
}

template<class MO>
INLINE bool Char2<MO>::c_inv() const {
//	return( rep.c_inv() );
	return( true );
}

#if HAVE_OSTREAM_OVERLOAD
template<class MO>
std::ostream& operator<<( std::ostream& os, const Char2<MO>& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "Char2 = (\n" << r.rep << ")\n";
	return( os );
}
#endif

