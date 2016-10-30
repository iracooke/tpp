/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.
//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

#ifndef COMIOHPP
#define COMIOHPP

#include "com-misc.hpp"

template<class T>
std::ostream& operator<<( std::ostream& os, const std::vector<T>& v ) {
	os << "Vector = (\n";
	auto typename std::vector<T>::const_iterator iter;
	auto int pos = 0;
	for( iter = v.begin(); iter != v.end(); ++iter, ++pos ) {
    	os << pos << "->" << *iter << "\n";
	}
	os << ")\n";
	return os;
}

template<class T>
std::ostream& operator<<( std::ostream& os, const std::set<T>& v ) {
	os << "Set = (\n";
	auto typename std::set<T>::const_iterator iter;
	auto int pos = 0;
	for( iter = v.begin(); iter != v.end(); ++iter, ++pos ) {
    	os << pos << "->" << *iter << "\n";
	}
	os << ")\n";
	return os;
}


#endif // COMIOHPP
