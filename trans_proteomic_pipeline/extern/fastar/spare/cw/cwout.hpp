/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC
//

#ifndef CWOUT_HPP
#define CWOUT_HPP
#define IN_CWOUT_HPP

#include "state.hpp"
#include "string.hpp"
//#include "set.hpp"
#include <set>
#include <iterator>
#include "stateto.hpp"
#include "stravrev.hpp"
#include "tries.hpp"
#include <assert.h>
#include <iostream>

// The Commentz-Walter output function.

template <class t_alphabet,const int alphabetsize> class CWOutput {
public:
	CWOutput( const kwset_t& P, const RTrie<t_alphabet,alphabetsize>& t );
	CWOutput( const CWOutput<t_alphabet,alphabetsize>& r );
	~CWOutput();
	
	// Is q a keyword?
	int isKeyword( const State q ) const;
	// Yes, what is the keyword?
	const kw_t& operator[]( const State q ) const;
	bool c_inv() const;
	template <class t_alphabet2,const int alphabetsize2> friend std::ostream& operator<<( std::ostream& os, const CWOutput<t_alphabet2,alphabetsize2>& r );
private:
	#include "cwout_p.hpp"
};

#include "cwout_i.hpp"

#undef IN_CWOUT_HPP
#endif

