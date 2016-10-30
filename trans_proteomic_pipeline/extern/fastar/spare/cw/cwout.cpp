/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#define IN_CWOUT_CPP

#include "cwout.hpp"

template <class t_alphabet,const int alphabetsize> CWOutput<t_alphabet,alphabetsize>::CWOutput( const kwset_t& P, const RTrie<t_alphabet,alphabetsize>& t ) :
		rep( t.size() ) {
//	SPAREPARTS_ASSERT( P.c_inv() );
	SPAREPARTS_ASSERT( t.c_inv() );
	// First make them all null
	for( State q = FIRSTSTATE; q < rep.size(); q++ ) {
		rep.map( q ) = 0;
	}
	// Go through all of the keywords.
//	for( int i = 0; i < P.size(); i++ ) {
	for( kwset_t::const_iterator i = P.begin(); i != P.end(); i++ ) {
		// Go through keyword P_i, using a STravREV corresponding
		// to P_i.
//		auto STravREV trav( P.iterSelect( i ) );
		auto STravREV trav( *i );
		auto State q = FIRSTSTATE;
//		for( int j = 0; P.iterSelect( i )[j]; j++ ) {
		for( const char * j = i->c_str(); *j; j++) {
//			q = t.image( q, P.iterSelect( i )[trav.traverse( j )] );
			q = t.image( q, alphabetNormalize( ( *i )[trav.traverse( j-i->c_str()) ] ) );
		}
		// q is the state corresponding to P_i.
//		// rep[q] != 0, unless P is messed up:
//		should probably be:
		// rep[q] == 0, unless P is messed up:
		SPAREPARTS_ASSERT( rep[q] == 0 );
//		rep.map( q ) = new kw_t( P.iterSelect( i ) );
		rep.map( q ) = new kw_t( i->c_str() );
	}
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> CWOutput<t_alphabet,alphabetsize>::CWOutput( const CWOutput<t_alphabet,alphabetsize>& r ) :
		rep( r.rep.size() ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	// Got to make copies of all of the kw_ts pointed to.
	for( State q = FIRSTSTATE; q < rep.size(); q++ ) {
		if( r.rep[q] != 0 ) {
			rep.map( q ) = new kw_t( *r.rep[q] );
		}
	}
	SPAREPARTS_ASSERT( c_inv() );
}

template <class t_alphabet,const int alphabetsize> CWOutput<t_alphabet,alphabetsize>::~CWOutput() {
	SPAREPARTS_ASSERT( c_inv() );
	// Delete each of the kw_ts pointed to.
	for( State q = FIRSTSTATE; q < rep.size(); q++ ) {
		delete rep[q];
	}
}

template <class t_alphabet,const int alphabetsize> bool CWOutput<t_alphabet,alphabetsize>::c_inv() const {
	// There must be at least one that is non empty.
	for( State q = FIRSTSTATE; q < rep.size(); q++ ) {
		if( rep[q] != 0 ) {
			return( rep.c_inv() );
		}
	}
	// This is bad.
	SPAREPARTS_ASSERT( !"This can't be happening" );
	return( false );
}

#if HAVE_OSTREAM_OVERLOAD
template <class t_alphabet,const int alphabetsize> std::ostream& operator<<( std::ostream& os, const CWOutput<t_alphabet,alphabetsize>& r ) {
	SPAREPARTS_ASSERT( r.c_inv() );
	os << "CWOutput = (\n";
	// Iterate over all of the states, looking for ones with output.
	for( State q = FIRSTSTATE; q < r.rep.size(); q++ ) {
		if( r.rep[q] != 0 ) {
			os << q << "-> \"" << *r.rep[q] << "\"\n";
		}
	}
	os << ")\n";
	SPAREPARTS_ASSERT( r.c_inv() );
	return( os );
}
#endif

// instantiate for the various supported alphabets
FASTAR_INSTANTIATE(CWOutput);

