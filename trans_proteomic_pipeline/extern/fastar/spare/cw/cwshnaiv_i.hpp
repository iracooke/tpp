/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

#if defined(IN_CWSHNAIV_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE CWShiftNaive<t_alphabet,alphabetsize>::CWShiftNaive( const kwset_t& P,
			const RTrie<t_alphabet,alphabetsize>& t,
			const CWOutput<t_alphabet,alphabetsize>& out ) {
	// Intentionally empty.
}

template <class t_alphabet,const int alphabetsize> INLINE CWShiftNaive<t_alphabet,alphabetsize>::CWShiftNaive( const CWShiftNaive<t_alphabet,alphabetsize>& r ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE int CWShiftNaive<t_alphabet,alphabetsize>::shift( const RTrie<t_alphabet,alphabetsize>& t,
		const char l,
		const State v,
		const char r ) const {
	SPAREPARTS_ASSERT( c_inv() );
	return( 1 );
}

template <class t_alphabet,const int alphabetsize> INLINE bool CWShiftNaive<t_alphabet,alphabetsize>::c_inv() const {
	return( true );
}

#endif

