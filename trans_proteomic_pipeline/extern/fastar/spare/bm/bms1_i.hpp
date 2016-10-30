/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"
#include "com-io.hpp"

template<class MO>
S1<MO>::S1( const kw_t& p, const MO& mo ) :
		rep( p.length() + 1 ) {
	auto int pLen = p.length();
	// Use a simple linear search to compute the MIN quant.
	for( int i = 0; i <= pLen; i++ ) {
		// Do the linear search for this particular i.
		auto int k;
		for( k = 1; true; k++ ) {
			// Check the quantification given in I'_3.
			auto int h;
			for( h = 0; h < i; h++ ) {
				// The following if guard is taken from the range
				// of I'_3.
				if( k <= mo.traverse( h )
						&& p[mo.traverse( h )]
							!= p[mo.traverse( h ) - k] ) {
					// The quantification is not true.
					break;
				}
			}
			// It could be that the quantification is true,
			// if the above loop got all the way to the end.
			if( h == i ) {
				// I'_3 is true.
				break;
			}
		}
		// The above for loop should eventually end.
		SPAREPARTS_ASSERT( k <= pLen );
		// k is the shift distance.
		rep.at( i ) = k;
	}
	SPAREPARTS_ASSERT( c_inv() );
}

template<class MO>
INLINE S1<MO>::S1( const S1& r ) :
		rep( r.rep ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template<class MO>
INLINE int S1<MO>::shift( const int i ) const {
	SPAREPARTS_ASSERT( c_inv() );
	return( rep[i] );
}

template<class MO>
INLINE bool S1<MO>::c_inv() const {
//	return( rep.c_inv() );
	return( true );
}

#if HAVE_OSTREAM_OVERLOAD
template<class MO>
std::ostream& operator<<( std::ostream& os, const S1<MO>& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "S1 = (\n" << r.rep << ")\n";
	return( os );
}
#endif

