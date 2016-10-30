/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.
//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC
//
// fgrep example.



//#include <fstream.h>
#include <fstream>
#include "string.hpp"
#include "bm/bms.hpp"

static int counter = 0;

static bool repAll( int where ) {
	counter++;
	std::cout << "\nEnding at " << where;
	return( true );
}

static bool countAll( int where ) {
	counter++;
	return( true );
}

int main( int argc, const char* argv[] ) {
	if( argc != 4 || !(argv[1][0] == '-'
			&& (argv[1][1] == 'n' || argv[1][1] == 'c')) ) {
		std::cerr << "Usage: bmgrep -{n,c} keyword inputfile\n";
		return( -1 );
	}
	
	auto kw_t p( argv[2] );
	auto PMBM< STravREV, SLFast1<standardAlphabet,standardAlphabetSize>, BMShift11< STravREV,standardAlphabet,standardAlphabetSize>,standardAlphabet,standardAlphabetSize> M( p );

	auto std::ifstream incoming( argv[3] );
   auto std::string S;
	auto char a;
	incoming.seekg( 0, std::ios::beg );
	while( incoming.get( a ) ) {
      char c = M.Normalize(a);
      if (0 <= c && c < M.Alphabetsize()) {
			S += a;
		}
	}

	M.match( S.c_str(), argv[1][1] == 'n' ? repAll : countAll );
	std::cout << "\n\n" << "Count : " << counter << std::endl;

	return( 0 );
}

