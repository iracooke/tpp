/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC
//

#ifndef ACMOPT_HPP
#define ACMOPT_HPP
#define IN_ACMOPT_HPP

#include "state.hpp"
#include "string.hpp"
//#include "set.hpp"
#include <set>
#include "acgamma.hpp"
#include "acout.hpp"
#include <assert.h>
#include <iostream>

// This is the Aho-Corasick transition and output machine corresponding
// to the AC optimized algorithm.

template <class t_alphabet,const int alphabetsize> class ACMachineOpt {
public:
	ACMachineOpt( const kwset_t& P );
	ACMachineOpt( const ACMachineOpt& M );
	~ACMachineOpt();

	// The machine transition stuff.
	State transition( const State q, const char a ) const;
	const kwset_t& output( const State q ) const;
	bool c_inv() const;
	template <class t_alphabet2,const int alphabetsize2> friend std::ostream& operator<<( std::ostream& os, const ACMachineOpt& t );
private:
	#include "acmopt_p.hpp"
};

#include "acmopt_i.hpp"

#undef IN_ACMOPT_HPP
#endif

