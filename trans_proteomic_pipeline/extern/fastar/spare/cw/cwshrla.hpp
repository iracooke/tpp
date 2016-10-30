/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC
//

#ifndef CWSHRLA_HPP
#define CWSHRLA_HPP
#define IN_CWSHRLA_HPP

#include "state.hpp"
#include "stateto.hpp"
#include "string.hpp"
//#include "set.hpp"
#include <set>
#include "tries.hpp"
#include "fails.hpp"
#include "cwout.hpp"
#include "cwdopt.hpp"
#include "cwd2.hpp"
#include "cwcharrl.hpp"
#include "com-misc.hpp"
#include "fails.hpp"
#include <assert.h>
#include <iostream>

// Optimized Commentz-Walter shifter (Fan and Su).

template <class t_alphabet,const int alphabetsize> class CWShiftRLA {
public:
	CWShiftRLA( const kwset_t& P,
			const RTrie<t_alphabet,alphabetsize>& t,
			const CWOutput<t_alphabet,alphabetsize>& out );
	CWShiftRLA( const CWShiftRLA& r );
	~CWShiftRLA();

	// The shifter itself.
	int shift( const RTrie<t_alphabet,alphabetsize>& t,
			const char l,
			const State v,
			const char r ) const;
	bool c_inv() const;
	template <class t_alphabet2,const int alphabetsize2> friend std::ostream& operator<<( std::ostream& os, const CWShiftRLA<t_alphabet2,alphabetsize2>& r );
private:
	#include "cwshrla_p.hpp"
};

#include "cwshrla_i.hpp"

#undef IN_CWSHRLA_HPP
#endif

