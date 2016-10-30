/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC
//


// Module for testing.
#include "com-misc.hpp"
#include "string.hpp"
//#include "set.hpp"
#include <set>
#include "ac/acs.hpp"
#include "cw/cws.hpp"
#include "kmp/pm-kmp.hpp"
#include "bm/bms.hpp"
#include "tries.hpp"
#include <iostream>
#include <vector>

// Keep track of the number of matches found.
static int count = 0;

// Some functions for the call-backs.
bool f1( int a ) {
	count++;
	std::cout << "match: " << a << '\n';
	return( true );
}

bool f2( int a, const kwset_t& O ) {
	count++;
	std::cout << "match: " << a << '\n' << O << '\n';
	return( true );
}

// The main test function.
int main() {
	auto kw_t p( "he" );

	auto kwset_t P;
	P.insert( "his" ); P.insert( "her" ); P.insert( "she" ); P.insert( "sher" );
	auto kw_t S( "hishershey" );

	auto Trie<STravFWD,standardAlphabet,standardAlphabetSize> O1( P );
#if HAVE_OSTREAM_OVERLOAD
	std::cout << O1;
#endif
	auto PMKMP<standardAlphabet,standardAlphabetSize> M1( p );
	M1.match( S, f1 );
   std::cout << count << std::endl;

	count = 0;
	auto PMACOpt_std M2( P );
	M2.match( S, f2 );
	std::cout << count << std::endl;

	count = 0;
	auto PMACFail_std M3( P );
	M3.match( S, f2 );
	std::cout << count << std::endl;

	count = 0;
	auto PMACKMPFail_std M4( P );
	M4.match( S, f2 );
	std::cout << count << std::endl;

	count = 0;
	auto PMCWNaive M48( P );
	M48.match( S, f2 );
	std::cout << count << std::endl;

	count = 0;
	auto PMCWNLA M5( P );
	M5.match( S, f2 );
	std::cout << count << std::endl;

	count = 0;
	auto PMCWWBM M58( P );
	M58.match( S, f2 );
	std::cout << count << std::endl;

	count = 0;
	auto PMCWNorm M6( P );
	M6.match( S, f2 );
	std::cout << count << std::endl;

	count = 0;
	auto PMCWOpt M7( P );
	M7.match( S, f2 );
	std::cout << count << std::endl;

	count = 0;
	auto PMCWRLA M8( P );
	M8.match( S, f2 );
	std::cout << count << std::endl;

	count = 0;
	auto PMBM< STravREV, SLNone, BMShift11<STravREV,standardAlphabet,standardAlphabetSize>,standardAlphabet,standardAlphabetSize > N1( p );
	N1.match( S, f1 );
	std::cout << count << std::endl;

	count = 0;
	auto PMBM< STravFWD, SLNone, BMShift12<STravFWD>,standardAlphabet,standardAlphabetSize > N2( p );
	N2.match( S, f1 );
	std::cout << count << std::endl;

	return( 0 );
}

