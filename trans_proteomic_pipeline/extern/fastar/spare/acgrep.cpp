/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.
//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//
// fgrep example.



//#include <fstream.h>
#include <fstream>
#include "string.hpp"
//#include "set.hpp"
#include <set>
#include "ac/acs.hpp"

static int counter = 0;

static bool repAll( int where, const kwset_t& what ) {
	counter++;
	std::cout << "\nEnding at " << where << " : " << what;
	return( true );
}

static bool countAll( int where, const kwset_t& what ) {
	counter++;
	return( true );
}

int main( int argc, const char* argv[] ) {
	if( argc < 4 || !(argv[1][0] == '-'
			&& (argv[1][1] == 'n' || argv[1][1] == 'c')) ) {
		std::cerr << "Usage: acgrep -{n,c} keyword1 ... keywordn inputfile\n";
		return( -1 );
	}
	
	auto kwset_t P;
	for( int i = 2; i < argc - 1; i++ ) {
		P.insert( argv[i] );
	}
	auto PMACOpt_std M( P );

	auto std::ifstream incoming( argv[argc - 1] );
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

