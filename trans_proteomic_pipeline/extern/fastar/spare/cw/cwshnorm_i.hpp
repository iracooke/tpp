/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#include "com-opt.hpp"

#if defined(IN_CWSHIFTNORM_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE CWShiftNorm<t_alphabet,alphabetsize>::CWShiftNorm( const CWShiftNorm& r ) :
		d1( r.d1 ),
		d2( r.d2 ),
		charCW( r.charCW ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE CWShiftNorm<t_alphabet,alphabetsize>::~CWShiftNorm() {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE int CWShiftNorm<t_alphabet,alphabetsize>::shift( const RTrie<t_alphabet,alphabetsize>& t,
		const char l,
		const State v,
		const char r ) const {
	SPAREPARTS_ASSERT( c_inv() );
	return( min( max( charCW.shift( l, t.BFTdepth( v ) ),
		d1[v] ), d2[v] ) );
}

template <class t_alphabet,const int alphabetsize> INLINE bool CWShiftNorm<t_alphabet,alphabetsize>::c_inv() const {
	return( !fr && d1.c_inv() && d2.c_inv() && charCW.c_inv() );
}

#endif

