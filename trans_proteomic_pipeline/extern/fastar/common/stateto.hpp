/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts and FIRE Lite class libraries.

// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC


#ifndef STATETO_HPP
#define STATETO_HPP
#define IN_STATETO_HPP

#include "state.hpp"
//#include "array.hpp"
#include <vector>
#include <assert.h>
#include <iostream>

// Provide a mapping from a State to a T.

template<class T,class t_alphabet, const int alphabetsize>
class StateTo {
public:
	// Give *this an initial capacity.
	StateTo( const int size = 0 );
	StateTo( const StateTo<T,t_alphabet, alphabetsize>& r );

	// Some mapping operators.
	// A changeable map operator.
	T& map( const State index );

	// A constant mapping operator.
	const T& operator[]( const State index ) const;

	void setSize( const int s );
	int size() const;
	bool c_inv() const;
	template<class T2,class t_alphabet2,const int alphabetsize2>
		friend std::ostream& operator<<( std::ostream& os, const StateTo<T2,t_alphabet2, alphabetsize2>& t ); // forward
private:
	#include "stateto_p.hpp"
};

#include "stateto_i.hpp"

#undef IN_STATETO_HPP
#endif

