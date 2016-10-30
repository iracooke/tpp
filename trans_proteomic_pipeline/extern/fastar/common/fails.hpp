/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.
#ifndef FAILS_HPP
#define FAILS_HPP
#define IN_FAILS_HPP

#include "fail.hpp"
#include "stravfwd.hpp"
#include "stravrev.hpp"

// Define a couple of different types of failure function.

// Forward failure function:
template <class t_alphabet,const int alphabetsize> class FFail : public Fail<STravFWD,t_alphabet, alphabetsize> {
public:
   FFail<t_alphabet,alphabetsize>( const FTrie<t_alphabet,alphabetsize>& r ):Fail<STravFWD,t_alphabet, alphabetsize>( r ) {
   }
};
// Reverse failure function:
template <class t_alphabet,const int alphabetsize> class RFail : public Fail<STravREV,t_alphabet, alphabetsize> {
public:
   RFail<t_alphabet,alphabetsize>( const RTrie<t_alphabet,alphabetsize>& r ):Fail<STravREV,t_alphabet, alphabetsize>( r ) {
   }
};

#undef IN_FAILS_HPP
#endif

