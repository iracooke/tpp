/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#include "com-opt.hpp"

#if defined(IN_CWSHOPT_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE CWShiftOpt<t_alphabet,alphabetsize>::CWShiftOpt( const CWShiftOpt& r ) :
		dopt( r.dopt ),
		d2( r.d2 ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE CWShiftOpt<t_alphabet,alphabetsize>::~CWShiftOpt() {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE int CWShiftOpt<t_alphabet,alphabetsize>::shift( const RTrie<t_alphabet,alphabetsize>& t,
		const char l,
		const State v,
		const char r ) const {
	SPAREPARTS_ASSERT( c_inv() );
	return( min( dopt.shift( v, l ), d2[v] ) );
}

template <class t_alphabet,const int alphabetsize> INLINE bool CWShiftOpt<t_alphabet,alphabetsize>::c_inv() const {
	return( !fr && dopt.c_inv() && d2.c_inv() );
}

#endif

