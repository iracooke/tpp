/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#ifndef PM_CW_HPP
#define PM_CW_HPP
#define IN_PM_CW_HPP

#include "alphabet.hpp"
#include "tries.hpp"
#include "cwout.hpp"
#include "pm-multi.hpp"
//#include "set.hpp"
#include <set>
#include "string.hpp"
#include <assert.h>
#include <iostream>

// A multiple-keyword pattern matching class for the
// Commentz-Walter family of pattern matchers.
// The template parameter must be a Commentz-Walter shifter,
// ie. one of the CWShift... classes.

template<class T,class t_alphabet, const int alphabetsize>
class PMCW : public PMMultiple<t_alphabet,alphabetsize> {
public:
	PMCW( const kwset_t& P );
	PMCW( const PMCW<T,t_alphabet, alphabetsize>& r );

	// The actual matching function.
	virtual void match( const kw_t &S,
		bool callBack (int, const kwset_t&) );

	bool c_inv() const;
	template<class T2>
		friend std::ostream& operator<<( std::ostream& os, const PMCW<T2,t_alphabet, alphabetsize> r ); // forward
private:
	#include "pm-cw_p.hpp"
};

#include "pm-cw_i.hpp"

#undef IN_PM_CW_HPP
#endif

