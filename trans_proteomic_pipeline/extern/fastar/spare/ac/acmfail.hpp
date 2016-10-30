/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC
//

#ifndef ACMFAIL_HPP
#define ACMFAIL_HPP
#define IN_ACMFAIL_HPP

#include "state.hpp"
#include "string.hpp"
//#include "set.hpp"
#include <set>
#include "aceftrie.hpp"
#include "fails.hpp"
#include "tries.hpp"
#include "acout.hpp"
#include <assert.h>
#include <iostream>

// This is the Aho-Corasick transition and output machine
// corresponding to the failure function version of the AC algorithm.

template <class t_alphabet,const int alphabetsize> class ACMachineFail {
public:
	ACMachineFail( const kwset_t& P );
	ACMachineFail( const ACMachineFail<t_alphabet,alphabetsize>& M );
	~ACMachineFail();

	// The machine transition stuff.
	State transition( State q, const char a ) const;
	const kwset_t& output( const State q ) const;
	bool c_inv() const;
	template <class t_alphabet2,const int alphabetsize2> friend std::ostream& operator<<( std::ostream& os, const ACMachineFail<t_alphabet,alphabetsize>& t );
private:
	#include "acmfail_p.hpp"
};

#include "acmfail_i.hpp"

#undef IN_ACMFAIL_HPP
#endif

