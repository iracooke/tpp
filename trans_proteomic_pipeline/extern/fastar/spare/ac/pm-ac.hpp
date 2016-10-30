/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#ifndef PM_AC_HPP
#define PM_AC_HPP
#define IN_PM_AC_HPP

#include "alphabet.hpp"
#include "pm-multi.hpp"
#include "state.hpp"
#include "string.hpp"
//#include "set.hpp"
#include <set>
#include <assert.h>
#include <iostream>

// A multiple-keyword pattern matching class for the
// Aho-Corasick pattern matching algorithms.
// The template parameter must be one of the ACMachine... classes.

template<class T,class t_alphabet, const int alphabetsize>
class PMAC : public PMMultiple<t_alphabet,alphabetsize> {
public:
	PMAC( const kwset_t& P );
	PMAC( const PMAC<T,t_alphabet, alphabetsize>& M );

	virtual void match( const kw_t &S,
		bool callBack (int, const kwset_t&) );

	bool c_inv() const;
	template<class T2>
		friend std::ostream& operator<<( std::ostream& os, const PMAC<T2,t_alphabet, alphabetsize>& r ); // forward
private:
	#include "pm-ac_p.hpp"
};

#include "pm-ac_i.hpp"

#undef IN_PM_AC_HPP
#endif

