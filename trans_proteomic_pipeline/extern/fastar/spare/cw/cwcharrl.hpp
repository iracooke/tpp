/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC
//

#ifndef CWCHARRLA_HPP
#define CWCHARRLA_HPP
#define IN_CWCHARRLA_HPP

#include "alphabet.hpp"
#include "com-misc.hpp"
#include "state.hpp"
//#include "set.hpp"
#include <set>
#include "symbolto.hpp"
#include "tries.hpp"
#include <assert.h>
#include <iostream>

// Character shift function for right lookahead in CW.

template <class t_alphabet,const int alphabetsize> class CharRLA {
public:
	CharRLA( const RTrie<t_alphabet,alphabetsize>& t, const kwset_t& P );
	CharRLA( const CharRLA<t_alphabet,alphabetsize>& r );

	// The shift itself.
	int operator[]( const char a ) const;
	bool c_inv() const;
	template <class t_alphabet2,const int alphabetsize2> friend std::ostream& operator<<( std::ostream& os, const CharRLA<t_alphabet2,alphabetsize2>& r );
private:
	#include "cwcharrl_p.hpp"
};

#include "cwcharrl_i.hpp"

#undef IN_CWCHARRLA_HPP
#endif

