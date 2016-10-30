/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

#if defined(IN_PM_BFSIN_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE PMBFSingle<t_alphabet,alphabetsize>::PMBFSingle( const kw_t& keyword ) :
		kw( keyword ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE bool PMBFSingle<t_alphabet,alphabetsize>::c_inv() const {
	return( true );
}

#endif

