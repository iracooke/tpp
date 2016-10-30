// -*- mode: c++ -*-

/*
	Program: MHConverter
	Description: converts Aglient MassHunter / SpectrumMill native 
	mass spectroscopy data into open XML formats (current mzXML 
	and beta mzML).  Please note, this program requires the
	MassHunter libraries from Agilent.

	Date: July 2007
	Author: Natalie Tasman, ISB Seattle, 2007

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


#include <string>
#include "mzXML/converters/trapper/MHDACWrapper/MHDACWrapper.h"


class MHConverter {
public:
	int run(int argc, char* argv[], MHDACWrapper* wrapper);
private:
	void usage(const std::string& exename, const std::string& version);	
};
