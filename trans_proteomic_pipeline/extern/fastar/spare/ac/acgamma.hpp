/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC


#ifndef ACGAMMA_HPP
#define ACGAMMA_HPP
#define IN_ACGAMMA_HPP

#include "state.hpp"
#include "stateto.hpp"
#include "symbolto.hpp"
#include "tries.hpp"
#include "fails.hpp"
#include <assert.h>
#include <iostream>

// The optimized Aho-Corasick transition function.

template <class t_alphabet,const int alphabetsize> class Gamma {
public:
	Gamma( const FTrie<t_alphabet,alphabetsize>& t, const FFail<t_alphabet,alphabetsize>& f );
	Gamma( const Gamma<t_alphabet,alphabetsize>& r );

	// The transition function itself.
	State image( const State q, const char a ) const;
	// What is the size of this function.
	int size() const;
	bool c_inv() const;
	template <class t_alphabet2,const int alphabetsize2> friend std::ostream& operator<<( std::ostream& os, const Gamma<t_alphabet2,alphabetsize2>& t );
private:
	#include "acgamma_p.hpp"
};

#include "acgamma_i.hpp"

#undef IN_ACGAMMA_HPP
#endif

