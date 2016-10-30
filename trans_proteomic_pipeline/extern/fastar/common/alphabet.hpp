/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

// templated alphabet for more efficient peptide search Oct 2006 bpratt Insilicos LLC


#ifndef ALPHABET_HPP
#define ALPHABET_HPP
#define IN_ALPHABET_HPP

#include <limits.h>
#include <assert.h>
#include <ctype.h>

#define alphabetNormalize t_alphabet::Normalize
#define alphabetDenormalize t_alphabet::Denormalize


class standardAlphabet  {
public:
   static char Normalize( char a ) {
      return( a );
   }
   static char Denormalize( char a ) {
      return( a );
   }
};
const int standardAlphabetSize = CHAR_MAX;

#ifdef ALPHABET_HOME
#define ALPHABET_HOME_EXTERN
#else
#define ALPHABET_HOME_EXTERN extern
#endif
ALPHABET_HOME_EXTERN int peptideAlphabetNormalizeTable[CHAR_MAX];
ALPHABET_HOME_EXTERN int peptideAlphabetDenormalizeTable[CHAR_MAX];

class peptideAlphabet {
public:
   // Functions used to take care of having different alphabets.
   // The alphabet is considered to consist only of the positive
   // characters.
   // Ensure that alphabetNormalize( 0 ) = 0.
   // Change them to support other alphabets, e.g. a,c,t,g.

   static void init() { // call this once to init fast lookup table
      for (int i=CHAR_MAX;i--;) {
         peptideAlphabetNormalizeTable[i] = slowNormalize(i);
      }
      for (int j=CHAR_MAX;j--;) {
         peptideAlphabetDenormalizeTable[peptideAlphabetNormalizeTable[j]] = j;
      }
   }
      
   // Map the alphabet to [0..m_alphabetsize).
   static char Normalize( char a ) {
      return peptideAlphabetNormalizeTable[a]; // did you call init() first?
   }
   static char slowNormalize( char a ) { // additional characters can be added here
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
      case 'u':	return( 24 );
      case '*':	return( 25 );
      case '-':	return( 26 );
      default:	return(0);
      }
      return(0);
   }
   
   static bool legal_string(const char *str) {
      for (const char *c=str;*c;c++) {
         if (Normalize(*c)<=0) {
            return false;
         }
      }
      return true;
   }

   // Inverse of alphabetNormalize().
   static char Denormalize( char a ) {
      return peptideAlphabetDenormalizeTable[a]; // did you call init() first?
   }
};
const int peptideAlphabetSize=27; 

#undef IN_ALPHABET_HPP
#endif

// update this macro when you add an alphabet
#define FASTAR_INSTANTIATE(myclass) \
   template class myclass<standardAlphabet,standardAlphabetSize>;\
   template class myclass<peptideAlphabet,peptideAlphabetSize>;
