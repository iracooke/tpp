/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

template<class MO>
INLINE BMShiftNaive<MO>::BMShiftNaive( const kw_t& p, const MO& mo ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template<class MO>
INLINE BMShiftNaive<MO>::BMShiftNaive( const BMShiftNaive<MO>& r ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template<class MO>
INLINE int BMShiftNaive<MO>::shift( const int i, const char a ) const {
	return( 1 );
}

template<class MO>
INLINE bool BMShiftNaive<MO>::c_inv() const {
	return( true );
}

#if HAVE_OSTREAM_OVERLOAD
template<class MO>
std::ostream& operator<<( std::ostream& os, const BMShiftNaive<MO>& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "BMShiftNaive = ()\n";
	return( os );
}
#endif

