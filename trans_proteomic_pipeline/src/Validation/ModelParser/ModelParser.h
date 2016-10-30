#ifndef MODEL_PARSER_H
#define MODEL_PARSER_H

/*
Program       : ModelParser
Author        : J.Eng and Andrew Keller <akeller@systemsbiology.org>, Robert Hubley, and
                open source code
Date          : 11.27.02

Primary data object holding all mixture distributions for each precursor ion charge

Copyright (C) 2003 Andrew Keller

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

#define szVERSION            "PLOTMODEL"
#define szAUTHOR             "by J.Eng A.Keller  Institute for Systems Biology, 2002. All rights reserved."
//#define SIZE_FILE             256
#define SIZE_BUFF              4096

/*unused alternatives for True, true, 1, and False, false, 0...
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
*/

#define MAX_SPEC_LEN 50

#define SENS_IMG_W 11 * 640 / 20 //352
#define SENS_IMG_H 3 * 480 / 5 //288

#define IMG_W 211
#define IMG_H 196

#define NUM_ERRORS 3

// environment setting
//#ifdef __CYGWIN__
//#define WINDOWS_CYGWIN
//#endif

struct sens_err
{
	double minprob;
	double sens;
	double err;
};

struct distr_pt
{
	double fval;
	int obs[MAX_CHARGE];
	double modelpos[MAX_CHARGE];
	double modelneg[MAX_CHARGE];
};

//stores all the data that is found in the roc_error_data XML tag
struct roc_error_data
{
	bool hasData;
	bool hasModel;
	double charge_est_correct;
	double errorMPT[NUM_ERRORS];
	int errorENC[NUM_ERRORS];
	Array<sens_err*>* sensitivity;
};

/*unused struct - not sure of purpose
typedef struct
{
	int processed; // Boolean whether obtained all info from .esi file
	double fval; // discr score
	int charge; // precursor ion charge
	int nmc; // number of missed cleavages
	int ntt; // number of tolerable termini (enz. cleavage)
	double massd; // massd
	char peptide[200];  // sequence
	char error[SIZE_BUFF]; // record error messages here
	//more here in future....
} datastrct;
*/

class ModelParser : public Parser
{
public:
	ModelParser(const char* xmlfile, const char* timestamp, const char* spectrum, const char* scores, char* prob);
	~ModelParser();
	void setFilter(Tag* tag);
	int getNumDatabases();
	char* getDatabases();

protected:
	void parse(const char* xmlfile);
	Boolean setFvalue(char* scores);
	char* format(char* spec);

	void removeOldFiles(char* filePath);
	char* setFilePaths(char* filePath);
	
	char* plotSensitivity(char* filePaths, int charge, long random, char sensColor, char errColor);
	char* plotModel(char* filePath, int charge, bool zoom, double maxpos, int range, long random,
		char colorBlack, char colorPink, char colorBlue, char colorRed);
	//char* formatFiles(char* replace);
	void writeModelResults(ostream& os, char* options, Array<sens_err*>* sensitivity, roc_error_data* chargeData);
	void writeSensitivityTable(ostream& os, Array<sens_err*>* sensitivity);
	void writeChargeModelsTab(ostream& os, int charge, roc_error_data* chargeData);
	void writeMixtureModelResults(ostream& os, Array<Tag*>* tags);
	void writeDistributionResults(ostream& os, Boolean pos, int start_ind, int end_ind, Array<Tag*>* tags);

	char* timestamp_;
	char* spectrum_;
	char* scores_;
	double fvalue_;
	Boolean display_fval_;
	char* pngSensGraphAll_;
	char* pngModel_[MAX_CHARGE];
	char* pngModelZoom_[MAX_CHARGE];
	char* pngSensGraph_[MAX_CHARGE];
	int charge_;
	char* prob_;
	Array<char*>* inputfiles_;
	Array<Array<Tag*>*>* model_tags_;
	double est_tot_num_correct_;
	double* errors_;
	double* min_prob_threshes_;
	int* est_num_corrects_;
	Boolean adjusted_;
	char* prophet_name_;
};

#endif
