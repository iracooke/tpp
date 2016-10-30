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
// ppParentProtein.h: interface for the ParentProtein class, which class Peptide uses
// for keeping track of the various proteins of which the peptide is a part.
//.
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

#if !defined(AFX_PARENTPROTEIN_H__A003F5B4_8E62_49E2_BBA4_F7F3FA90E2B2__INCLUDED_)
#define AFX_PARENTPROTEIN_H__A003F5B4_8E62_49E2_BBA4_F7F3FA90E2B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <iostream>

#include <string>
#include <assert.h>
#include <map> // collection class

#include "common/sysdepend.h"

#define UNINIT_VAL -777
template <class T> bool isInit(T val) {
   return UNINIT_VAL != val;
}

#ifdef _DEBUG
#define _DEBUG_PARANOID
#endif

#ifdef _DEBUG_PARANOID
#define PARANOID_ASSERT(exp) assert(exp)
#else
#define PARANOID_ASSERT(exp)
#endif

class ppPeptide; // forward ref
class ppProtein; // forward ref

class ppParentProtein {
public:
   ppParentProtein():  
         MaxProb(UNINIT_VAL),
         OrigMaxProb(UNINIT_VAL),
         NTT(UNINIT_VAL),
         GroupPepWt(UNINIT_VAL),
         PepWt(UNINIT_VAL),
         OrigPepWt(UNINIT_VAL),
         NSPBin(UNINIT_VAL),
         EstNSP(UNINIT_VAL),
         NumInsts(UNINIT_VAL),
         ExpNumInsts(UNINIT_VAL),
         InstsMaxProb(UNINIT_VAL), 
         InstsTotProb(UNINIT_VAL)
      {
      }

   void setMaxProb(double newMaxProb) {
      MaxProb = newMaxProb;
   }
   void setOrigMaxProb(double newOrigMaxProb) {
      OrigMaxProb = newOrigMaxProb;
   }
   void setNTT(int    newNTT) {
      NTT    = newNTT;
   }
   void setPepWt(double newPepWt) {
      PepWt = newPepWt;
      PARANOID_ASSERT(test_invariant());
   }
   void setGroupPepWt(double newGroupPepWt) {
      GroupPepWt = newGroupPepWt;
      PARANOID_ASSERT(test_invariant());
   }
   void setOrigPepWt(double newOrigPepWt) {
      OrigPepWt = newOrigPepWt;
   }
   void setNSPBin(int    newNSPBin) {
      NSPBin    = newNSPBin;
   }

   void setFPKMBin(int    newFPKMBin) {
      FPKMBin    = newFPKMBin;
   }

   void setNIBin(int    newNIBin) {
      NIBin    = newNIBin;
   }
   void setEstNSP(double newEstNSP) {
      EstNSP = newEstNSP;
   }

   void setFPKM(double newFPKM) {
      FPKM = newFPKM;
   }

   void setNumInsts(int    newNumInsts) {
      NumInsts    = newNumInsts;
   }
   void setInstsTotProb(double    newInstsTotProb) {
      InstsTotProb    = newInstsTotProb;
   }
   void setInstsMaxProb(double    newInstsMaxProb) {
      InstsMaxProb    = newInstsMaxProb;
   }
   void setExpNI(double    newExpNumInsts) {
      ExpNumInsts    = newExpNumInsts;
   }
   bool hasMaxProb() const {
      return isInit(MaxProb);
   }
   double getMaxProb() const {
      PARANOID_ASSERT(isInit(MaxProb));
      return MaxProb;
   }
   double getInstsTotProb() const {
      PARANOID_ASSERT(isInit(InstsTotProb));
      return InstsTotProb;
   }
   bool hasOrigMaxProb() const {
      return isInit(OrigMaxProb);
   }
   bool hasInstsTotProb() const {
      return isInit(InstsTotProb);
   }
   double getOrigMaxProb() const {
      PARANOID_ASSERT(isInit(OrigMaxProb));
      return OrigMaxProb;
   }
   double getInstsMaxProb() const {
      PARANOID_ASSERT(isInit(InstsMaxProb));
      return InstsMaxProb;
   }
   bool    hasNTT() const {
      return isInit(NTT);
   }
   int    getNTT() const {
	   return isInit(NTT)?NTT:2; // default to NTT=2
   }
   bool hasPepWt() const {
      return isInit(PepWt);
   }
   double getPepWt() const {
      PARANOID_ASSERT(isInit(PepWt));
      return PepWt;
   }
   bool hasGroupPepWt() const {
      return isInit(GroupPepWt);
   }
   double getGroupPepWt() const {
      PARANOID_ASSERT(isInit(GroupPepWt));
      return GroupPepWt;
   }
   double getOrigPepWt() const {
      PARANOID_ASSERT(isInit(OrigPepWt));
      return OrigPepWt;
   }
   int    getNSPBin() const {
      PARANOID_ASSERT(isInit(NSPBin));
      return NSPBin   ;
   }
   int    getNIBin() const {
      PARANOID_ASSERT(isInit(NIBin));
      return NIBin   ;
   }
   double getEstNSP() const {
      PARANOID_ASSERT(isInit(EstNSP));
      return EstNSP;
   }
   int    getFPKMBin() const {
      PARANOID_ASSERT(isInit(FPKMBin));
      return FPKMBin   ;
   }
   double getFPKM() const {
      PARANOID_ASSERT(isInit(FPKM));
      return FPKM;
   }

   bool    hasNumInsts() const {
      return isInit(NumInsts);
   }
   int    getNumInsts() const {
      PARANOID_ASSERT(isInit(NumInsts));
      return NumInsts   ;
   }
   double    getExpNI() const {
      PARANOID_ASSERT(isInit(ExpNumInsts));
      return ExpNumInsts   ;
   }
#ifdef _DEBUG
   void setDebugInfo(const ppPeptide *pep,const ppProtein *prot); // for debug convenience
   bool test_invariant();
   const char *peptideName; // for debug convenience
   const char *proteinName; // for debug convenience
   const ppProtein *protein; // for debug convenience
#endif
private:
  double MaxProb;
  double OrigMaxProb;
  double InstsMaxProb;
  int    NTT;
  double GroupPepWt;
  double PepWt;
  double OrigPepWt;
  double FPKM;
  int FPKMBin;

  int    NSPBin;
  int    NIBin;
  double EstNSP;
  int    NumInsts;
  double InstsTotProb;
  double ExpNumInsts;
};

#endif // AFX_PARENTPROTEIN_H__A003F5B4_8E62_49E2_BBA4_F7F3FA90E2B2__INCLUDED_
