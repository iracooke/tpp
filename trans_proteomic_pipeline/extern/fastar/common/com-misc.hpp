/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts and FIRE Lite class libraries.
//
// modified for nearly-zero-copy operation by bpratt Insilicos LLC, July 2006 
//
#ifndef COM_MISC_HPP
#define COM_MISC_HPP
#define IN_COM_MISC_HPP

#include <limits.h>
#include <string>
#include <ostream.h>

// Give some miscellaneous functions for SPARE Parts and
// FIRE Lite.
#ifndef min
int min( const int a, const int b );
int max( const int a, const int b );
#endif

//
// bpratt Insilicos LLC July 2006
//
// replacing std::string with this:
// like std::string but no deep copy  
// without it we get as many as 3 copies of each keyword
// extant during trie build
//
class shallowstring { 
public:
   shallowstring() : 
      m_str(""),m_len(0) {
   }
   shallowstring(const std::string &str) : 
      m_str(str.c_str()),m_len(str.length()) 
   {};
   shallowstring(const shallowstring &str) :
      m_str(str.c_str()),m_len(str.length()) 
   {};
   shallowstring(const char *str) : 
      m_str(str),m_len(strlen(str)) 
   {};
   shallowstring * operator = (const shallowstring &str) {
      m_str = str.c_str();
      m_len = str.length();
      return this;
   }
   bool operator < (const shallowstring &str) const {
      return (strcmp(m_str,str.c_str())<0);
   }
   char operator [] (int index) const {
      return m_str[index];
   }
   int length() const {
      return (int)m_len;
   }
   const char *c_str() const {
      return m_str;
   }
   //
   // data
   //
private:
   const char *m_str;
   size_t m_len;
};

inline std::ostream& operator<<( std::ostream& os, const shallowstring &str) {
   os << str.c_str();
   return os;
}

#define kw_t shallowstring 
#include <set>
typedef std::set<kw_t> kwset_t;



#include "com-misc_i.hpp"

// And the units of these operators.
const int PLUSINFINITY = INT_MAX;
const int MINUSINFINITY = INT_MIN;


#undef IN_COM_MISC_HPP
#endif

