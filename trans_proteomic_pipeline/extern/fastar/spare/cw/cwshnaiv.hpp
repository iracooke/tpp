/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC
//

#ifndef CWSHNAIV_HPP
#define CWSHNAIV_HPP
#define IN_CWSHNAIV_HPP

#include "state.hpp"
#include "string.hpp"
//#include "set.hpp"
#include <set>
#include "tries.hpp"
#include "cwout.hpp"
#include <assert.h>
#include <iostream>

// A naive Commentz-Walter shifter.

template <class t_alphabet,const int alphabetsize> class CWShiftNaive {
public:
	CWShiftNaive( const kwset_t& P,
			const RTrie<t_alphabet,alphabetsize>& t,
			const CWOutput<t_alphabet,alphabetsize>& out );
	CWShiftNaive( const CWShiftNaive<t_alphabet,alphabetsize>& r );

	// The shifter itself.
	int shift( const RTrie<t_alphabet,alphabetsize>& t,
			const char l,
			const State v,
			const char r ) const;
	bool c_inv() const;
	template <class t_alphabet2,const int alphabetsize2> friend std::ostream& operator<<( std::ostream& os, const CWShiftNaive<t_alphabet2,alphabetsize2>& r );
};

#include "cwshnaiv_i.hpp"

#undef IN_CWSHNAIV_HPP
#endif

