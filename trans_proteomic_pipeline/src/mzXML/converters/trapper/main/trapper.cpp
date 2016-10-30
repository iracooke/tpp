// -*- mode: c++ -*-

/*
	Program: trapper
	Description: converts Aglient MassHunter / SpectrumMill native 
	mass spectroscopy data into open XML formats (current mzXML 
	and beta mzML).  Please note, this program requires the
	MassHunter libraries from Agilent.


	license purpose: apache-licensed main project, in order to insulate LGPL code from directly calling into propretrary 
	Agilent code.  The author's intention is to remove any possiblity of implication of reciprocal open-source code obligations
	from Agilent.  More specifically, we want to use the MHDAC system without any implied requirement that the MHDAC be open-
	source or redistributable in any way.  This arraignment was jointly determined and agreed upon between the author of 
	this code and Agilent.

	Copyright 2008 N. Tasman (ntasman (a t) systemsbiology (d o t) org)

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.	
*/


#include "mzXML/converters/trapper/MHDACInterface/MHConverter.h"
#include "mzXML/converters/trapper/MHDACWrapper/MHDACWrapper.h"

using namespace std;


int main(int argc, char* argv[]) {
	MHConverter converter;
	MHDACWrapper mhdacWrapper;
	converter.run(argc, argv, &mhdacWrapper);
}
