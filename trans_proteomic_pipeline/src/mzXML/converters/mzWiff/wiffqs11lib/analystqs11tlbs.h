// -*- mode: c++ -*-


/*
    File: analysttlbs.h
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
#import "..\\libs\\analystqs-1.1\\MSMethodSvr.dll" rename_namespace("AnalystQS11") exclude("IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG","IPersistStorage","IPersist","ISupportErrorInfo")
#else
#include "wiffqs11lib\\msmethodsvr.tlh"
//#include "wiffqs11lib\\msmethodsvr.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analystqs-1.1\\ExploreDataObjects.dll" rename_namespace("AnalystQS11") exclude("ISequentialStream")
#else
#include "wiffqs11lib\\ExploreDataObjects.tlh"
//#include "wiffqs11lib\\ExploreDataObjects.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analystqs-1.1\\WIFFTransSvr.dll" rename_namespace("AnalystQS11") exclude("IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG")
#else
#include "wiffqs11lib\\WIFFTransSvr.tlh"
//#include "wiffqs11lib\\WIFFTransSvr.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analystqs-1.1\\Peak_Finder2.dll" rename_namespace("AnalystQS11") 
#else
#include "wiffqs11lib\\Peak_Finder2.tlh"
//#include "wiffqs11lib\\Peak_Finder2.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analystqs-1.1\\PeakFinderFactory.dll" rename_namespace("AnalystQS11") 
#else
#include "wiffqs11lib\\PeakFinderFactory.tlh"
//#include "wiffqs11lib\\PeakFinderFactory.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analystqs-1.1\\FileManager.dll" rename_namespace("AnalystQS11") exclude("IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG")
#else
#include "wiffqs11lib\\FileManager.tlh"
//#include "wiffqs11lib\\FileManager.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analystqs-1.1\\AcqMethodSvr.dll" rename_namespace("AnalystQS11") exclude("tagACQMETHODSTGTYPE", "IPersistWIFFStg", "IEnableTuneMode","IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG","IPersistStorage","IPersist","ISupportErrorInfo")
#else
#include "wiffqs11lib\\AcqMethodSvr.tlh"
//#include "wiffqs11lib\\AcqMethodSvr.tli"
#endif

#ifdef _IMPORT_PROCESSING_
#import "..\\libs\\analystqs-1.1\\ParameterSvr.dll" rename_namespace("AnalystQS11") exclude("IStorage","IStream","ISequentialStream","_LARGE_INTEGER","_ULARGE_INTEGER","tagSTATSTG","_FILETIME","wireSNB","tagRemSNB","IEnumSTATSTG","IPersistStorage","IPersist","ISupportErrorInfo","tagACQMETHODSTGTYPE","IPersistWIFFStg","IMiscDAMCalls")
#else
#include "wiffqs11lib\\ParameterSvr.tlh"
//#include "wiffqs11lib\\ParameterSvr.tli"
#endif
