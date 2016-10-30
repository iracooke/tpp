/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC

#ifndef CWD1_HPP
#define CWD1_HPP
#define IN_CWD1_HPP

#include "com-misc.hpp"
#include "state.hpp"
#include "stateto.hpp"
#include "tries.hpp"
#include "fails.hpp"
#include <assert.h>
#include <iostream>

// Commentz-Walter's d_1 shift function.

template <class t_alphabet,const int alphabetsize> class D1 {
public:
	D1( const RTrie<t_alphabet,alphabetsize>& t, const RFail<t_alphabet,alphabetsize>& f );
	D1( const D1<t_alphabet,alphabetsize>& r );

	// The shift itself.
	int operator[]( const State q ) const;
	bool c_inv() const;
	template <class t_alphabet2,const int alphabetsize2> friend std::ostream& operator<<( std::ostream& os, const D1<t_alphabet2,alphabetsize2>& r );
private:
	#include "cwd1_p.hpp"
};

#include "cwd1_i.hpp"

#undef IN_CWC1_HPP
#endif

