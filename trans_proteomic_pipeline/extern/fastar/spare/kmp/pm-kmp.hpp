/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC
//

#ifndef PM_KMP_HPP
#define PM_KMP_HPP
#define IN_PM_KMP_HPP

#include "alphabet.hpp"
#include "pm-singl.hpp"
#include "string.hpp"
#include "failidx.hpp"
#include <assert.h>
#include <iostream>

// A concrete single-keyword pattern matching class for
// the Knuth-Morris-Pratt pattern matching algorithm.

template <class t_alphabet,const int alphabetsize> class PMKMP : public PMSingle<t_alphabet,alphabetsize> {
public:
	PMKMP( const kw_t &kw );
	PMKMP( const PMKMP<t_alphabet,alphabetsize>& r );

	// The matcher.
	virtual void match( const kw_t &S, bool callBack (int) );
	bool c_inv() const;
	template <class t_alphabet2,const int alphabetsize2> friend std::ostream& operator<<( std::ostream& os, const PMKMP<t_alphabet2,alphabetsize2>& t );
private:
	#include "pm-kmp_p.hpp"
};

#include "pm-kmp_i.hpp"

#undef IN_PM_KMP_HPP
#endif

