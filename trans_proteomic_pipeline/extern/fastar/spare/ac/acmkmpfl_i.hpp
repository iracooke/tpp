/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#include "com-opt.hpp"

#if defined(IN_ACMKMPFAIL_CPP) || defined(INLINING)

template <class t_alphabet,const int alphabetsize> INLINE ACMachineKMPFail<t_alphabet,alphabetsize>::ACMachineKMPFail( const kwset_t& P ) :
		tau( P ),
		ff( tau ),
		out( P, tau, ff ) {
//	SPAREPARTS_ASSERT( P.c_inv() );
	SPAREPARTS_ASSERT( !P.empty() );
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE ACMachineKMPFail<t_alphabet,alphabetsize>::ACMachineKMPFail( const ACMachineKMPFail& M ) :
		tau( M.tau ),
		ff( M.ff ),
		out( M.out ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE ACMachineKMPFail<t_alphabet,alphabetsize>::~ACMachineKMPFail() {
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> INLINE State ACMachineKMPFail<t_alphabet,alphabetsize>::transition( State q, const char a ) const {
	SPAREPARTS_ASSERT( c_inv() );
	// The char a will not have been normalized yet, so do it.
	// Standard linear search.
	while( tau.image( q, alphabetNormalize( a ) ) == INVALIDSTATE && q != FIRSTSTATE ) {
		q = ff[q];
	}
	if( tau.image( q, alphabetNormalize( a ) ) != INVALIDSTATE ) {
		SPAREPARTS_ASSERT( c_inv() );
		return( tau.image( q, alphabetNormalize( a ) ) );
	} else {
		SPAREPARTS_ASSERT( c_inv() );
		SPAREPARTS_ASSERT( q == FIRSTSTATE );
		return( q );
	}
}

template <class t_alphabet,const int alphabetsize> INLINE const kwset_t& ACMachineKMPFail<t_alphabet,alphabetsize>::output( const State q ) const {
	SPAREPARTS_ASSERT( c_inv() );
	return( out[q] );
}

template <class t_alphabet,const int alphabetsize> INLINE bool ACMachineKMPFail<t_alphabet,alphabetsize>::c_inv() const {
	return( tau.c_inv() && ff.c_inv() && out.c_inv()
		&& tau.size() == ff.size()
		&& tau.size() == out.size() );
}

#endif

