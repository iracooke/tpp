/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC
//

#ifndef CWCHARBM_HPP
#define CWCHARBM_HPP
#define IN_CWCHARBM_HPP

#include "alphabet.hpp"
#include "com-misc.hpp"
#include "state.hpp"
//#include "set.hpp"
#include <set>
#include "symbolto.hpp"
#include "tries.hpp"
#include <assert.h>
#include <iostream>

// Character shift function for the CW-BM algorithm.

template <class t_alphabet,const int alphabetsize> class CharBM {
public:
	CharBM( const RTrie<t_alphabet,alphabetsize>& t, const kwset_t& P );
	CharBM( const CharBM<t_alphabet,alphabetsize>& r );

	// The shift itself.
	int operator[]( const char a ) const;
	bool c_inv() const;
	template <class t_alphabet2,const int alphabetsize2> friend std::ostream& operator<<( std::ostream& os, const CharBM<t_alphabet2,alphabetsize2>& r );
private:
	#include "cwcharbm_p.hpp"
};

#include "cwcharbm_i.hpp"

#undef IN_CWCHARBM_HPP
#endif

