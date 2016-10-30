/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC
//

#ifndef CWSHIFTNORM_HPP
#define CWSHIFTNORM_HPP
#define IN_CWSHIFTNORM_HPP

#include "state.hpp"
#include "stateto.hpp"
#include "string.hpp"
//#include "set.hpp"
#include <set>
#include "tries.hpp"
#include "fails.hpp"
#include "cwout.hpp"
#include "cwchar.hpp"
#include "cwd1.hpp"
#include "cwd2.hpp"
#include "com-misc.hpp"
#include "fails.hpp"
#include <assert.h>
#include <iostream>

// Normal Commentz-Walter shifter.

template <class t_alphabet,const int alphabetsize> class CWShiftNorm {
public:
	CWShiftNorm( const kwset_t& P,
			const RTrie<t_alphabet,alphabetsize>& t,
			const CWOutput<t_alphabet,alphabetsize>& out );
	CWShiftNorm( const CWShiftNorm<t_alphabet,alphabetsize>& r );
	~CWShiftNorm();

	// The shifter itself.
	int shift( const RTrie<t_alphabet,alphabetsize>& t,
			const char l,
			const State v,
			const char r ) const;
	bool c_inv() const;
	template <class t_alphabet2,const int alphabetsize2> friend std::ostream& operator<<( std::ostream& os, const CWShiftNorm<t_alphabet2,alphabetsize2>& r );
private:
	#include "cwshnorm_p.hpp"
};

#include "cwshnorm_i.hpp"

#undef IN_CWSHIFTNORM_HPP
#endif

