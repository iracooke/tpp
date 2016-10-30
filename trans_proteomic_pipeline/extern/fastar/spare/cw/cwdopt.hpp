/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC

#ifndef CWDOPT_HPP
#define CWDOPT_HPP
#define IN_CWDOPT_HPP

#include "com-misc.hpp"
#include "state.hpp"
#include "stateto.hpp"
#include "symbolto.hpp"
#include "tries.hpp"
#include "fails.hpp"
#include <assert.h>
#include <iostream>

// d_opt shift function.

template <class t_alphabet,const int alphabetsize> class DOpt {
public:
	DOpt( const RTrie<t_alphabet,alphabetsize>& t, const RFail<t_alphabet,alphabetsize>& f );
	DOpt( const DOpt<t_alphabet,alphabetsize>& r );

	// The shift itself.
	int shift( const State q, const char a ) const;
	bool c_inv() const;
	template <class t_alphabet2,const int alphabetsize2> friend std::ostream& operator<<( std::ostream& os, const DOpt<t_alphabet2,alphabetsize2>& r );
private:
	#include "cwdopt_p.hpp"
};

#include "cwdopt_i.hpp"

#undef IN_CWDOPT_HPP
#endif

