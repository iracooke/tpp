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
// Protein.h: interface for the Protein class.
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

#if !defined(AFX_PROTEIN_H__FC6E0D1B_54DA_4861_A764_1014AAFAA45D__INCLUDED_)
#define AFX_PROTEIN_H__FC6E0D1B_54DA_4861_A764_1014AAFAA45D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <assert.h>
#include <math.h>

#include "ppPeptide.h"
class ppProtein; // forward ref

typedef std::vector< ppProtein* > degenList;

// stuff for working with probabilites at output resolution of .01
inline double roundProtProbability(double newProbability) {
   char buf[512];
   snprintf(buf,sizeof(buf),"%0.4f",newProbability); // so we're using same rounding algo as outputs, so sort makes sense to user
   return atof(buf);
}
inline int rankProtProbability(double newProbability) {
   return (int)( (roundProtProbability(newProbability) * 10000.0) + 0.5) ; 
}

class ppProtein  
{
public:
   ppProtein() {
      assert(false); // we're going for a zero-copy design, no collection should need or use this
   };
   ppProtein(    const char * newProteinName   );
	~ppProtein();                                                       // destructor not virtual for now

   // set methods
   void setAnnotation(        const char *newAnnotation )     {
     m_Annotation = newAnnotation;
     test_invariant(); // internal sanity check
   }
   void setAnnotation(        const std::string& newAnnotation )     {
     m_Annotation = newAnnotation;
     test_invariant(); // internal sanity check
   }
   void setProbability(       double newProbability )                {
      m_Probability = newProbability; 
      test_invariant(); // internal sanity check
   }
   void setFPKM(       double newFPKM )                {
     m_FPKM = newFPKM;
      test_invariant(); // internal sanity check
   }
   void setProteinLen(       unsigned int newProteinLen )                {
      m_ProteinLen = newProteinLen; 
      test_invariant(); // internal sanity check
   }
   void setProteinMW (       double newProteinMW )                {
      m_ProteinMW = newProteinMW; 
      test_invariant(); // internal sanity check
   }
   void setFinalProbability(  double newFinalProbability )           {
      m_FinalProbability = newFinalProbability;
      test_invariant(); // internal sanity check
   }
   void setCoverage(          double newCoverage )                   {
      m_Coverage = newCoverage;
      test_invariant(); // internal sanity check
   }

   void setConfidence(          double newConfidence )                   {
      m_Confidence = newConfidence;
      test_invariant(); // internal sanity check
   }

   void setGroupMembership(   int newGroupMembership )               {
      m_GroupMembership = newGroupMembership;
      test_invariant(); // internal sanity check
   }
   void setMember(            bool newMember )                       {
      m_Member = newMember;
      test_invariant(); // internal sanity check
   }
   void setSubsumingProtein(ppProtein * subsumed_by)          {
      m_SubsumingProtein = subsumed_by;
      test_invariant(); // internal sanity check
   }

   // get methods
   const char * getName() const        {
      test_invariant(); // internal sanity check
      return m_ProteinName;
   }
   const degenList &getDegenList() const { // will always contain at list one (this)
      test_invariant(); // internal sanity check
      return m_DegenList; // useful when name is a concatenation
   }
   const std::string&   getAnnotation() const         {
      test_invariant(); // internal sanity check
      return m_Annotation;
   }
   bool hasProbability() const        {
      test_invariant(); // internal sanity check
      return isInit(m_Probability);
   }
   double getProbability() const        {
      test_invariant(); // internal sanity check
      PARANOID_ASSERT(isInit(m_Probability));
      return m_Probability;
   }
   double getFPKM() const        {
      test_invariant(); // internal sanity check
      PARANOID_ASSERT(isInit(m_FPKM));
      return m_FPKM;
   }
   unsigned int getProteinLen() const        {
      test_invariant(); // internal sanity check
      PARANOID_ASSERT(isInit(m_ProteinLen));
      return m_ProteinLen;
   }
   double getProteinMW() const        {
      test_invariant(); // internal sanity check
      PARANOID_ASSERT(isInit(m_ProteinMW));
      return m_ProteinMW;
   }
   bool hasFinalProbability() const   {
      test_invariant(); // internal sanity check
      return isInit(m_FinalProbability);
   }
   double getFinalProbability() const   {
      test_invariant(); // internal sanity check
      PARANOID_ASSERT(isInit(m_FinalProbability));
      return m_FinalProbability;
   }
   bool hasCoverage() const           {
      test_invariant(); // internal sanity check
      return isInit(m_Coverage);
   }
   double getCoverage() const           {
      test_invariant(); // internal sanity check
      PARANOID_ASSERT(isInit(m_Coverage));
      return m_Coverage;
   }
   bool hasConfidence() const           {
      test_invariant(); // internal sanity check
      return isInit(m_Confidence);
   }
   double getConfidence() const           {
      test_invariant(); // internal sanity check
      PARANOID_ASSERT(isInit(m_Confidence));
      return m_Confidence;
   }
   bool    hasGroupMembership() const    {
      test_invariant(); // internal sanity check
      return isInit(m_GroupMembership);
   }
   int    getGroupMembership() const    {
      test_invariant(); // internal sanity check
      PARANOID_ASSERT(isInit(m_GroupMembership));
      return m_GroupMembership;
   }
   ppProtein *   getSubsumingProtein() const           {
      test_invariant(); // internal sanity check
      return m_SubsumingProtein;
   }              
   bool   isMember() const              {
      test_invariant(); // internal sanity check
      return m_Member;
   }
   bool   isDegen() const {
      test_invariant(); // internal sanity check
      return m_Degen;
   }
   const orderedPeptideList& getPeptidesInProt()  const {
      test_invariant(); // internal sanity check
      return m_PeptidesInProt;
   } 

