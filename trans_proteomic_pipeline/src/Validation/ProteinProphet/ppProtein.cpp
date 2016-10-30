// Copyright (c) 2006, 2007 Insilicos LLC and LabKey Software. All rights reserved.
//
// Ported from the Perl module "ProteinProphet.pl", which is 
// copyright Andy Keller and the Institute for Systems Biology.
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
// ppProtein.cpp: implementation of the ppProtein class.
// C++ version of ProteinProphet, originated by Jeff Howbert for Insilicos LLC
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

#include "ppProtein.h"
#include "ppPeptide.h"
#include <algorithm>

extern ppProtein *getProteinByName(const char *name); // in ProteinProphet.cpp


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ppProtein::ppProtein(    const char * newProteinName) :
   m_ProteinName( newProteinName ),
   m_Annotation(""),
   m_Probability(UNINIT_VAL ),
   m_ProteinLen(0),
   m_ProteinMW(0.0),
   m_FinalProbability(UNINIT_VAL ),
   m_Coverage( UNINIT_VAL ),
   m_Confidence( UNINIT_VAL ),
   m_GroupMembership( UNINIT_VAL ),
   m_SubsumingProtein(NULL),
   m_Member( false ),
   m_Degen( false ),
   m_PeptidesInProt(),
   m_DegenList()
{
   if (strchr(m_ProteinName,' ')) { // this is a concatenation
      char *tmp=strdup(m_ProteinName);
      char *name = tmp;
      while (name && *name) {
         char *space = strchr(name,' ');
         if (space) {
            *space = 0;
         }
         m_DegenList.push_back(getProteinByName(name));
         if (space) {
            name = space+1;
         } else {
            break;
         }
      }
      free(tmp);
      // sort degen protein list by name
      std::sort(m_DegenList.begin(),m_DegenList.end(),compareProteinNamesAsc );
      m_Degen = true;
   } else {
      m_DegenList.push_back(this); // point to self for convenience
   }
   test_invariant();
}

ppProtein::~ppProtein()
{
}

void ppProtein::outputPeptidesInProt()
{
   test_invariant(); // internal sanity check
   orderedPeptideList::iterator iter = m_PeptidesInProt.begin();
   while ( iter != m_PeptidesInProt.end() )
   {
      std::cout << "      " << ( *iter ) -> getName() << std::endl;
      iter++;
   }
}

//##############################################################################
// function compareProteinNamesAsc
//		input:	ppProtein* - pointer to first protein
//    input:   ppProtein* - pointer to second protein
//		output:	true if first protein is lower alphabetically
//
// supports sorting of an STL vector of proteins by protein name in ascending order
//##############################################################################
bool compareProteinNamesAsc( const ppProtein* first_prot, const ppProtein* second_prot )  
{
   return (strcmp(first_prot -> getName(),second_prot -> getName()) <0 );
}


