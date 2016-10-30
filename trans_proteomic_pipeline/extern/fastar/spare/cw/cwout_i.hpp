/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

#if defined(IN_CWOUT_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE int CWOutput<t_alphabet,alphabetsize>::isKeyword( const State q ) const {
	SPAREPARTS_ASSERT( c_inv() );
	return( rep[q] != 0 );
}

template <class t_alphabet,const int alphabetsize> INLINE const kw_t& CWOutput<t_alphabet,alphabetsize>::operator[]( const State q ) const {
	SPAREPARTS_ASSERT( c_inv() );
	SPAREPARTS_ASSERT( isKeyword( q ) );
	return( *rep[q] );
}

#endif

