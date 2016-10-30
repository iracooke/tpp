/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

template<class MO>
BMShift12<MO>::BMShift12( const kw_t& p, const MO& mo ) :
		s1( p, mo ),
		char2( p, mo ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template<class MO>
INLINE BMShift12<MO>::BMShift12( const BMShift12<MO>& r ) :
		s1( r.s1 ),
		char2( r.char2 ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template<class MO>
INLINE int BMShift12<MO>::shift( const int i, const char a ) const {
	SPAREPARTS_ASSERT( c_inv() );
	// Expect a to be normalized already.
	return( max( s1.shift( i ), char2.shift( i ) ) );
}

template<class MO>
INLINE bool BMShift12<MO>::c_inv() const {
	return( s1.c_inv() && char2.c_inv() );
}

#if HAVE_OSTREAM_OVERLOAD
template<class MO>
std::ostream& operator<<( std::ostream& os, const BMShift12<MO>& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "BMShift12 = (\n" << r.s1 << r.char2 << ")\n";
	return( os );
}
#endif

