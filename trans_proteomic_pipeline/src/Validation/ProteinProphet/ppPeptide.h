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
// ppPeptide.h: interface for the ppPeptide class.
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

#pragma warning(disable: 4786)

#if !defined(AFX_PEPTIDE_H__A003F5B4_8E62_49E2_BBA4_F7F3FA90E2B2__INCLUDED_)
#define AFX_PEPTIDE_H__A003F5B4_8E62_49E2_BBA4_F7F3FA90E2B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ppParentProtein.h"

class ppProtein; // forward ref

extern bool compareProteinNamesAsc( const ppProtein* first_prot, 
                                    const ppProtein* second_prot ); 

struct ltprotnam
{
  bool operator()(const ppProtein * s1, const ppProtein * s2) const
  {
    return compareProteinNamesAsc(s1,s2);
  }
};
// this shouldn't need to be sorted, but if its not the numbers shift a bit from the perl version
typedef std::map<ppProtein *,ppParentProtein *,ltprotnam> ppParentProteinList;

class ppPeptide  
{
public:
   // ppPeptide();                                       // no default constructor for now
   ppPeptide(    const char * newPeptideName,
               double newPeptideMass = 0,                 //&& should decide on a default value
               int newSpectrumQueryIndex = 0,             //&& should decide on a default value
               int newHitRank = 1,
               bool newUnique = true,
               double newSpectrumCounts = 0.0
               );
	~ppPeptide();                                        // destructor not virtual for now

   // set methods
   void setPeptideMass(          double newPeptideMass )             {
      m_PeptideMass = newPeptideMass;
   }
   void setSpectrumQueryIndex(   int newSpectrumQueryIndex )         {
      m_SpectrumQueryIndex = newSpectrumQueryIndex;
   }
   void setHitRank(              int newHitRank )                    {
      m_HitRank = newHitRank;
   }
   void setUnique(               bool newUnique )                    {
      m_Unique = newUnique;
   }
   void setSpectrumCounts(       double newSpectrumCounts )          {
      m_SpectrumCounts = newSpectrumCounts;
   }

   //increment method
   void incrementSpectrumCounts( double incrSpectrumCounts )         {
      m_SpectrumCounts += incrSpectrumCounts;
   }

   // get methods
   const char *   getName() const           {
      return m_PeptideName;
   }
   double               getPeptideMass() const           {
      return m_PeptideMass;
   }
   int                  getSpectrumQueryIndex() const    {
      return m_SpectrumQueryIndex;
   }
   int                  getHitRank() const               {
      return m_HitRank;
   }
   bool                 getUnique() const                {                     //&& could rename as isUnique
      return m_Unique;
   }
   double               getSpectrumCounts() const        {
      return m_SpectrumCounts;
   }

   ppParentProteinList &getParentProteinListNonConst() {
      return m_ParentProteins;
   }

   const ppParentProteinList &getParentProteinList() const {
      return m_ParentProteins;
   }

   ppParentProtein &getParentProtein(const ppProtein *protein) {
      ppParentProteinList::iterator iter = m_ParentProteins.find((ppProtein *)protein); // casting away const, yuck
      if (m_ParentProteins.end()==iter) { // not found, must add
         ppParentProtein *result = m_ParentProteins[(ppProtein *)protein]; // will create a new entry
         iter = m_ParentProteins.find((ppProtein *)protein); // cast away const, sorry
         iter->second = new ppParentProtein();
#ifdef _DEBUG
         iter->second->setDebugInfo(this,protein); // for debug convenience
#endif
      }
      return *(iter->second);
   }
   const bool hasParentProtein(const ppProtein *protein) const {
      return getParentProteinList().find((ppProtein *)protein) != getParentProteinList().end();
   }

   bool hasParentProteinNTT(const ppProtein *protein) {
      return getParentProtein(protein).hasNTT();
   }

   int getParentProteinNTT(const ppProtein *protein) {
      return getParentProtein(protein).getNTT();
   }

   double getParentProteinOrigPepWt(const ppProtein *protein) {
      return getParentProtein(protein).getOrigPepWt();
   }

   double getParentProteinPepWt(const ppProtein *protein) {
      return getParentProtein(protein).getPepWt();
   }


   double getParentProteinMaxProb(const ppProtein *protein) {
      return getParentProtein(protein).getMaxProb();
   }
   void clearProteinMaxProb() {
      ppParentProteinList& prot_ref = getParentProteinListNonConst();
      for (ppParentProteinList::iterator prot_iter = prot_ref.begin();
         prot_iter != prot_ref.end(); prot_iter++ ) {
         prot_iter->second->setMaxProb(UNINIT_VAL);
      }
   };
   void clearProteinOrigMaxProb() {
      ppParentProteinList& prot_ref = getParentProteinListNonConst();
      for (ppParentProteinList::iterator prot_iter = prot_ref.begin();
         prot_iter != prot_ref.end(); prot_iter++ ) {
         prot_iter->second->setOrigMaxProb(UNINIT_VAL);
      }
   };

private:
                                                         // required in pepXML schema:
   const char * m_PeptideName; // note we don't alloc it //    yes
   double m_PeptideMass;                                 //    yes
   int    m_SpectrumQueryIndex;                          //    yes
   int    m_HitRank;                                     //    yes
   bool   m_Unique;                                      //    (does not appear in pepXML schema)
   double m_SpectrumCounts;                              //    (does not appear in pepXML schema)
   ppParentProteinList    m_ParentProteins;                //    (does not appear in pepXML schema)

// these attributes in pepXML_v18.xsd are not used by ProteinProphet
//    ( to be populated )

};

#include <set>
// do NOT change this collection class!
// There are other places in the code that will 
// break if you change the sense of "less than" 
// here, such as the group_helper() function in 
// ProteinProphet.cpp.  
struct ltpep
{
  bool operator()(ppPeptide* p1, ppPeptide* p2) const
  {
    return p1 < p2;
  }
};

typedef std::set<ppPeptide *, ltpep> orderedPeptideList; 
// optimizations rely on this being sorted by address                                                
// as opposed to alpha sort etc

#endif // !defined(AFX_PEPTIDE_H__A003F5B4_8E62_49E2_BBA4_F7F3FA90E2B2__INCLUDED_)
