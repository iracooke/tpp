#ifndef IMODEL_PARSER_H
#define IMODEL_PARSER_H

/*
Program:	iModelParser
Author:		Richard Stauffer, modified from ModelParser: J.Eng and Andrew Keller <akeller@systemsbiology.org>, Robert Hubley, and open source code
Date:		Summer 2010

Primary data object holding all mixture distributions for each precursor ion charge

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

Andrew Keller
Institute for Systems Biology
401 Terry Avenue North
Seattle, WA  98109  USA
akeller@systemsbiology.org
*/

#include "common/constants.h"
#include "Parsers/Parser/Parser.h"
#include "Parsers/Parser/TagFilter.h"
#include "Validation/PeptideProphet/PeptideProphetParser/PeptideProphetParser.h"

//definitions
#define szVERSION "PLOTMODEL"
#define szAUTHOR "by J.Eng A.Keller, Institute for Systems Biology, 2002. All rights reserved."

//string length maximum constants
#define TEXT_SIZE1 64
#define	TEXT_SIZE2 512
#define TEXT_SIZE3 4096

#define N_SCORE_SIZE 64

//definitions for image size constants, gnuplot default image size is 640 by 480
#define SENS_IMG_W  11 * 640 / 20
#define SENS_IMG_H	3 * 480 / 5

#define IMG_W 3 * 640 / 4
#define IMG_H 3 * 480 / 4

#define MODEL_IMGS 8
#define PERFORM_IMGS 6

#define ERROR_NUM 3

enum N_Score
{
	NSS,
	NSE,
	NSI,
	NSM,
	NRS,
	NSP,
	FPKM,
	LENGTH,
	NONE
};

struct sensTableDataPoint
{
	double minprob, sens, err;
};

struct nScoreDataPoint
{
	double nScore, pos, neg, ln, posObs, negObs;
};

struct nScoreDataSet
{
	double scoreVal;
	Array<nScoreDataPoint*>* dataPoints;

	nScoreDataSet()
	{
		scoreVal = 0;
		dataPoints = new Array<nScoreDataPoint*>;
	}

	//need destructor?
};

class iModelParser : public Parser
{
public:
	//functions

	iModelParser(const char* xmlfile, const char* timestamp, const char* spectrum, const char* scores, char* prob);
	~iModelParser();

protected:
	//functions

	void parse(const char* xmlfile);
	char* setFilePaths(char* filePath);
	void removeOldFiles(char* filePath);

	void writeModelResults(ostream& os, char* options, Array<sensTableDataPoint*>* sensitivity);
	void writeSensitivityTable(ostream& os, Array<sensTableDataPoint*>* sensitivity);
	char* format(char* spec);

	char* plotSensitivity(char* filePath, long random, char sensColor, char errColor);

	char* plotModel(char* filePath, char* scoreName, char* scoreTitle, double scoreVal, long random,
	char posColor, char negColor, char lnColor, char scoreColor);

	//variables

	Boolean adjusted_, drawPobs_, drawNobs_;

	Array<char*>* inputfiles_;

	char* timestamp_;
	char* spectrum_;
	char* scores_;
	char* prob_;
	const char* prophet_name_;

	//paths of image files
	char* pngsensgraph_; //sensitivity graph
	char* pngmodels_[MODEL_IMGS]; //images for models tab
	char* pngperforms_[PERFORM_IMGS]; //images for performance tab

	double est_tot_num_correct_psm_;
	double est_tot_num_correct_pep_;

	//error table
	double errors_[ERROR_NUM];
	double min_prob_threshes_[ERROR_NUM];
	int est_num_corrects_[ERROR_NUM];
};

#endif
