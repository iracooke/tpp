/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

#if defined(IN_PM_KMP_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE PMKMP<t_alphabet,alphabetsize>::PMKMP( const kw_t &kw ) :
		keyword( kw ),
		ff( keyword ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE PMKMP<t_alphabet,alphabetsize>::PMKMP( const PMKMP<t_alphabet,alphabetsize>& r ) :
		keyword( r.keyword ),
		ff( r.ff ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE bool PMKMP<t_alphabet,alphabetsize>::c_inv() const {
//	return( keyword.c_inv() && ff.c_inv() );
	return( ff.c_inv() );
}

#endif

