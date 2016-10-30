/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

#if defined(IN_ACOUT_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE ACOutput<t_alphabet,alphabetsize>::ACOutput( const ACOutput<t_alphabet,alphabetsize>& r ) :
		rep( r.rep ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE const kwset_t& ACOutput<t_alphabet,alphabetsize>::operator[]( const State q ) const {
	SPAREPARTS_ASSERT( c_inv() );
	SPAREPARTS_ASSERT( FIRSTSTATE <= q && q < rep.size() );
	return( rep[q] );
}

template <class t_alphabet,const int alphabetsize> INLINE int ACOutput<t_alphabet,alphabetsize>::size() const {
	SPAREPARTS_ASSERT( c_inv() );
	return( rep.size() );
}

template <class t_alphabet,const int alphabetsize> INLINE bool ACOutput<t_alphabet,alphabetsize>::c_inv() const {
	return( rep.c_inv() );
}

#endif

