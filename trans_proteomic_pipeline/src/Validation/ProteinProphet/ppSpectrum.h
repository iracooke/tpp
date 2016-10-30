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
// ppSpectrum.h: interface for the ppSpectrum class.
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

#if !defined(AFX_SPECTRUM_H__ECA84F09_56F3_44CA_94D7_925F153C9012__INCLUDED_)
#define AFX_SPECTRUM_H__ECA84F09_56F3_44CA_94D7_925F153C9012__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <assert.h>

const int MAX_PREC_CHARGE = 7;      // maximum precursor charge handled by system ( currently +5 )
const int NUM_NTT_STATES = 3;       // number of NNT states ( currently NTT = 0, 1, 2 )

class ppPeptide; // forward ref

class ppSpectrum  
{

public:
   ppSpectrum() {
      assert(false);                                              // no reason to use default constructor
   }
   ppSpectrum(   const char * newSpectrumName);
	~ppSpectrum();                                                   // destructor not virtual for now

   //set methods
   void setAssumedCharge(  int newAssumedCharge )                 {
      m_AssumedCharge = newAssumedCharge;
   }
   void setIndex(          int newIndex )                         {
      m_Index = newIndex;
   }
   void setSpecPeps( ppPeptide * newPeptide, int arrayIndex )         {
      m_SpecPeps[ arrayIndex ] = newPeptide;
   }
   void setSpecProbs(      double newProbability, int arrayIndex1, int arrayIndex2 )   {
      m_SpecProbs[ arrayIndex1 ][ arrayIndex2 ] = newProbability;
   }
   void setSinglySpecPeps( ppPeptide * newPeptide )    {
      m_SinglySpecPeps = newPeptide;
   }
   void setSinglySpecProbs(   double newProbability, int arrayIndex )                  {
      m_SinglySpecProbs[ arrayIndex ] = newProbability;
   }

   // get methods
   const char * getSpectrumName() const    {
      return m_SpectrumName;
   }
   int                  getAssumedCharge() const   {
      return m_AssumedCharge;
   }
   int                  getIndex() const           {
      return m_Index;
   }
   ppPeptide *   getSpecPeps( int arrayIndex ) const                      {
      return m_SpecPeps[ arrayIndex ];
   }
   double               getSpecProbs( int arrayIndex1, int arrayIndex2 ) const   {
      return m_SpecProbs[ arrayIndex1 ][ arrayIndex2 ];
   }
   ppPeptide *   getSinglySpecPeps() const  {
      return m_SinglySpecPeps;
   }
   double               getSinglySpecProbs( int arrayIndex ) const               {
      return m_SinglySpecProbs[ arrayIndex ];
   }

private:
                                                                           // required in pepXML schema:
   const char *   m_SpectrumName;                                             //    yes
   int            m_AssumedCharge;                                            //    yes
   int            m_Index;                                                    //    yes
   ppPeptide *    m_SpecPeps[ MAX_PREC_CHARGE - 1 ];                          //    (does not appear in pepXML schema)
   double         m_SpecProbs[ MAX_PREC_CHARGE - 1 ][ NUM_NTT_STATES ];       //    (does not appear in pepXML schema)
   ppPeptide *    m_SinglySpecPeps;                                           //    (does not appear in pepXML schema)
   double         m_SinglySpecProbs[ NUM_NTT_STATES ];                        //    (does not appear in pepXML schema)

// these attributes in pepXML_v18.xsd are not used by ProteinProphet
//   int            m_SearchID                                                //    no
//   int            m_StartScan;                                              //    yes
//   int            m_EndScan;                                                //    yes
//   double         m_PrecursorNeutralMass;                                   //    yes
//   std::string    m_SearchSpecification;                                    //    no

};

#endif // !defined(AFX_SPECTRUM_H__ECA84F09_56F3_44CA_94D7_925F153C9012__INCLUDED_)
