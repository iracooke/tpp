/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC
//

#ifndef TRIE_HPP
#define TRIE_HPP
#define IN_TRIE_HPP

#include "alphabet.hpp"
#include "string.hpp"
//#include "set.hpp"
#include "com-misc.hpp"
#include "state.hpp"
#include "stateto.hpp"
#include "symbolto.hpp"
#include <assert.h>
#include <iostream>

// The raw tries. For the client level, see tries.hpp.
// This class already does alphabet normalizing of the keyword
// set in the ctor. It does not do normalizing in the Trie::image
// member function.
// The template class parameter must be one of the kw_t
// traversal classes. Instantiating with STravFWD yields a
// forward trie, while STravREV gives a reverse trie.

template<class T,class t_alphabet, const int alphabetsize>
class Trie {
public:
	// Construct the trie from a set of keywords, using
	// the kw_t traverser of class T.
	Trie( const kwset_t& P );
	Trie( const Trie<T,t_alphabet, alphabetsize>& r );

	// Some trie related members.
	// Map a State and a char to the destination State.
	State image( const State q, const char a ) const;
	// What is the first State in a breadth-first traversal
	// of the trie?
	State BFTfirst() const;
	// What is the successor in a BFT of the trie?
	// (INVALIDSTATE if there isn't one.)
	State BFTsuccessor( const State q ) const;
	// At what BFT depth is q?
	int BFTdepth( const State q ) const;
	// What is the size of the trie?
	int size() const;
	bool c_inv() const;
	template<class T2, class t_alphabet2, const int alphabetsize2>
		friend std::ostream& operator<<( std::ostream& os, const Trie<T2,t_alphabet2, alphabetsize>& t2 ); // forward
private:
	#include "trie_p.hpp"
};

#include "trie_i.hpp"

#undef IN_TRIE_HPP
#endif

