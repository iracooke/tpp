/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#include "com-opt.hpp"

#if defined(IN_CWSHIFTWBM_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE CWShiftWBM<t_alphabet,alphabetsize>::CWShiftWBM( const CWShiftWBM<t_alphabet,alphabetsize>& r ) :
		rep( r.rep ),
		charBM( r.charBM ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE CWShiftWBM<t_alphabet,alphabetsize>::~CWShiftWBM() {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE int CWShiftWBM<t_alphabet,alphabetsize>::shift( const RTrie<t_alphabet,alphabetsize>& t,
		const char l,
		const State v,
		const char r ) const {
	SPAREPARTS_ASSERT( c_inv() );
	return( max( charBM[l] - t.BFTdepth( v ), rep[v] ) );
}

template <class t_alphabet,const int alphabetsize> INLINE bool CWShiftWBM<t_alphabet,alphabetsize>::c_inv() const {
	return( rep.c_inv() && charBM.c_inv() );
}

#endif

