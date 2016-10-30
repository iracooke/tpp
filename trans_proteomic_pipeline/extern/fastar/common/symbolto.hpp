/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC

#ifndef SYMBOLTO_HPP
#define SYMBOLTO_HPP
#define IN_SYMBOLTO_HPP

#include "alphabet.hpp"
//#include "array.hpp"
#include <vector>
#include <assert.h>
#include <iostream>
#include "state.hpp"

// Provide mappings from a symbol (in the alphabet) to a T.

template<class T, class t_alphabet, const int alphabetsize>
class SymbolTo {
public:
	// The capacity must be [0..alphabetsize).
	SymbolTo();
	SymbolTo( const SymbolTo<T, t_alphabet, alphabetsize>& r );

	// Some mapping related members.
	// A changeable map.
	T& map( const int index );
	// A constant mapping.
	const T& operator[]( const int index ) const;
	bool c_inv() const;
   void invalidate(); // set all rep[] members to INVALIDSTATE
	template<class T2, class t_alphabet2, const int alphabetsize2>
		friend std::ostream& operator<<( std::ostream& os, const SymbolTo<T2, t_alphabet2, alphabetsize>& t2 ); // forward
private:
	#include "symbolto_p.hpp"
};

#include "symbolto_i.hpp"

#undef IN_SYMBOLTO_HPP
#endif

