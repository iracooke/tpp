/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.
#include "com-opt.hpp"

#if defined(IN_CWSHRLA_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE CWShiftRLA<t_alphabet,alphabetsize>::CWShiftRLA( const CWShiftRLA<t_alphabet,alphabetsize>& r ) :
		dopt( r.dopt ),
		d2( r.d2 ),
		rla( r.rla ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE CWShiftRLA<t_alphabet,alphabetsize>::~CWShiftRLA() {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE int CWShiftRLA<t_alphabet,alphabetsize>::shift( const RTrie<t_alphabet,alphabetsize>& t,
		const char l,
		const State v,
		const char r ) const {
	SPAREPARTS_ASSERT( c_inv() );
	return( max( min( dopt.shift( v, l ), d2[v] ), rla[r] ) );
}

template <class t_alphabet,const int alphabetsize> INLINE bool CWShiftRLA<t_alphabet,alphabetsize>::c_inv() const {
	return( !fr && dopt.c_inv() && d2.c_inv() && rla.c_inv() );
}

#endif

