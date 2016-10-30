// -*- mode: c++ -*-


/*
    File: analyst15tlbs.h
    Description: Analyst typelibs information via #import directives
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
#import "..\\libs\\analyst-1.5\\MSMethodSvr.dll" rename_namespace("Analyst15") exclude("IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG","IPersistStorage","IPersist","ISupportErrorInfo")
#else
#include "wiff15lib\\msmethodsvr.tlh"
//#include "wiff15lib\\msmethodsvr.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analyst-1.5\\ExploreDataObjects.dll" rename_namespace("Analyst15") exclude("ISequentialStream")
#else
#include "wiff15lib\\ExploreDataObjects.tlh"
//#include "wiff15lib\\ExploreDataObjects.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analyst-1.5\\WIFFTransSvr.dll" rename_namespace("Analyst15") exclude("IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG")
#else
#include "wiff15lib\\WIFFTransSvr.tlh"
//#include "wiff15lib\\WIFFTransSvr.tli"
#endif

// CParamSettings.h
//#import "..\\libs\\analyst-1.5\\ParamSettings.dll" rename_namespace("Analyst15") exclude("IPersistStorage","IPersist","IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG")

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analyst-1.5\\Peak_Finder2.dll" rename_namespace("Analyst15") 
#else
#include "wiff15lib\\Peak_Finder2.tlh"
//#include "wiff15lib\\Peak_Finder2.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analyst-1.5\\PeakFinderFactory.dll" rename_namespace("Analyst15") 
#else
#include "wiff15lib\\PeakFinderFactory.tlh"
//#include "wiff15lib\\PeakFinderFactory.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analyst-1.5\\FileManager.dll" rename_namespace("Analyst15") exclude("IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG")
#else
#include "wiff15lib\\FileManager.tlh"
//#include "wiff15lib\\FileManager.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analyst-1.5\\AcqMethodSvr.dll" rename_namespace("Analyst15") exclude("tagACQMETHODSTGTYPE", "IPersistWIFFStg", "IEnableTuneMode","IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG","IPersistStorage","IPersist","ISupportErrorInfo")
#else
#include "wiff15lib\\AcqMethodSvr.tlh"
//#include "wiff15lib\\AcqMethodSvr.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analyst-1.5\\ParameterSvr.dll" rename_namespace("Analyst15") exclude("IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG","IPersistStorage","IPersist","ISupportErrorInfo","tagACQMETHODSTGTYPE","IPersistWIFFStg","IMiscDAMCalls")
#else
#include "wiff15lib\\ParameterSvr.tlh"
//#include "wiff15lib\\ParameterSvr.tli"
#endif
