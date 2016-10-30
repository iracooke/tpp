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
// ppSpectrum.cpp: implementation of the ppSpectrum class.
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

#include "ppSpectrum.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*
ppSpectrum::ppSpectrum()                                        // no default constructor for now
{

}
*/

ppSpectrum::ppSpectrum(  const char * newSpectrumName) :
   m_SpectrumName( newSpectrumName )
{
      
   m_AssumedCharge = -1;
   m_Index = -1;
   for ( int i = 0; i < MAX_PREC_CHARGE - 1; i++ )
   {
      m_SpecPeps[ i ] = NULL;
   }
   m_SinglySpecPeps = NULL;
   for ( int j = NUM_NTT_STATES; j--; )
   {
      for ( int i = MAX_PREC_CHARGE - 1; i--; )
      {
         m_SpecProbs[ i ][ j ] = -1.0;
      }
      m_SinglySpecProbs[ j ] = -1.0;
   }
}

ppSpectrum::~ppSpectrum()
{

}
