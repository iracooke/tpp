/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#ifndef PM_BFSIN_HPP
#define PM_BFSIN_HPP
#define IN_PM_BFSIN_HPP

#include "com-misc.hpp"
#include "pm-singl.hpp"
#include "string.hpp"
#include <assert.h>
#include <iostream>

// A Brute-Force single keyword pattern matcher.

template <class t_alphabet,const int alphabetsize> class PMBFSingle : public PMSingle<t_alphabet,alphabetsize> {
public:
	PMBFSingle( const kw_t& keyword );

	// Perform the actual matching.
	virtual void match( const kw_t &S,
			bool callBack (int) );

	bool c_inv() const;
	friend std::ostream& operator<<( std::ostream& os, const PMBFSingle& r );
private:
	#include "pm-bfsin_p.hpp"
};

#include "pm-bfsin_i.hpp"

#undef IN_PM_BFSIN_HPP
#endif