   int getPeptideCount() const {
     test_invariant(); 
     return m_PeptidesInProt.size();
   }
   void addPeptide(ppPeptide *peptide) {
     m_PeptidesInProt.insert(peptide);
   }

   // test methods
   void  outputPeptidesInProt();

   bool  hasPeptide(ppPeptide *peptide) {
      return m_PeptidesInProt.find(peptide) != m_PeptidesInProt.end();
   }

   void test_invariant() const {  // internal sanity check
     PARANOID_ASSERT(m_SubsumingProtein!=this);
   }

private:
                                                        // required in pepXML schema:
   const char *   m_ProteinName; // note we don't alloc //    yes
   std::string    m_Annotation;                         //    yes
   double         m_FPKM;
   double         m_Probability;                        //    (does not appear in pepXML schema)
   double         m_FinalProbability;                   //    (does not appear in pepXML schema)
   double         m_Coverage;                           //    (does not appear in pepXML schema)
   double         m_Confidence;                           //    (does not appear in pepXML schema)
   int            m_GroupMembership;      // group to which this protein belongs
                                          //    has value = -1 until it is assigned to a group
   unsigned int m_ProteinLen; //Length of the protein
   
   double m_ProteinMW; // Mol Weight of the protein
  
   ppProtein *      m_SubsumingProtein;     // protein which subsumes this, if any        
                                                      //    (does not appear in pepXML schema)
   bool           m_Member;               //&& not sure if this is an informative name - may want to choose a better one
                                                      //    (does not appear in pepXML schema)
   bool           m_Degen;                //&& not sure if this is an informative name - may want to choose a better one
                                                      //    (does not appear in pepXML schema)
                                                      //    (does not appear in pepXML schema)
   orderedPeptideList m_PeptidesInProt;       // set: keys are pointers to all peptides that occur in sequence of this protein
                                                      //    (does not appear in pepXML schema)
   degenList      m_DegenList; // useful when name is a concatenation
                                       // will always contain at least the (this) ptr, 
                                       // or if actually a concatenation then a list of pointers

// these attributes in pepXML_v18.xsd are not used by ProteinProphet
//    ( to be populated )

};

//##############################################################################
// function compareProteinNamesAsc
//		input:	ppProtein* - pointer to first protein
//    input:   ppProtein* - pointer to second protein
//		output:	true if first protein is lower alphabetically
//
// supports sorting of an STL vector of proteins by protein name in ascending order
//##############################################################################
bool compareProteinNamesAsc( const ppProtein* first_prot, const ppProtein* second_prot ); 


#ifdef _DEBUG_SORT // alpha sorted may be easier for debug
struct ltprotname
{
  bool operator()(const ppProtein * s1, const ppProtein * s2) const
  {
    return strcmp(s1->getName(),s2->getName())<0;
  }
};
typedef std::set< ppProtein*, ltprotname > ProteinSet;
#else
typedef std::set< ppProtein* > ProteinSet;
#endif



#endif // !defined(AFX_PROTEIN_H__FC6E0D1B_54DA_4861_A764_1014AAFAA45D__INCLUDED_)
