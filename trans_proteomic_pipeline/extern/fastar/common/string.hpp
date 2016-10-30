/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts and FIRE Lite class libraries.


#ifndef STRING_HPP
#define STRING_HPP
#define IN_STRING_HPP

#ifdef _MSC_VER // microsoft weirdness
#pragma warning(disable:4786) // don't bark about "identifier was truncated to '255' characters in the browser information"
#endif



#include <string>

///*
//#include "com-misc.hpp"
//#include <iostream>
//#include <assert.h>
//
//// Represent strings at a higher level than char *.
// This class can be replaced by a standard library class, when
// those become available.
//
//class std::string {
//public:
//	std::string();
//	std::string( const char *str );
//	std::string( const std::string& str );
//	std::string( istream& is );
//	~std::string();
//	const std::string& operator=( const std::string& str );
//
//	int operator==( const std::string& str ) const;
//	int operator!=( const std::string& str ) const;
//	int length() const;
//	char operator[]( const int index ) const;
//	bool c_inv() const;
//	friend ostream& operator<<( ostream& os, const std::string& str );
////private:
////	#include "string.ppp"
////};
//
//#include "string.ipp"
//*/

#undef IN_STRING_HPP
#endif

