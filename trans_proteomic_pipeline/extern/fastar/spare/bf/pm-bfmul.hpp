/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#ifndef PM_BFMUL_HPP
#define PM_BFMUL_HPP
#define IN_PM_BFMUL_HPP

#include "com-misc.hpp"
#include "pm-multi.hpp"
#include "string.hpp"
//#include "set.hpp"
#include <set>
#include <assert.h>
#include <iostream>

// A Brute-Force multi keyword pattern matcher.

template <class t_alphabet,const int alphabetsize> class PMBFMulti : public PMMultiple<t_alphabet,alphabetsize> {
public:
	PMBFMulti( const kwset_t& keywords );

	// Perform the actual matching.
	virtual void match( const kw_t &S,
		bool callBack (int, const kwset_t&) );

	bool c_inv() const;
	friend std::ostream& operator<<( std::ostream& os, const PMBFMulti<t_alphabet,alphabetsize>& r );
private:
	#include "pm-bfmul_p.hpp"
};

#include "pm-bfmul_i.hpp"

#undef IN_PM_BFMUL_HPP
#endif

