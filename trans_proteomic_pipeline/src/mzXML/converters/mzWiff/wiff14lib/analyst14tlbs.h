// -*- mode: c++ -*-


/*
    File: analyst14tlbs.h
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
#import "..\\libs\\analyst-1.4\\MSMethodSvr.dll" rename_namespace("Analyst14") exclude("IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG","IPersistStorage","IPersist","ISupportErrorInfo")
#else
#include "wiff14lib\\msmethodsvr.tlh"
//#include "wiff14lib\\msmethodsvr.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analyst-1.4\\ExploreDataObjects.dll" rename_namespace("Analyst14") exclude("ISequentialStream")
#else
#include "wiff14lib\\ExploreDataObjects.tlh"
//#include "wiff14lib\\ExploreDataObjects.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analyst-1.4\\WIFFTransSvr.dll" rename_namespace("Analyst14") exclude("IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG")
#else
#include "wiff14lib\\WIFFTransSvr.tlh"
//#include "wiff14lib\\WIFFTransSvr.tli"
#endif

// CParamSettings.h
//#import "..\\libs\\analyst-1.4\\ParamSettings.dll" rename_namespace("Analyst14") exclude("IPersistStorage","IPersist","IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG")

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analyst-1.4\\Peak_Finder2.dll" rename_namespace("Analyst14") 
#else
#include "wiff14lib\\Peak_Finder2.tlh"
//#include "wiff14lib\\Peak_Finder2.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analyst-1.4\\PeakFinderFactory.dll" rename_namespace("Analyst14") 
#else
#include "wiff14lib\\PeakFinderFactory.tlh"
//#include "wiff14lib\\PeakFinderFactory.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analyst-1.4\\FileManager.dll" rename_namespace("Analyst14") exclude("IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG")
#else
#include "wiff14lib\\FileManager.tlh"
//#include "wiff14lib\\FileManager.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analyst-1.4\\AcqMethodSvr.dll" rename_namespace("Analyst14") exclude("tagACQMETHODSTGTYPE", "IPersistWIFFStg", "IEnableTuneMode","IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG","IPersistStorage","IPersist","ISupportErrorInfo")
#else
#include "wiff14lib\\AcqMethodSvr.tlh"
//#include "wiff14lib\\AcqMethodSvr.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analyst-1.4\\ParameterSvr.dll" rename_namespace("Analyst14") exclude("IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG","IPersistStorage","IPersist","ISupportErrorInfo","tagACQMETHODSTGTYPE","IPersistWIFFStg","IMiscDAMCalls")
#else
#include "wiff14lib\\ParameterSvr.tlh"
//#include "wiff14lib\\ParameterSvr.tli"
#endif
