/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC
//

#ifndef ACOUT_HPP
#define ACOUT_HPP
#define IN_ACOUT_HPP

#include "state.hpp"
#include "stateto.hpp"
#include "string.hpp"
//#include "set.hpp"
#include <set>
#include <iterator>
#include "tries.hpp"
#include "fails.hpp"
#include <assert.h>
#include <iostream>

// The Aho-Corasick output function.

template <class t_alphabet,const int alphabetsize> class ACOutput {
public:
	ACOutput( const kwset_t& P, const FTrie<t_alphabet,alphabetsize>& t, const FFail<t_alphabet,alphabetsize>& f );
	ACOutput( const ACOutput<t_alphabet,alphabetsize>& r );

	// The output function itself.
	const kwset_t& operator[]( const State q ) const;
	// What is the size of this function?
	int size() const;
	bool c_inv() const;
	template <class t_alphabet2,const int alphabetsize2> friend std::ostream& operator<<( std::ostream& os, const ACOutput<t_alphabet,alphabetsize>& t );
private:
	#include "acout_p.hpp"
};

#include "acout_i.hpp"

#undef IN_ACOUT_HPP
#endif

