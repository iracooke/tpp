/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC

#ifndef ACEFTRIE_HPP
#define ACEFTRIE_HPP
#define IN_ACEFTRIE_HPP

#include "state.hpp"
#include "stateto.hpp"
#include "symbolto.hpp"
#include "tries.hpp"
#include <assert.h>
#include <iostream>

// The extended forward trie, for use in the failure
// function Aho-Corasick algorithm.

template <class t_alphabet, const int alphabetsize> class EFTrie {
public:
	EFTrie( const FTrie<t_alphabet,alphabetsize>& t );
	EFTrie( const EFTrie<t_alphabet,alphabetsize>& r );
	// The trie stuff.
	State image( const State q, const char a ) const;
	// What is the size of the domain.
	int size() const;
	bool c_inv() const;
	template <class t_alphabet2, const int alphabetsize2> friend std::ostream& operator<<( std::ostream& os, const EFTrie<t_alphabet,alphabetsize>& t );
private:
	#include "aceftrie_p.hpp"
};

#include "aceftrie_i.hpp"

#undef IN_ACEFTRIE_HPP
#endif

