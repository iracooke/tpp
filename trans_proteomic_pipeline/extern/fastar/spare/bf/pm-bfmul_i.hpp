/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

#if defined(IN_PM_BFMUL_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE PMBFMulti<t_alphabet,alphabetsize>::PMBFMulti( const kwset_t& keywords ) :
		P( keywords ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE bool PMBFMulti<t_alphabet,alphabetsize>::c_inv() const {
//	return( P.c_inv() );
	return( true );
}

#endif

