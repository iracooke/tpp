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
//
// DESCRIPTION:
//
// fast_map, a convenience class for hashtables 
//
// NOTES:
//
//
//
// TODO:
//
//
//
//////////////////////////////////////////////////////////////////////

#pragma warning(disable: 4786)

#if !defined(AFX_FASTMAP_H__A003F5B4_8E62_49E2_BBA4_F7F3FA90E2B2__INCLUDED_)
#define AFX_FASTMAP_H__A003F5B4_8E62_49E2_BBA4_F7F3FA90E2B2__INCLUDED_

#include "common/tpp_hashmap.h"  // defines TPP_HASHMAP(T)

template <class T> class fast_map : public TPP_CONSTCHARP_HASHMAP(T) 
{
#ifdef _MSC_VER  // why does GCC hate this? who knows, just let it leak
public:
   void free_keys() { // free the keys, assumed produced by strdup()
      std::vector<char *> keys;
      for (TPP_CONSTCHARP_HASHMAP(T)::iterator iter = this->begin();iter!=this->end();++iter) {
         keys.push_back((char *)iter->first);
      }
      for (int i=keys.size();i--;) {
         free(keys[i]);
      }
   }
   void print_keys() { // print the keys
      for (TPP_CONSTCHARP_HASHMAP(T)::iterator iter = this->begin();iter!=this->end();++iter) {
         std::cout<<iter->first<<"\n";
      }
   }
   void delete_values() { // free the values, assumed produced by new
      std::vector<T> values;
      for (TPP_CONSTCHARP_HASHMAP(T)::iterator iter = this->begin();iter!=this->end();++iter) {
         values.push_back(iter->second);
      }
      for (int i=values.size();i--;) {
         delete values[i];
      }
   }
   void free_values() { // free the values, assumed produced by malloc
      std::vector<T> values;
      for (TPP_CONSTCHARP_HASHMAP(T)::iterator iter = this->begin();iter!=this->end();++iter) {
         valuess.push_back(iter->second);
      }
      for (int i=values.size();i--;) {
         free(values[i]);
      }
   }
#endif
};

#endif // !defined(AFX_FASTMAP_H__A003F5B4_8E62_49E2_BBA4_F7F3FA90E2B2__INCLUDED_)
