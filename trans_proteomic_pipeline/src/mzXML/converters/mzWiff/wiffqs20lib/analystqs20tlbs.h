// -*- mode: c++ -*-


/*
    File: analyst20tlbs.h
    Description: AnalystQS typelibs information via #import directives
                 macro _IMPORT_PROCESSING_ is meant for generating 
                 .tlh and .tli to be included in mzWiff
    Date: July 31, 2007

    Copyright (C) 2007 Chee Hong WONG, Bioinformatics Institute


    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/


#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analystqs-2.0\\MSMethodSvr.dll" rename_namespace("AnalystQS20") exclude("IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG","IPersistStorage","IPersist","ISupportErrorInfo")
#else
#include "wiffqs20lib\\msmethodsvr.tlh"
//#include "wiffqs20lib\\msmethodsvr.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analystqs-2.0\\ExploreDataObjects.dll" rename_namespace("AnalystQS20") exclude("ISequentialStream")
#else
#include "wiffqs20lib\\ExploreDataObjects.tlh"
//#include "wiffqs20lib\\ExploreDataObjects.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analystqs-2.0\\WIFFTransSvr.dll" rename_namespace("AnalystQS20") exclude("IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG")
#else
#include "wiffqs20lib\\WIFFTransSvr.tlh"
//#include "wiffqs20lib\\WIFFTransSvr.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analystqs-2.0\\Peak_Finder2.dll" rename_namespace("AnalystQS20") 
#else
#include "wiffqs20lib\\Peak_Finder2.tlh"
//#include "wiffqs20lib\\Peak_Finder2.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analystqs-2.0\\XYMathUtils.dll" rename_namespace("AnalystQS20") 
#else
#include "wiffqs20lib\\XYMathUtils.tlh"
//#include "wiffqs20lib\\XYMathUtils.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analystqs-2.0\\PeakFinderFactory.dll" rename_namespace("AnalystQS20") 
#else
#include "wiffqs20lib\\PeakFinderFactory.tlh"
//#include "wiffqs20lib\\PeakFinderFactory.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analystqs-2.0\\FileManager.dll" rename_namespace("AnalystQS20") exclude("IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG")
#else
#include "wiffqs20lib\\FileManager.tlh"
//#include "wiffqs20lib\\FileManager.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analystqs-2.0\\AcqMethodSvr.dll" rename_namespace("AnalystQS20") exclude("tagACQMETHODSTGTYPE", "IPersistWIFFStg", "IEnableTuneMode","IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG","IPersistStorage","IPersist","ISupportErrorInfo")
#else
#include "wiffqs20lib\\AcqMethodSvr.tlh"
//#include "wiffqs20lib\\AcqMethodSvr.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analystqs-2.0\\ParameterSvr.dll" rename_namespace("AnalystQS20") exclude("IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG","IPersistStorage","IPersist","ISupportErrorInfo","tagACQMETHODSTGTYPE","IPersistWIFFStg","IMiscDAMCalls")
#else
#include "wiffqs20lib\\ParameterSvr.tlh"
//#include "wiffqs20lib\\ParameterSvr.tli"
#endif

