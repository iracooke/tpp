//
// Copyright (c) 2006, 2007 Insilicos LLC and LabKey Software. All rights reserved.
//
// This library is free software; you can redistribute it and/or 
// modify it under the terms of the GNU Lesser General Public 
// License as published by the Free Software Foundation; either 
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public 
// License along with this library; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
// 
// Brian Pratt 
// Insilicos LLC 
// www.insilicos.com
//
// DESCRIPTION:
//
// CheapRegEx, an unsophisticated but fast regexp to be used in place of RegExp where possible.
//
// NOTES:
//
// Two changes, Tom Blackwell, 6/12/2007, both marked by "// was: ...".  
// These fix the case where the first matching line in the target file 
// uses empty string "" as the attribute's value, and eliminate one 
// unnecessary byte from  m_seekstr  (a purely cosmetic change).  
//
// For anyone else reading the code, it may help to point out that when 
// assigning  m_seekstr  from  exp  in the constructor, we will right-
// shift by 3 chars, but need to include the two additional chars '="'.  
// This makes the arithmetic for  m_seeklen  work out correctly.  
//
// tblackw@umich.edu
//
// (Tom's changes got me looking harder at my own code, I added a
//  missing ctor init - BSP)
//
// TODO:
//
//
//
//////////////////////////////////////////////////////////////////////

#pragma warning(disable: 4786)

#if !defined(AFX_CHEAPREGEX_H__A003F5B4_8E62_49E2_BBA4_F7F3FA90E2B2__INCLUDED_)
#define AFX_CHEAPREGEX_H__A003F5B4_8E62_49E2_BBA4_F7F3FA90E2B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <assert.h>

class CheapRegEx  
{
public:
   CheapRegEx( const char * exp ) : m_result(NULL),m_seekstr(NULL), m_reslen(-1) {  // was: m_reslen(0)
      const char *eq=strchr(exp,'='); // expecting something like "\s+spectrum="(\S+)""
      if (!strncmp(exp,"\\s+",3) && eq && !strcmp(eq+1,"\"(\\S+)\"")) {
         m_seekstr = (char *)malloc((eq-exp));	 	    // was: malloc((eq-exp)+1)
         memmove(m_seekstr,exp+3,m_seeklen = (eq-exp)-1);
         m_seekstr[m_seeklen] = 0;
      } else {
         assert(false); // oops, not a simple regexp
      }
   }
   ~CheapRegEx(){
      free(m_result);
      free(m_seekstr);
   };          
   
   bool Search(const char *buf) {
      const char *found = strstr(buf,m_seekstr);
      if (found) {
         found+=m_seeklen;
         const char *close = strchr(found,'\"');
         if (close) {
            int foundlen = close-found;
            if (foundlen>m_reslen) {
               m_result = (char *)realloc(m_result,(m_reslen=foundlen)+1);
            }
            memcpy(m_result,found,foundlen);
            m_result[foundlen] = 0;
            return true;
         }
      }
      return false;
   }

   const char * result() const {
      return m_result;
   }

private:
   char *m_result;
   int m_reslen;
   char *m_seekstr;
   int m_seeklen;
};

#endif // !defined(AFX_CHEAPREGEX_H__A003F5B4_8E62_49E2_BBA4_F7F3FA90E2B2__INCLUDED_)
