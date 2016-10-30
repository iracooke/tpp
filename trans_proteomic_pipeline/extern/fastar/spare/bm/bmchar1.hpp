/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#ifndef BMCHAR1_HPP
#define BMCHAR1_HPP
#define IN_BMCHAR1_HPP

#include "alphabet.hpp"
//#include "array.hpp"
#include <vector>
#include "symbolto.hpp"
#include "string.hpp"
#include <assert.h>
#include <iostream>

// This is the char_1 shift function for the Boyer-Moore
// type algorithms. It requires more space than the char_2.
// It depends upon the match order, and is template
// parameterized by it.

template<class MO,class t_alphabet, const int alphabetsize>
class Char1 {
public:
	Char1( const kw_t& p, const MO& mo );
	Char1( const Char1& r );

	// The shift itself.
	int shift( const int i, const char a ) const;
	bool c_inv() const;
	template<class MO2>
		friend std::ostream& operator<<( std::ostream& os, const Char1<MO2,t_alphabet, alphabetsize>& r ); // forward
private:
	#include "bmchar1_p.hpp"
};

#include "bmchar1_i.hpp"

#undef IN_BMCHAR1_HPP
#endif

