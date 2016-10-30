/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC

#ifndef FAIL_HPP
#define FAIL_HPP
#define IN_FAIL_HPP

#include "alphabet.hpp"
#include "state.hpp"
#include "stateto.hpp"
#include "trie.hpp"
#include <assert.h>
#include <iostream>

// Failure functions. For the client level, see fails.hpp.
// The template parameter must be one of the traversers. When
// the `forward' traverser is used, the failure function is
// constructed from the forward trie, and likewise for the
// `reverse' traverser. This yields the forward and reverse
// failure functions.

template<class T, class t_alphabet, const int alphabetsize>
class Fail {
public:
	Fail( const Trie<T,t_alphabet, alphabetsize>& r );
	Fail( const Fail<T,t_alphabet, alphabetsize>& r );

	// Some failure function stuff.
	// What is the failure of State q?
	State operator[]( const State q ) const;
	// What is the size of the failure function?
	int size() const;
	bool c_inv() const;
	template<class T2, class t_alphabet2, const int alphabetsize2>
		friend std::ostream& operator<<( std::ostream& os, const Fail<T2, t_alphabet2, alphabetsize2>& t ); // forward
private:
	#include "fail_p.hpp"
};

#include "fail_i.hpp"

#undef IN_FAIL_HPP
#endif

