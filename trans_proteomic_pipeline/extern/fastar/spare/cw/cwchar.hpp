/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC


#ifndef CWCHAR_HPP
#define CWCHAR_HPP
#define IN_CWCHAR_HPP

#include "alphabet.hpp"
#include "com-misc.hpp"
#include "state.hpp"
#include "symbolto.hpp"
#include "tries.hpp"
#include <assert.h>
#include <iostream>

// Simple character shift function for the CW algorithm.

template <class t_alphabet,const int alphabetsize> class CharCW {
public:
	CharCW( const RTrie<t_alphabet,alphabetsize> & t );
	CharCW( const CharCW<t_alphabet,alphabetsize> & r );

	// The shift itself.
	int shift( const char a, const int z ) const;
	bool c_inv() const;
	template <class t_alphabet2,const int alphabetsize2> friend std::ostream& operator<<( std::ostream& os, const CharCW<t_alphabet2,alphabetsize2> & r );
private:
	#include "cwchar_p.hpp"
};

#include "cwchar_i.hpp"

#undef IN_CWCHAR_HPP
#endif

