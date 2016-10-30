/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC
//

#ifndef BMSH1_1_HPP
#define BMSH1_1_HPP
#define IN_BMSH1_1_HPP

#include "string.hpp"
#include "bms1.hpp"
#include "bmchar1.hpp"
#include <assert.h>
#include <iostream>

// This is a Boyer-Moore shifter. It makes use of the s_1 and the
// char_1 shift functions (and hence its name).
// It is template parameterized by the match order because all of
// the other shifters are too.

template<class MO,class t_alphabet, const int alphabetsize>
class BMShift11 {
public:
	BMShift11( const kw_t& p, const MO& mo );
	BMShift11( const BMShift11<MO,t_alphabet, alphabetsize>& r );

	// The shift itself.
	int shift( const int i, const char a ) const;
	bool c_inv() const;
	template<class MO2,class t_alphabet2, const int alphabetsize2>
		friend std::ostream& operator<<( std::ostream& os, const BMShift11<MO2,t_alphabet2, alphabetsize2>& r ); // forward
private:
	#include "bmsh1-1_p.hpp"
};

#include "bmsh1-1_i.hpp"

#undef IN_BMSH1_1_HPP
#endif

