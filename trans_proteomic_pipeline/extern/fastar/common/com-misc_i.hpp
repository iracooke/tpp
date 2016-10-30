/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.
//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//

// This stuff is always inline.
#ifndef min
inline int min( const int a, const int b ) {
	return( a < b ? a : b );
}

inline int max( const int a, const int b ) {
	return( a < b ? b : a );
}
#endif

inline bool shorter( const kw_t &a, const kw_t &b ) {
	return( a.length() < b.length() );
}
