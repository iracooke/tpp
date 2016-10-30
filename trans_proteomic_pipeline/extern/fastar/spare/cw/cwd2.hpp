/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC
//

#ifndef CWD2_HPP
#define CWD2_HPP
#define IN_CWD2_HPP

#include "com-misc.hpp"
#include "state.hpp"
#include "stateto.hpp"
#include "string.hpp"
//#include "set.hpp"
#include <set>
#include "tries.hpp"
#include "fails.hpp"
#include "cwout.hpp"
#include <assert.h>
#include <iostream>

// Commentz-Walter's d_2 shift function.

template <class t_alphabet,const int alphabetsize> class D2 {
public:
	D2( const kwset_t& P,
			const RTrie<t_alphabet,alphabetsize>& t,
			const RFail<t_alphabet,alphabetsize>& f,
			const CWOutput<t_alphabet,alphabetsize>& out );
	D2( const D2& r );

	// The shift itself.
	int operator[]( const State q ) const;
	bool c_inv() const;
	template <class t_alphabet2,const int alphabetsize2> friend std::ostream& operator<<( std::ostream& os, const D2<t_alphabet2,alphabetsize2>& r );
private:
	#include "cwd2_p.hpp"
};

#include "cwd2_i.hpp"

#undef IN_CWD2_HPP
#endif

