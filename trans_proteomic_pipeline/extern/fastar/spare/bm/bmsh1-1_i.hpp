/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

template<class MO,class t_alphabet, const int alphabetsize>
INLINE BMShift11<MO,t_alphabet, alphabetsize>::BMShift11( const kw_t& p, const MO& mo ) :
		s1( p, mo ),
		char1( p, mo ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template<class MO,class t_alphabet, const int alphabetsize>
INLINE BMShift11<MO,t_alphabet, alphabetsize>::BMShift11( const BMShift11<MO,t_alphabet, alphabetsize>& r ) :
		s1( r.s1 ),
		char1( r.char1 ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template<class MO,class t_alphabet, const int alphabetsize>
INLINE int BMShift11<MO,t_alphabet, alphabetsize>::shift( const int i, const char a ) const {
	SPAREPARTS_ASSERT( c_inv() );
	// Expect a to be normalized already.
	return( max( s1.shift( i ), char1.shift( i, a ) ) );
}

template<class MO,class t_alphabet, const int alphabetsize>
INLINE bool BMShift11<MO,t_alphabet, alphabetsize>::c_inv() const {
	return( s1.c_inv() && char1.c_inv() );
}

#if HAVE_OSTREAM_OVERLOAD
template<class MO,class t_alphabet, const int alphabetsize>
std::ostream& operator<<( std::ostream& os, const BMShift11<MO,t_alphabet, alphabetsize>& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "BMShift11 = (\n" << r.s1 << r.char1 << ")\n";
	return( os );
}
#endif

