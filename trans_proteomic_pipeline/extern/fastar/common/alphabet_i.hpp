/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.
// $Revision: 1714 $
// $Date: 2006-10-18 15:50:40 -0700 (Wed, 18 Oct 2006) $

// This stuff is always inline. To out-of-line them, create
// an alphabet.cpp file.

// These two are just the identity function for now.
// Change them to support other alphabets, e.g. a,c,t,g.

#ifdef _MSC_VER // microsoft weirdness
#pragma warning(disable:4786) // don't bark about "identifier was truncated to '255' characters in the browser information"
#endif

inline char alphabetNormalize( const char a ) {
  //	return( a );
  char tmp = tolower(a);
  switch( tmp ) {
  case 0:		return( 0 );
  case 'a':	return( 1 );
  case 'r':	return( 2 );
  case 'n':	return( 3 );
  case 'd':	return( 4 );
  case 'c':	return( 5 );
  case 'q':	return( 6 );
  case 'e':	return( 7 );
  case 'g':	return( 8 );
  case 'h':	return( 9 );
  case 'i':	return( 10 );
  case 'l':	return( 11 );
  case 'k':	return( 12 );
  case 'm':	return( 13 );
  case 'f':	return( 14 );
  case 'p':	return( 15 );
  case 's':	return( 16 );
  case 't':	return( 17 );
  case 'w':	return( 18 );
  case 'y':	return( 19 );
  case 'v':	return( 20 );
  case 'b':	return( 21 );
  case 'z':	return( 22 );
  case 'x':	return( 23 );
  default:	assert( !"Incorrect character" );
  }

}

inline char alphabetDenormalize( const char a ) {
  //	return( a );
  switch( a ) {
  case 0:		return( 0 );
  case 1:		return( 'a' );
  case 2:		return( 'r' );
  case 3:		return( 'n' );
  case 4:		return( 'd' );
  case 5:		return( 'c' );
  case 6:		return( 'q' );
  case 7:		return( 'e' );
  case 8:		return( 'g' );
  case 9:		return( 'h' );
  case 10:		return( 'i' );
  case 11:		return( 'l' );
  case 12:		return( 'k' );
  case 13:		return( 'm' );
  case 14:		return( 'f' );
  case 15:		return( 'p' );
  case 16:		return( 's' );
  case 17:		return( 't' );
  case 18:		return( 'w' );
  case 19:		return( 'y' );
  case 20:		return( 'v' );
  case 21:		return( 'b' );
  case 22:		return( 'z' );
  case 23:		return( 'x' );
  default:	assert( !"Incorrect image" );
  }
  
}

