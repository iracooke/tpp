// -*- mode: c++ -*-


/*
    File: ConverterArgs.h
    Description: common program argument parsing for MS data converters.
    Date: July 25, 2007

    Copyright (C) 2007 Natalie Tasman, ISB Seattle


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

#pragma once

#include <string>


class ConverterArgs {
public:
	bool centroidScans;
	bool compressScans;
	bool verbose;
	bool mzMLMode;
	bool mzXMLMode;
	std::string inputFileName;
	std::string outputFileName;
	bool gzipOutputFile; // gzip the output file? (independent of peak compression)

	// new 2/20/08
	bool skipChecksum;

	bool deisotope_;
	bool requirePeptideLikeAbundanceProfile_;
	double relativeTolerance_;
	double absoluteTolerance_;
	bool limitChargeState_;
	int maxChargeState_;
	

	ConverterArgs();
	void printArgs(void);
	bool parseArgs(int argc, char* argv[]);
};
