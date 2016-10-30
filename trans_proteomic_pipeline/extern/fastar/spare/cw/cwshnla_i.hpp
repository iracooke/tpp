/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#include "com-opt.hpp"

#if defined(IN_CWSHIFTNLA_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE CWShiftNLA<t_alphabet,alphabetsize>::CWShiftNLA( const CWShiftNLA<t_alphabet,alphabetsize>& r ) :
		rep( r.rep ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE int CWShiftNLA<t_alphabet,alphabetsize>::shift( const RTrie<t_alphabet,alphabetsize>& t,
		const char l,
		const State v,
		const char r ) const {
	SPAREPARTS_ASSERT( c_inv() );
	return( rep[v] );
}

template <class t_alphabet,const int alphabetsize> INLINE bool CWShiftNLA<t_alphabet,alphabetsize>::c_inv() const {
	return( rep.c_inv() );
}

#endif

