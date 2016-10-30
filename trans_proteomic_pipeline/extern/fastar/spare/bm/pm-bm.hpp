/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */


#ifndef PM_BM_HPP
#define PM_BM_HPP
#define IN_PM_BM_HPP

#include "alphabet.hpp"
#include "pm-singl.hpp"
#include "string.hpp"
#include <assert.h>
#include <iostream>

// A single-keyword pattern matching class for the
// Boyer-Moore family of pattern matchers. It is template
// parameterized by the `match order', the `skip loop'
// and the `match information shifter'.
// MO must be one of the STrav... classes.
// MI will be a template class which is instantiated
// for the particular MO. For example, we could use:
//		PMBM< STravFWD, SLNone, BMShiftNaive<STravFWD> >

template<class MO, class SL, class MI, class t_alphabet, const int alphabetsize>
class PMBM : public PMSingle<t_alphabet,alphabetsize> {
public:
	PMBM( const kw_t& kw );

	virtual void match( const kw_t &S, bool callBack (int) );

	bool c_inv() const;
	template<class MO2, class SL2, class MI2>
		friend std::ostream& operator<<( std::ostream& os, const PMBM<MO2,SL2,MI2,t_alphabet, alphabetsize> r ); // forward
private:
	#include "pm-bm_p.hpp"
};

#include "pm-bm_i.hpp"

#undef IN_PM_BM_HPP
#endif

