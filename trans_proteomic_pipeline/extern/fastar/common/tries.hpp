/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#ifndef TRIES_HPP
#define TRIES_HPP
#define IN_TRIES_HPP

#include "trie.hpp"
#include "stravfwd.hpp"
#include "stravrev.hpp"

// Define a couple of different types of tries.

// Forward tries:
template <class t_alphabet,const int alphabetsize> class FTrie : public Trie<STravFWD,t_alphabet,alphabetsize> {
public:
   FTrie( const kwset_t& P ) : Trie<STravFWD,t_alphabet, alphabetsize>(P) {};
};
// Reverse tries:
template <class t_alphabet,const int alphabetsize> class RTrie : public Trie<STravREV,t_alphabet,alphabetsize> {
public:
   RTrie( const kwset_t& P ) : Trie<STravREV,t_alphabet, alphabetsize>(P) {};
};

#undef IN_TRIES_HPP
#endif

