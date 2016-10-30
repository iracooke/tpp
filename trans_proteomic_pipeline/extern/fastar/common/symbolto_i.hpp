// made "rep" a fixed size array instead of the STL vector it was July 2006 bpratt Insilicos LLC
// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC

/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#include "com-opt.hpp"
#include "com-io.hpp"

template<class T, class t_alphabet, const int alphabetsize>
INLINE SymbolTo<T,t_alphabet, alphabetsize>::SymbolTo() {
	SPAREPARTS_ASSERT( c_inv() );
}

template<class T, class t_alphabet, const int alphabetsize>
INLINE SymbolTo<T,t_alphabet, alphabetsize>::SymbolTo( const SymbolTo<T,t_alphabet,alphabetsize>& r ) 
		 {
   memmove(rep, r.rep, alphabetsize*sizeof(T));
	SPAREPARTS_ASSERT( c_inv() );
}

template<class T, class t_alphabet, const int alphabetsize>
INLINE T& SymbolTo<T,t_alphabet, alphabetsize>::map( const int index ) {
	SPAREPARTS_ASSERT( c_inv() );
//	return( rep.at( index ) );
	return( rep[index] );
}

template<class T, class t_alphabet, const int alphabetsize>
INLINE const T& SymbolTo<T,t_alphabet, alphabetsize>::operator[]( const int index ) const {
	SPAREPARTS_ASSERT( c_inv() );
	return( rep[index] );
}

template<class T, class t_alphabet, const int alphabetsize>
INLINE bool SymbolTo<T,t_alphabet, alphabetsize>::c_inv() const {
//	return( rep.size() == alphabetsize && rep.c_inv() );
	return( true );
}

template<class T, class t_alphabet, const int alphabetsize>
std::ostream& operator<<( std::ostream& os, const SymbolTo<T,t_alphabet, alphabetsize>& t ) {
	SPAREPARTS_ASSERT( t.c_inv() );
	os << "SymbolTo = (\n" << t.rep << ")\n";
	SPAREPARTS_ASSERT( t.c_inv() );
	return( os );
}

template<class T, class t_alphabet, const int alphabetsize>
INLINE void SymbolTo<T,t_alphabet, alphabetsize>::invalidate() {
//   for( int a = 0; a < alphabetsize; a++ ) {
//		rep.map( a ) = INVALIDSTATE;
//	}
   memset(rep,INVALIDSTATE,alphabetsize*sizeof(T));
}

