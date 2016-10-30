/*
Program       : Agilent2mzXML.cpp for Agilent2mzXML
Author        : David Shteynberg <dshteynb@systemsbiology.org> 
Date          : 01.30.06 
                                                                       
Copyright (C) 2006 David Shteynberg

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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

David Shteynberg
Institute for Systems Biology
401 Terry Avenue North 
Seattle, WA  98109  USA
akeller@systemsbiology.org

Institute for Systems Biology, hereby disclaims all copyright interest 
in Agilent2mzXML written by David Shteynberg

*/
#include "stdafx.h"
// This is the main project file for VC++ application project 
// generated using an Application Wizard.
#include "xml_out.h"
#include <comdef.h>
#include <iostream>
#include <conio.h>
#import "C:/WINDOWS/SYSTEM32/TrapDataAccess.dll" rename_namespace("ADAL")


#define RETURN_ON_BAD_HR(x) \
	if(!SUCCEEDED(x)) return 0;

using namespace std;

int main(int argc, char* argv[])
{

	//Paths must be specified in C++ code with double slashes.
	//  A sample assignment follows::
	//bstrDataFilePath1 = "..\\..\\TestData\\Test_1.d\\";

	_bstr_t bstrDataFilePath;

	//Unless there are 3 args (first being valid file path), quit
	if(argc >= 3) {
	//if(argc >= -1) {
		//bstrDataFilePath = "C:/Documents and Settings/dshteynb/My Documents/Agilent Converter/Sample data/Reto";	
		bstrDataFilePath = argv[1];
		bool bInvalidArgs = false;
		if(!::PathFileExists(bstrDataFilePath + "\\Analysis.yep")) {
			cout<<endl<<"ERROR: \""<<argv[1]<<"\" is NOT a valid path"<<endl;
			cout<<"   OR \""<<bstrDataFilePath + "\\Analysis.yep"<<"\" does not exist"<<endl<<endl;
			bInvalidArgs = true;
		}
		if(bInvalidArgs) {
			cout<<"If using a non-DOS shell, be sure to use double backslashes in your path,"<<endl;
			cout<<"   ie. D:\\\\Data\\\\Test.d"<<endl;
			return 0;
		}
		else{
//			cout<<"\""<<argv[1]<<"\" is a valid path"<<endl;
		}
	}
	else {
		cout<<"Please provide the path to one data file"<<endl<<endl;
		cout<<"Syntax:"<<endl<<"\tAgilent2mzXML.exe <dataFolderPath> <c/p>"<<endl<<endl;
		cout<<"Parameters:"<<endl<<"\t<dataFolderPath> = full path to data folder without \"Analysis.yep\""<<endl<<endl;
		return 0;
	}
	
	HRESULT test = CoInitialize( NULL );	
	RETURN_ON_BAD_HR(test);		// Initializes the COM library on the current thread 
												// and identifies the concurrency model as single-thread 
												// apartment (STA)
	//Wrapping the body of the code in a separate function
	//  is done quite purposefully to limit the scope of the smartpointers.
	//  If smartpointers are not NULLed or do not drop out of scope
	//  before CoUninitialize() is called, there are problems!!!  Limiting
	//  the smartpoitners scopes ensures that they all drop out of scope
	//  before the CoUninitialize()
//	XmlOut* xml_out = new XmlOut(bstrDataFilePath, argv[2]);

	XmlOut* XmlWriter = new XmlOut(bstrDataFilePath, argv[2]);

	//XmlOut* XmlWriter = new XmlOut(bstrDataFilePath, "c");

	int		msControllerType = 0;
	XmlWriter->setController( msControllerType, 1 );

	// Get the total number of scans
	long	firstScan, lastScan;
	firstScan = XmlWriter->getFirstScanNumber();
	lastScan = XmlWriter->getLastScanNumber();
	XmlWriter->writeHeader();
	long	scanNum;
	cout << "Processing scans\n";
	for( scanNum = firstScan ; scanNum <= lastScan ; scanNum++ )
	{
		XmlWriter->writeScan( scanNum );
	}
	XmlWriter->finalizeXml();
	CoUninitialize();

	//getch();
	return 1;
}
