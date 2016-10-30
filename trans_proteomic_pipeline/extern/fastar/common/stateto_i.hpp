/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#include "com-opt.hpp"
#include "com-io.hpp"

template<class T, class t_alphabet, const int alphabetsize>
INLINE StateTo<T, t_alphabet, alphabetsize>::StateTo( const int size ) :
		rep( size ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template<class T, class t_alphabet, const int alphabetsize>
INLINE StateTo<T, t_alphabet, alphabetsize>::StateTo( const StateTo<T, t_alphabet, alphabetsize>& r ) :
		rep( r.rep ) {
	SPAREPARTS_ASSERT( c_inv() );
}

template<class T, class t_alphabet, const int alphabetsize>
INLINE T& StateTo<T, t_alphabet, alphabetsize>::map( const State index ) {
	SPAREPARTS_ASSERT( c_inv() );
//	return( rep.at( index ) );
	return( rep[index] );
}

template<class T, class t_alphabet, const int alphabetsize>
INLINE const T& StateTo<T, t_alphabet, alphabetsize>::operator[]( const State index ) const {
	SPAREPARTS_ASSERT( c_inv() );
	return( rep[index] );
}

template<class T, class t_alphabet, const int alphabetsize>
INLINE void StateTo<T, t_alphabet, alphabetsize>::setSize( const int s ) {
	SPAREPARTS_ASSERT( c_inv() );
//	rep.setSize( s );
	rep.resize( s );
	SPAREPARTS_ASSERT( c_inv() );
}

template<class T, class t_alphabet, const int alphabetsize>
INLINE int StateTo<T, t_alphabet, alphabetsize>::size() const {
	SPAREPARTS_ASSERT( c_inv() );
	return( rep.size() );
}

template<class T, class t_alphabet, const int alphabetsize>
INLINE bool StateTo<T, t_alphabet, alphabetsize>::c_inv() const {
//	return( rep.c_inv() );
	return( true );
}

template<class T, class t_alphabet, const int alphabetsize>
std::ostream& operator<<( std::ostream& os, const StateTo<T, t_alphabet, alphabetsize>& t ) {
	SPAREPARTS_ASSERT( t.c_inv() );
	os << "StateTo = (\n" << t.rep << ")\n";
	SPAREPARTS_ASSERT( t.c_inv() );
	return( os );
}

