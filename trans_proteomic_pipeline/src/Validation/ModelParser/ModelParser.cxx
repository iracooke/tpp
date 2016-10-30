/*
Program       : ModelParser
Author        : J.Eng and Andrew Keller <akeller@systemsbiology.org>, Robert Hubley, and
                open source code
Date          : 11.27.02
Revision      : $Id: ModelParser.cxx 6590 2014-08-22 20:21:52Z slagelwa $

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

#include <string.h>
#include <stdio.h>
#include <fstream>
#include "common/TPPVersion.h" // contains version number, name, revision
#include "common/sysdepend.h"
#include "common/util.h"

#include "ModelParser.h"

ModelParser::ModelParser(const char* xmlfile, const char* timestamp, const char* spectrum, const char* scores, char* prob) : Parser(NULL)
{
	timestamp_ = NULL;

	if(timestamp != NULL)
	{
		timestamp_ = new char[strlen(timestamp)+1];
		strcpy(timestamp_, timestamp);
	}

	spectrum_ = new char[strlen(spectrum)+1];
	strcpy(spectrum_, spectrum);

	scores_ = new char[strlen(scores)+1];
	strcpy(scores_, scores);
	// change : to =
	for(int k = 0; scores_[k]; k++)
		if(scores_[k] == ':')
			scores_[k] = '=';

	inputfiles_ = new Array<char*>;

	display_fval_ = setFvalue(scores_);
	charge_ = -1;

	if(strlen(spectrum_) > 2 && spectrum_[strlen(spectrum_) - 2] == '.')
		charge_ = spectrum_[strlen(spectrum_) - 1] - 48;

	adjusted_ = False;

	if(strlen(prob) > 0 && prob[strlen(prob)-1] == 'a')
	{
		adjusted_ = True;
		prob[strlen(prob) - 1] = 0;
	}
	prob_ = new char[strlen(prob) + 1];
	strcpy(prob_, prob);

	model_tags_ = new Array<Array<Tag*>*>;

	est_tot_num_correct_ = 0.0;

	errors_ = new double[NUM_ERRORS];
	errors_[0] = 0.0;
	errors_[1] = 0.025;
	errors_[2] = 0.05;
	min_prob_threshes_ = new double[NUM_ERRORS];
	min_prob_threshes_[0] = 0.0;
	min_prob_threshes_[1] = 0.0;
	min_prob_threshes_[2] = 0.0;

	est_num_corrects_ = new int[NUM_ERRORS];
	est_num_corrects_[0] = 0;
	est_num_corrects_[1] = 0;
	est_num_corrects_[2] = 0;

	prophet_name_ = NULL;
	PeptideProphetParser* prophet = new PeptideProphetParser();
	if(prophet != NULL)
	{
		const char* prophet_name = prophet->getName();
		if(prophet_name != NULL)
		{
			prophet_name_ = new char[strlen(prophet_name)+1];
			strcpy(prophet_name_, prophet_name);
		}
		delete prophet;
	}
	else
	{
		cout << "error: no name for peptideprophet" << std::endl;
		exit(1);
	}

	char *xf=strdup(xmlfile);
	fixPath(xf,1); //tidy up path seperators etc - expect existence

	init(xf);
	free(xf);
}

ModelParser::~ModelParser()
{
	delete[] prophet_name_;
	delete[] timestamp_;
	delete[] spectrum_;
	delete[] scores_;
	delete[] pngSensGraphAll_;

	for(char i = 0; i != MAX_CHARGE; i++)
	{
		delete[] pngModel_[i];
		delete[] pngModelZoom_[i];
		delete[] pngSensGraph_[i];
	}

	delete[] prob_;
	delete[] errors_;
	delete[] min_prob_threshes_;
	delete[] est_num_corrects_;

	if(model_tags_ != NULL)
	{
		for(int k = 0; k < model_tags_->length(); k++)
			if((*model_tags_)[k] != NULL)
			{
				for(int j = 0; j < (*model_tags_)[k]->length(); j++)
					if((*(*model_tags_)[k])[j] != NULL)
						delete (*(*model_tags_)[k])[j];
			} //if not null
		delete model_tags_;
	}

	if(inputfiles_ != NULL)
	{
		for(int k = 0; k < inputfiles_->length(); k++)
			if((*inputfiles_)[k] != NULL)
				delete (*inputfiles_)[k];
					delete inputfiles_;
	}
}

void ModelParser::parse(const char* xmlfile)
{
	Boolean debug = False; //True;

	int range[] = {0, 0, 0, 0, 0, 0, 0};

	Tag* tag = NULL;
	char* data = NULL;

	Boolean analyze = False;

	Array<sens_err*>* sensitivity = new Array<sens_err*>; // collect sens data
	sens_err* next_sens = NULL;

	roc_error_data chargeModelData[MAX_CHARGE]; //holds roc and error info for charge specific models tab

	for(char i = 0; i != MAX_CHARGE; i++)
	{
		chargeModelData[i].hasData = false; //this will change later on if data is found
		chargeModelData[i].hasModel = false; //this will change later on if model is found
		chargeModelData[i].sensitivity = new Array<sens_err*>;
	}

	Array<distr_pt*>* distributions = new Array<distr_pt*>;
	distr_pt* next_distr = NULL;

	// add this check since pwiz::util::random_access_compressed_ifstream does not seem to throw a useful error (or even return!) when file is not present
	if (!std::ifstream(xmlfile)) {
	  cout << "ModelParser: error cannot open " << xmlfile << endl;
	}

	pwiz::util::random_access_compressed_ifstream fin(xmlfile); // can read gzipped files
	if(! fin)
	{
		cerr << "ModelParser: error opening " << xmlfile << endl;
		exit(1);
	}

	Boolean done = False;
	Boolean collect = False;
	Boolean collectRocErr = False;
	int chargeRocErr = 0;

	int error_index = 0;
	char options[1000];
	options[0] = 0;

	char *nextline = new char[line_width_];

	while(! done && fin.getline(nextline, line_width_))
	{
		data = strstr(nextline, "<");

		while(data != NULL)
		{
			tag = new Tag(data);
			setFilter(tag);

			if(tag != NULL)
			{
				if(filter_)
				analyze = True;

				if(tag->isStart() && ! strcmp(tag->getName(), "peptideprophet_summary"))
				{
					est_tot_num_correct_ = atof(tag->getAttributeValue("est_tot_num_correct"));
					strcpy(options, tag->getAttributeValue("options"));
				}

				if(analyze && tag->isEnd() && ! strcmp(tag->getName(), "peptideprophet_summary"))
				{
					analyze = False; // done
					done = True;
					break; // no more need to read summary
				}

				if(analyze) //grab desired data
				{
					if(tag->isStart() && !strcmp(tag->getName(), "inputfile"))
					{
						char* nextfile = new char[strlen(tag->getAttributeValue("name"))+1];
						strcpy(nextfile, tag->getAttributeValue("name"));
						inputfiles_->insertAtEnd(nextfile);
					}
					else if(! strcmp(tag->getName(), "distribution_point"))
					{
						next_distr = new distr_pt;
						next_distr->fval = atof(tag->getAttributeValue("fvalue"));

						char tmp[20];

						for (char i = 0; i != MAX_CHARGE; i++)
						{
							sprintf(tmp, "obs_%d_distr", i + 1);
							if (tag->getAttributeValue(tmp) != NULL)
							{
							        sprintf(tmp, "%s", tag->getAttributeValue(tmp));
								next_distr->obs[i] = atoi(tmp);
								
								sprintf(tmp, "model_%d_pos_distr", i + 1);
								next_distr->modelpos[i] = atof(tag->getAttributeValue(tmp));
								
								if (next_distr->modelpos[i]) //if not zero
									chargeModelData[i].hasModel = true;
								
								sprintf(tmp, "model_%d_neg_distr", i + 1);
								next_distr->modelneg[i] = atof(tag->getAttributeValue(tmp));
								
								if (next_distr->modelneg[i]) //if not zero
									chargeModelData[i].hasModel = true;
							}

							if(next_distr->obs[i] > range[i])
								range[i] = next_distr->obs[i];
						}

						distributions->insertAtEnd(next_distr);
					} //distribution

					if(tag->isStart() && ! strcmp(tag->getName(), "mixture_model"))
					{
						Array<Tag*>* next = new Array<Tag*>;
						model_tags_->insertAtEnd(next);
						collect = True;
					}
					else if(tag->isEnd() && ! strcmp(tag->getName(), "mixture_model"))
					{
						collect = False;
					}

					if(collect)
						(*model_tags_)[model_tags_->length() - 1]->insertAtEnd(tag);

					//collect roc error data
					if(tag->isStart() && !strcmp(tag->getName(), "roc_error_data"))
					{
						collectRocErr = True;

						if (strcmp(tag->getAttributeValue("charge"), "all"))
						{
							chargeRocErr = atoi(tag->getAttributeValue("charge"));
							chargeModelData[chargeRocErr - 1].hasData = true;
							chargeModelData[chargeRocErr - 1].charge_est_correct = atof(tag->getAttributeValue("charge_est_correct"));
						}
					}
					else if(tag->isEnd() && !strcmp(tag->getName(), "roc_error_data"))
					{
						collectRocErr = False;
					}

					if(collectRocErr)
					{
						if (chargeRocErr) //roc_error_data tag is considering data from a single charge
						{
							//load all data into roc_error_data struct
							if (!strcmp(tag->getName(), "roc_data_point"))
							{
								next_sens = new sens_err;
								next_sens->minprob = atof(tag->getAttributeValue("min_prob"));
								next_sens->sens = atof(tag->getAttributeValue("sensitivity"));
								next_sens->err = atof(tag->getAttributeValue("error"));
								chargeModelData[chargeRocErr - 1].sensitivity->insertAtEnd(next_sens);
							} //roc
							else if(error_index < NUM_ERRORS && tag->isStart() && ! strcmp(tag->getName(), "error_point"))
							{
								double nexterr = atof(tag->getAttributeValue("error"));
								double maxdiff = 0.005;
								if(nexterr == errors_[error_index] ||
								(nexterr > errors_[error_index] && nexterr - errors_[error_index] <= maxdiff) ||
								(nexterr < errors_[error_index] && errors_[error_index] - nexterr <= maxdiff))
								{
									chargeModelData[chargeRocErr - 1].errorMPT[error_index] = atof(tag->getAttributeValue("min_prob"));
									chargeModelData[chargeRocErr - 1].errorENC[error_index] = atoi(tag->getAttributeValue("num_corr"));
									error_index++;
								}
							}
							else
								error_index = 0;
						}
						else //roc_error_data tag is considering all data
						{
							if (!strcmp(tag->getName(), "roc_data_point"))
							{
								next_sens = new sens_err;
								next_sens->minprob = atof(tag->getAttributeValue("min_prob"));
								next_sens->sens = atof(tag->getAttributeValue("sensitivity"));
								next_sens->err = atof(tag->getAttributeValue("error"));
								sensitivity->insertAtEnd(next_sens);
							} //roc
							else if(error_index < NUM_ERRORS && tag->isStart() && ! strcmp(tag->getName(), "error_point"))
							{
								double nexterr = atof(tag->getAttributeValue("error"));
								double maxdiff = 0.005;
								if(nexterr == errors_[error_index] ||
								(nexterr > errors_[error_index] && nexterr - errors_[error_index] <= maxdiff) ||
								(nexterr < errors_[error_index] && errors_[error_index] - nexterr <= maxdiff))
								{
									min_prob_threshes_[error_index] = atof(tag->getAttributeValue("min_prob"));
									est_num_corrects_[error_index] = atoi(tag->getAttributeValue("num_corr"));
									error_index++;
								}
							}
							else
								error_index = 0;
						}
					}
					else //collect roc/error tags as original version did if roc_error_data tag does not surround them
					{
						//this data becomes part of the main sensitivity and error tables at the top of the page
						if (!strcmp(tag->getName(), "roc_data_point"))
						{
							next_sens = new sens_err;
							next_sens->minprob = atof(tag->getAttributeValue("min_prob"));
							next_sens->sens = atof(tag->getAttributeValue("sensitivity"));
							next_sens->err = atof(tag->getAttributeValue("error"));
							sensitivity->insertAtEnd(next_sens);
						} //roc
						else if(error_index < NUM_ERRORS && tag->isStart() && !strcmp(tag->getName(), "error_point"))
						{
							double nexterr = atof(tag->getAttributeValue("error"));
							double maxdiff = 0.005;
							if(nexterr == errors_[error_index] ||
							(nexterr > errors_[error_index] && nexterr - errors_[error_index] <= maxdiff) ||
							(nexterr < errors_[error_index] && errors_[error_index] - nexterr <= maxdiff))
							{
								min_prob_threshes_[error_index] = atof(tag->getAttributeValue("min_prob"));
								est_num_corrects_[error_index] = atoi(tag->getAttributeValue("num_corr"));
								error_index++;
							}
						}
						else
							error_index = 0;
					}
					
				} //if analyze

				if(!collect)
					delete tag;
			} //if not null

			data = strstr(data + 1, "<");
		} // next tag
	} // next line

	tag = NULL;
	delete[] tag;

	fin.close();
	delete[] nextline;

	data = NULL;

	/* bunch of unused variables, not sure what they are for
	int bNewFileType = 0;   //new file type includes 1+
	data = NULL;
	char esifile[SIZE_FILE];
	char suffix[] = ".model.txt";
	char esi_suffix[] = ".htm.esi";
	int abbrev = 0; // whether or not probabilities are single digits (from incomplete analysis)
	int icat = 0;
	char icat_indicator[] = "icat cys";
	int glyc = 0;
	char glyc_indicator[] = "N glyc motif";
	int nmc = 0;
	char nmc_indicator[] = "no. missed";
	*/

	if(timestamp_ != NULL && strcmp(prob_, "-nofigs"))
	{
		char* filePath = new char[SIZE_FILE];
		strcpy(filePath, xmlfile);

		//filePath should be something like: c:/Inetpub/wwwroot/ISB/data/class/iProphet/xtandem/interact.iproph.pep.xml.a15956

		char* performPath = setFilePaths(filePath);
		removeOldFiles(filePath);

		char* szTmpDataFile = new char[SIZE_FILE];
		sprintf(szTmpDataFile, "%s.data", filePath);

		// write data files
		FILE* fpTmp;

		time_t tStartTime = time((time_t *)NULL);;
		double max_pos[MAX_CHARGE];
		double max_obs[MAX_CHARGE];
		srandom((int)(strlen(filePath) + tStartTime + filePath[strlen(filePath) - 3]));
		long rdm = (long)random(); //create random number that becomes part of the file paths

		//these are ok for Linux and Windows/Cygwin (for now)
		char gnuplotBlack = -1;
		char gnuplotRed = 1;
		char gnuplotGreen = 2;
		char gnuplotBlue = 3; //8;
		char gnuplotPink = 4; //5;

		//create data and image files for sensitivity graphs

		//write info to data file (all charges)s
		if((fpTmp = fopen(szTmpDataFile, "w")) == NULL)
			printf("Error - cannot create main sensitivity graph data file %s.", szTmpDataFile);
		else
		{
			for(int k = 0; k < sensitivity->length(); k++)
				fprintf(fpTmp, "%0.2f\t%0.3f\t%0.3f\n", (*sensitivity)[k]->minprob, (*sensitivity)[k]->sens, (*sensitivity)[k]->err);
		}
		fclose(fpTmp);

		//call function to plot sensitivity graph
		pngSensGraphAll_ = plotSensitivity(filePath, 0,  rdm, gnuplotRed, gnuplotGreen);

		//create and plot the sensitivity graph for indvidual charges

		for(char i = 0; i != MAX_CHARGE; i++)
		{
			if(chargeModelData[i].hasData)
			{
				if((fpTmp = fopen(szTmpDataFile, "w")) == NULL)
					printf("Error - cannot create charge sensitivity graph data file %s.", szTmpDataFile);
				else
				{
						for(int k = 0; k < chargeModelData[i].sensitivity->length(); k++)
						{
							fprintf(fpTmp, "%0.2f\t%0.3f\t%0.3f\n",
								(*chargeModelData[i].sensitivity)[k]->minprob, (*chargeModelData[i].sensitivity)[k]->sens, (*chargeModelData[i].sensitivity)[k]->err);
						}
				}
				fclose(fpTmp);

				pngSensGraph_[i] = plotSensitivity(filePath, i + 1, rdm, gnuplotRed, gnuplotGreen);
			}
			else
				pngSensGraph_[i] = NULL;
		}

		//create data and image files for charge models

		for (char i = 0; i != MAX_CHARGE; i++)
		{
			max_pos[i] = 1;
			max_obs[i] = 1;
		}

		if ((fpTmp = fopen(szTmpDataFile, "w")) == NULL)
			printf("Error - cannot create charge model image data file %s.", szTmpDataFile);
		else
		{
			for(int k = 0; k < distributions->length(); k++)
			{
				fprintf(fpTmp, "%0.2f", (*distributions)[k]->fval);
				for(char i = 0; i != MAX_CHARGE; i++)
				{
					fprintf(fpTmp, "\t%d\t%0.3f\t%0.3f", (*distributions)[k]->obs[i], (*distributions)[k]->modelpos[i], (*distributions)[k]->modelneg[i]);

					if (max_pos[i] < (*distributions)[k]->modelpos[i])
						max_pos[i] = (*distributions)[k]->modelpos[i];

					if (max_obs[i] < (*distributions)[k]->obs[i])
						max_obs[i] = (*distributions)[k]->obs[i];
				}
				fprintf(fpTmp, "\n");
			}
		}

		fclose(fpTmp);

		//create png model files for all 7 charges
		for(char i = 0; i != MAX_CHARGE; i++)
		{
			if(chargeModelData[i].hasModel)
			{
				//testing new plot model
				pngModel_[i] = plotModel(filePath, i + 1, false, max_obs[i], range[i], rdm,
					gnuplotBlack, gnuplotPink, gnuplotBlue, gnuplotRed);
				//Now the Zoom
				pngModelZoom_[i] = plotModel(filePath, i + 1, true, max_pos[i], range[i], rdm,
					gnuplotBlack, gnuplotPink, gnuplotBlue, gnuplotRed);
			}
			else
			{
				pngModel_[i] = NULL;
				pngModelZoom_[i] = NULL;
			}
		}

		unlink(szTmpDataFile);

		//note: the image files are deleted as soon as they are viewed via the mechanism used by makeTmpPNGFileSrcRef
	} //if write figs

	writeModelResults(cout, options, sensitivity, chargeModelData);
}

char* ModelParser::plotSensitivity(char* filePath, int charge, long rnd, char sensColor, char errColor)
{
	FILE* fpPlotCode;

	char* pngFilePath = new char[SIZE_FILE];
	char* dataFilePath = new char[SIZE_FILE]; //by this point data file exists and is populated, but it's file path is known
	char* plotFilePath = new char[SIZE_FILE];

	char plotCommand[4096];

	if(charge)
		sprintf(pngFilePath, "%s.%ld.s%d.ISB.png", filePath, rnd, charge);
	else
		sprintf(pngFilePath, "%s.%ld.ISB.png", filePath, rnd);

	sprintf(plotFilePath, "%s.gp", filePath);
	sprintf(dataFilePath, "%s.data", filePath);

	if ((fpPlotCode = fopen(plotFilePath, "w")) == NULL)
		printf("Error - cannot create gnuplot file %s.", plotFilePath);
	else
	{
		//gnuplot code
		fprintf(fpPlotCode, "set terminal png size %d, %d\n", SENS_IMG_W, SENS_IMG_H);
		fprintf(fpPlotCode, "set output \"%s\"\n", pngFilePath);
		fprintf(fpPlotCode, "set border\n");
		fprintf(fpPlotCode, "set title \"Sensitivity & Error Rates\"\n");
		fprintf(fpPlotCode, "set xlabel \"Min Probability Threshhold (MPT) To Accept\"\n");
		fprintf(fpPlotCode, "set ylabel \"Sensitivity & Error\"\n");
		fprintf(fpPlotCode, "set grid\n");
		fprintf(fpPlotCode, "set xrange [0:1]\n");
		fprintf(fpPlotCode, "set yrange [0:1]\n");
		fprintf(fpPlotCode, "set key bottom right\n");
		fprintf(fpPlotCode, "plot \"%s\" using 1:2 title 'sensitivity' with lines lc %d, \\\n", dataFilePath, sensColor);
		fprintf(fpPlotCode, "\"%s\" using 1:3 title 'error' with lines lc %d\n", dataFilePath, errColor);

		fprintf(fpPlotCode, "unset output\n");
		fclose(fpPlotCode);

		char* plotFileName = findRightmostPathSeperator(plotFilePath);
		*plotFileName++ = '\0';

		//write and call plot command
		sprintf(plotCommand, "cd %s; %s %s; rm -f %s", plotFilePath, GNUPLOT_BINARY, plotFileName, plotFileName);
		verified_system(plotCommand); //system() with verbose error check

#ifdef USING_RELATIVE_WEBSERVER_PATH  //fix up the image path if needed
		sprintf(pngFilePath, "%s", translate_relative_webserver_root_path_to_absolute_filesystem_path(pngFilePath).c_str());
#endif
	}

	unlink(plotFilePath);

	delete[] plotFilePath;
	delete[] dataFilePath;

	return pngFilePath;
}

char* ModelParser::plotModel(char* filePath, int charge, bool zoom, double maxpos, int range, long rnd,
	char colorBlack, char colorPink, char colorBlue, char colorRed)
{
	FILE* fpPlotCode;

	char* pngFilePath = new char[SIZE_FILE];
	char* dataFilePath = new char[SIZE_FILE]; //by this point data file exists and is populated, but it's file path is known
	char* plotFilePath = new char[SIZE_FILE];

	char plotCommand[4096];

	int distrData = 3 * charge - 1;
	int posData = distrData + 1;
	int negData = distrData + 2;

	if(zoom)
		sprintf(pngFilePath, "%s.%ld.%dzoom.ISB.png", filePath, rnd, charge);
	else
		sprintf(pngFilePath, "%s.%ld.%d.ISB.png", filePath, rnd, charge);

	sprintf(plotFilePath, "%s.gp", filePath);
	sprintf(dataFilePath, "%s.data", filePath); //the data should already be at this location

	//write gnuplot plotting file
	if((fpPlotCode = fopen(plotFilePath, "w")) == NULL)
	{
		printf("Error - cannot create gnuplot file %s for %d+ charge.", plotFilePath, charge);
	}
	else
	{
		//working with gnuplot 4.4
		fprintf(fpPlotCode, "set terminal png size %d, %d;\n", IMG_W, IMG_H);
		fprintf(fpPlotCode, "set output \"%s\";\n", pngFilePath);
		if (zoom)
			fprintf(fpPlotCode, "set title \"%d+ Zoom\";\n", charge);
		else
			fprintf(fpPlotCode, "set title \"%d+\";\n", charge);
		fprintf(fpPlotCode, "set border;\n");
		fprintf(fpPlotCode, "set xlabel \"Discriminant Score (fval)\";\n");
		fprintf(fpPlotCode, "set ylabel \"# of Spectra\";\n");
		fprintf(fpPlotCode, "set grid;\n");

		if(display_fval_ && charge_ == charge)
		{
			fprintf(fpPlotCode, "set parametric;\n");
			fprintf(fpPlotCode, "set trange [0:%d];\n", range / 2);
			fprintf(fpPlotCode, "const = %0.4f;\n", fvalue_);
		}
		fprintf(fpPlotCode, "set yrange [0:%f];\n", maxpos * 1.2);

		if (zoom)
		{
			fprintf(fpPlotCode, "plot \"%s\" using 1:%d notitle with lines lc %d, \\\n", dataFilePath, distrData, colorBlack);
			fprintf(fpPlotCode, "\"%s\" using 1:%d notitle  with lines lc %d, \\\n", dataFilePath, posData, colorPink);

			if(display_fval_ && charge_ == charge)
			{
				fprintf(fpPlotCode, "\"%s\" using 1:%d notitle with lines lc %d,\\\n", dataFilePath, negData, colorBlue);
				fprintf(fpPlotCode, "const,t notitle with lines lc %d\n", colorRed);
			}
			else
				fprintf(fpPlotCode, "\"%s\" using 1:%d notitle with lines lc %d\n", dataFilePath, negData, colorBlue);
		}
		else
		{
			fprintf(fpPlotCode, "plot \"%s\" using 1:%d notitle with lines lc %d, \\\n", dataFilePath, distrData, colorBlack);
			fprintf(fpPlotCode, "\"%s\" using 1:%d notitle with lines lc %d, \\\n", dataFilePath, posData, colorPink);

			if(display_fval_ && charge_ == charge)
			{
				fprintf(fpPlotCode, "\"%s\" using 1:%d notitle  with lines lc %d,\\\n", dataFilePath, negData, colorBlue);
				fprintf(fpPlotCode, "const, t title '%0.4f' with lines lc %d\n", fvalue_, colorRed);
			}
			else
				fprintf(fpPlotCode, "\"%s\" using 1:%d notitle  with lines lc %d\n", dataFilePath, negData, colorBlue);
		}

		fclose(fpPlotCode);

		char* plotFileName = findRightmostPathSeperator(plotFilePath);
		*plotFileName++ = '\0';

		//write and call plot command
		sprintf(plotCommand, "cd %s; %s %s; rm -f %s", plotFilePath, GNUPLOT_BINARY, plotFileName, plotFileName);
		verified_system(plotCommand); //system() with verbose error check

#ifdef USING_RELATIVE_WEBSERVER_PATH  //fix up the image path if needed
		sprintf(pngFilePath, "%s", translate_relative_webserver_root_path_to_absolute_filesystem_path(pngFilePath).c_str());
#endif
	}

	unlink(plotFilePath);

	delete[] plotFilePath;
	delete[] dataFilePath;

	return pngFilePath;
}

//this correctly sets filePath and returns the path of the performance pngs to be loaded
char* ModelParser::setFilePaths(char* filePath)
{
	const char* tmpRoot;
	char* pngPath = new char[SIZE_FILE];
	char webserverRoot[SIZE_FILE];

	tmpRoot = getWebserverRoot();

	if (tmpRoot == NULL)
	{
		printf("<PRE> Environment variable WEBSERVER_ROOT does not exist.\n\n");
#ifdef WINDOWS_CYGWIN
		printf(" For Windows users, you can set this environment variable\n");
		printf(" through the Advanced tab under System Properties when you\n");
		printf(" right-mouse-click on your My Computer icon.\n\n");

		printf(" Set this environment variable to your webserver's document\n");
		printf(" root directory such as c:\\inetpub\\wwwroot for IIS or\n");
		printf(" c:\\website\\htdocs or WebSite Pro.\n\n");
#endif
		printf(" Exiting.\n");
		exit(0);
	}
	else
	{
		//must first pass to cygpath program
#ifdef WINDOWS_CYGWIN
		char
		szCommand[SIZE_FILE],
		szNewRoot[SIZE_FILE];

		sprintf(szCommand, "cygpath '%s'", tmpRoot);
		FILE* fp;

		if((fp = popen(szCommand, "r")) == NULL)
		{
			printf("cygpath error, exiting\n");
			exit(1);
		}
		else
		{
			fgets(szNewRoot, TEXT_SIZE3, fp);
			pclose(fp);
			szBuf[strlen(szNewRoot) - 1] = 0;
			strcpy(webServerRoot, szNewRoot);
		}
#else
		strcpy(webserverRoot, tmpRoot);
#endif
	}

	//Check if webServerRoot is present
	if (access(webserverRoot, F_OK))
	{
		printf(" Cannot access the webserver's root directory:\n");
		printf("	%s\n", webserverRoot);
		printf(" This was set as the environment variable WEBSERVER_ROOT\n\n");

		printf(" For Windows users, you can check this environment variable\n");
		printf(" through the Advanced tab under System Properties when you\n");
		printf(" right-mouse-click on your My Computer icon.\n\n");
		printf(" Exiting.\n");
		exit(1);
	}

	//set up correct filePath
	replace_path_with_webserver_tmp(filePath, SIZE_FILE); //do this in designated tmpdir, if any
	
	//set up correct pngPath
	char* result = NULL;

	result = strstri(filePath, webserverRoot);

	if(result != NULL)
	{
		if(strlen(result) > strlen(webserverRoot) && result[strlen(webserverRoot)] != '/')
			sprintf(pngPath, "/%s", result + strlen(webserverRoot));
		else
			strcpy(pngPath, result + strlen(webserverRoot));
	}
	else
		strcpy(pngPath, filePath);

	*strstr(pngPath, ".xml") = 0;

	strcat(pngPath, "_");
	
	strcat(filePath, ".XXXXXX");
	safe_fclose(FILE_mkstemp(filePath)); //create then close a uniquely named file

	unlink(filePath); //we only wanted its name, we never actually use it

	return pngPath;
}

void ModelParser::removeOldFiles(char* filePath)
{
	//remove any aging tmpfiles that may be around for any reason
	std::string pngMask(filePath);
	int rslash = findRightmostPathSeperator(pngMask);

	if (rslash > 0)
		pngMask = pngMask.substr(0, rslash + 1);

	pngMask += "*.ISB.png";
	remove_files_olderthan(pngMask, 600); //kill any png files more than 10 minutes old (600 seconds)
}

/*unused function
char* ModelParser::formatFiles(char* replace)
{
	int num_spaces = inputfiles_->length() - 1;

	if (num_spaces<0)
		return strCopy("");

	int k, len = 0;
	for(k = 0; k < inputfiles_->length(); k++)
		len += (int)strlen((*inputfiles_)[k]);

	// for(int k = 0; k < strlen(inputfiles); k++)
		// if(inputfiles[k] == ' ')
			// num_spaces++;

	char* output = new char[len + (strlen(replace) * (num_spaces + 1)) + 1];
	int index = 0;
	for(k = 0; k < inputfiles_->length(); k++)
	{
		for(int j = 0; (*inputfiles_)[k][j]; j++)
			output[index++] = ((*inputfiles_)[k])[j];
		if(k < inputfiles_->length() - 1)
			for(int j = 0; replace[j]; j++)
				output[index++] = replace[j];
	}

	// for(int k = 0; k < strlen(inputfiles); k++)
		// if(inputfiles[k] == ' ')
			// for(int j = 0; j < strlen(replace); j++)
				// output[index++] = replace[j];
		// else
			// output[index++] = inputfiles[k];

	//end
	for(k = 0; replace[k]; k++)
		if(replace[k] != '\n' && replace[k] != ' ')
			output[index++] = replace[k];
		else
		{
			output[index] = 0;
			return output;
		}

	output[index] = 0;
	return output;
}
*/

void ModelParser::setFilter(Tag* tag)
{
	if(tag == NULL)
		return;

	if(filter_memory_)
	{
		filter_memory_ = False;
		filter_ = False;
	}
	//tag->write(cout);

	if(!strcmp(tag->getName(), "analysis_summary"))
	{
		if(tag->isEnd() && filter_)
			filter_memory_ = True;
		else if(tag->isStart() && ! strcmp(tag->getName(), "analysis_summary") &&
			!strcmp(tag->getAttributeValue("analysis"), prophet_name_) &&
			(timestamp_ == NULL || !strcmp(tag->getAttributeValue("time"), timestamp_)))
		{
			filter_ = True;
			if(timestamp_ == NULL)
			{
				timestamp_ = new char[strlen(tag->getAttributeValue("time")) + 1];
				strcpy(timestamp_, tag->getAttributeValue("time"));
			}
		}
	}
}

Boolean ModelParser::setFvalue(char* scores)
{
	// fval=xxx
	//cout << "score: " << scores << endl;
	if(scores == NULL)
		return False;

	char prefix[] = "fval=";

	if(strlen(scores) <= strlen(prefix))
		return False;
	char* result = strstr(scores, prefix);

	if(result != NULL)
	{
		sscanf(result+strlen(prefix), "%lf", &fvalue_);
		//cout << "fval: " << fvalue_ << endl;
		return True;
	}

	//cout << "returning false" << endl;
	return False;
}

void ModelParser::writeModelResults(ostream& os, char* options, Array<sens_err*>* sensitivity, roc_error_data* chargeData)
{
	char *fp = new char[50000];
	int k;

	sprintf(fp, "<HTML>\n<HEAD>\n<TITLE>%s (%s), %s</TITLE>\n\n", szVERSION, szTPPVersionInfo, szAUTHOR);
	os << fp;

	// style-sheet
	os << "<style type=\"text/css\">\n";
	os << ".hideit {display:none}\n";
	os << ".showit {display:table-row}\n";
	os << ".accepted {background: #87ff87; font-weight:bold;}\n";
	os << ".rejected {background: #ff8700;}\n";
	os << "body{font-family: Helvetica, sans-serif; }\n";
	os << "h1  {font-family: Helvetica, Arial, Verdana, sans-serif; font-size: 24pt; font-weight:bold; color:#0E207F}\n";
	os << "h2  {font-family: Helvetica, Arial, sans-serif; font-size: 20pt; font-weight: bold; color:#0E207F}\n";
	os << "h3  {font-family: Helvetica, Arial, sans-serif; font-size: 16pt; color:#FF8700}\n";
	os << "h4  {font-family: Helvetica, Arial, sans-serif; font-size: 14pt; color:#0E207F}\n";
	os << "h5  {font-family: Helvetica, Arial, sans-serif; font-size: 10pt; color:#AA2222}\n";
	os << "h6  {font-family: Helvetica, Arial, sans-serif; font-size:  8pt; color:#333333}\n";
	os << "table   {border-collapse: collapse; border-color: #000000;}\n";
	os << "td      {border-collapse: collapse; border-color: #000000;}\n";
	os << ".banner_cid   {\n";
	os << "                 background: #0e207f;\n";
	os << "                 border: 2px solid #0e207f;\n";
	os << "                 color: #eeeeee;\n";
	os << "                 font-weight:bold;\n";
	os << "              }\n";
	os << ".markSeq      {\n";
	os << "                 color: #0000FF;\n";
	os << "                 font-weight:bold;\n";
	os << "              }\n";
	os << ".markAA       {\n";
	os << "                 color: #AA2222;\n";
	os << "                 font-weight:bold;\n";
	os << "              }\n";
	os << ".glyco        {\n";
	os << "                 background: #d0d0ff;\n";
	os << "                 border: 1px solid black;\n";
	os << "              }\n";
	os << ".messages     {\n";
	os << "                 background: #ffffff;\n";
	os << "                 border: 2px solid #FF8700;\n";
	os << "                 color: black;\n";
	os << "                 padding: 1em;\n";
	os << "              }\n";
	os << ".formentry    {\n";
	os << "                 background: #eeeeee;\n";
	os << "                 border: 2px solid #0e207f;\n";
	os << "                 color: black;\n";
	os << "                 padding: 1em;\n";
	os << "              }\n";
	os << ".model        {\n";
	os << "                 background: #ffffff;\n";
	os << "                 border: 2px solid #0e207f;\n";
	os << "                 color: black;\n";
	os << "                 padding: 1em;\n";
	os << "              }\n";
	os << ".nav          {\n";
	os << "                 background: #eeeeee;\n";
	os << "                 border: 2px solid #0e207f;\n";
	os << "                 font-family: Helvetica, Arial, Verdana, sans-serif;\n";
	os << "                 font-weight:bold;\n";
	os << "              }\n";
	os << ".navselected  {\n";
	os << "                 background: #dddddd;\n";
	os << "                 border: 2px solid #0e207f;\n";
	os << "                 font-family: Helvetica, Arial, Verdana, sans-serif;\n";
	os << "                 font-weight:bold;\n";
	os << "              }\n";
	os << ".graybox      {\n";
	os << "                 background: #dddddd;\n";
	os << "                 border: 1px solid black;\n";
	os << "                 font-weight:bold;\n";
	os << "              }\n";
	os << ".seq          {\n";
	os << "                 background: #ffaa33;\n";
	os << "                 border: 1px solid black;\n";
	os << "                 font-weight:bold;\n";
	os << "              }\n";
	os << ".info	      {\n";
	os << "                 color: #333333;\n";
	os << "		  font-size: 10pt;\n";
	os << "              }\n";
	os << ".infoplus      {\n";
	os << "                 border-top: 1px solid black;\n";
	os << "                 color: #333333;\n";
	os << "		 font-size: 10pt;\n";
	os << "              }\n";
	os << ".hist_valueL{\n";
	os << "	height: 10px;\n";
	os << "	float: left;\n";
	os << "	background-color: #BBBBBB;\n";
	os << "	border: 1px solid #999999;\n";
	os << "	font-size: 1px;\n";
	os << "	line-height: 0pt;\n";
	os << "	display: inline;\n";
	os << "	clear: both;\n";
	os << "	margin: 0pt 0pt 0pt 0px;\n";
	os << "}\n";
	os << ".hist_valueR{\n";
	os << "	height: 10px;\n";
	os << "	float: right;\n";
	os << "	background-color: #BBBBBB;\n";
	os << "	border: 1px solid #999999;\n";
	os << "	font-size: 1px;\n";
	os << "	line-height: 0pt;\n";
	os << "	display: inline;\n";
	os << "	clear: both;\n";
	os << "	margin: 0pt 0pt 0pt 0px;\n";
	os << "}\n";
	os << ".hist_full{\n";
	os << "	width: 100px;\n";
	os << "	height: 10px;\n";
	os << "	border: 0px solid #999999;\n";
	os << "	font-size: 1px;\n";
	os << "	margin: 0px 0pt 0pt;\n";
	os << "	line-height: 0px;\n";
	os << "}\n";
	os << "</style>\n";

	//some javascript, for the kids...
	os << "<script language=\"JavaScript\">\n";
	os << "var showParams = true;\n";
	os << "var chargeTabs = new Array();\n";
	for(k = 0; k != MAX_CHARGE; k++)
	{
		sprintf(fp, "chargeTabs[%d] = 'chargeTab%d';\n", k, k + 1);
		os << fp;
	}
	os << "function toggleParams()\n";
	os << "{\n";
	os << "	var new_state = 'hideit';\n";
	os << "	if (showParams)\n";
	os << "	{\n";
	os << "		showParams = false;\n";
	os << "		new_state = 'showit';\n";
	os << "		document.getElementById('paramshead').innerHTML = ':   [ - ]';\n";
	os << "	}\n";
	os << "	else\n";
	os << "	{\n";
	os << "		showParams = true;\n";
	os << "		document.getElementById('paramshead').innerHTML = '... [ + ]';\n";
	os << "	}\n";
	os << "	\n";
	os << "	for(var i = 1; i <= 3; i++)\n";
	os << "	{\n";
	os << "		document.getElementById('parameters' + i).className = new_state;\n";
	os << "	}\n";
	os << "}\n";
	os << "function displayChargeInfo(charge)\n";
	os << "{\n";
	os << "	var i;\n";
	os << "	var new_state;\n";
	os << "	var new_nav;\n";
	os << "	for(i in chargeTabs)\n";
	os << "	{\n";
	os << "		if(i == charge)\n";
	os << "		{\n";
	os << "			new_state = 'showit';\n";
	os << "			new_nav = 'nav';\n";
	os << "		}\n";
	os << "		else\n";
	os << "		{\n";
	os << "			new_state = 'hideit';\n";
	os << "			new_nav = 'banner_cid';\n";
	os << "		}\n";
	os << "		document.getElementById(chargeTabs[i] + 'm_state').className = new_state;\n";
	os << "		document.getElementById(chargeTabs[i] + 'm_nav').className = new_nav;\n";
	os << "		document.getElementById(chargeTabs[i] + 't_state').className = new_state;\n";
	os << "		document.getElementById(chargeTabs[i] + 't_nav').className = new_nav;\n";
	os << "	}\n";
	os << "}\n";
	os << "function displayAll()\n";
	os << "{\n";
	os << "	var i;\n";
	os << "	for(i in chargeTabs)\n";
	os << "	{\n";
	os << "		document.getElementById(chargeTabs[i] + 't_state').className = 'showit';\n";
	os << "		document.getElementById(chargeTabs[i] + 't_nav').className = 'navselected';\n";
	os << "	}\n";
	os << "}\n";

	os << "</script>\n";

	os << "</HEAD>\n";

	os << "<body bgcolor=\"#c0c0c0\" onload=\"self.focus();\" link=\"#0000FF\" vlink=\"#0000FF\">\n";

	//overview div
	os << "<table cellspacing=\"0\"><tr><td class=\"banner_cid\">&nbsp;&nbsp;PeptideProphet<sup><small>TM</small></sup> analysis results&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td></tr></table>\n";

	os << "<div class=\"formentry\">\n";
	os << "<table cellpadding=\"2\"><tbody>\n";
	os << "<tr>\n";

	sprintf(fp, "<td valign=\"top\"><img border=\"1\" src=\"%spep-proph.jpg\"></td>\n", HELP_DIR);
	os << fp;

	sprintf(fp, "<td valign=\"top\"><img border=\"1\" src=\"%s\"></td>\n", makeTmpPNGFileSrcRef(pngSensGraphAll_).c_str());
	os << fp;

	os << "<td valign=\"top\">\n";
	sprintf(fp, "Estimated total number of correct peptide assignments in dataset: <b>%0.1f</b><br/><br/>\n", est_tot_num_correct_);
	os << fp;
	sprintf(fp, "<FONT COLOR=\"RED\">Sensitivity</FONT>:  fraction of all correct assignments (<font color=\"blue\">%0.1f</font>) passing MPT filter<br/><br/>\n", est_tot_num_correct_);
	os << fp;
	sprintf(fp, "<FONT COLOR=\"GREEN\">Error</FONT>:  fraction of peptide assignments passing MPT filter that are incorrect<br/><br/>\n");
	os << fp;
	os << "MPT = Minimum Probability Threshhold to Accept<br/><br/>\n";

	// put table of error here
	os << "<center>\n";
	os << "<table style=\"font-family: 'Courier New',Courier,mono; font-size: 8pt;\" bgcolor=\"white\" cellpadding=\"2\" frame=\"border\" rules=\"all\">\n";
	os << "<tr><td><b><i>error</i></b></td><td><b><i>MPT</i></b></td><td><b><i>est # corr</i></b></td></tr>\n";

	for(k = 0; k < NUM_ERRORS; k++)
	{
		sprintf(fp, "<tr><td>%0.3f</td><td>%0.2f</td><td>%d</td></tr>\n", errors_[k], min_prob_threshes_[k], est_num_corrects_[k]);
		os << fp;
	}

	os << "</table>\n</center>\n</td>\n\n<td rowspan=\"2\">\n";

	writeSensitivityTable(os, sensitivity);

	os << "</td>\n</tr>\n<tr>\n\n<td colspan=\"3\">\n";

	sprintf(fp, "<div class=\"infoplus\">PeptideProphetParser (%s) AKeller<br/>",szTPPVersionInfo);
	os << fp;

	os << "<a onclick=\"toggleParams()\">Input Files and Run Parameters<font color=\"blue\" id=\"paramshead\">... [ + ]</font></a>\n";
	os << "</div>\n</td>\n</tr>\n\n";

	os << "<tr id=\"parameters1\" class=\"hideit\">\n";
	os << "<td class=\"info\" align=\"right\" valign=\"top\">Analysis Date: </td>\n";

	sprintf(fp, "<td class=\"info\" colspan=\"3\"><b>%s</b></td>\n", timestamp_);
	os << fp;

	os << "</tr>\n";
	os << "<tr id=\"parameters2\" class=\"hideit\">\n";
	os << "<td class=\"info\" align=\"right\" valign=\"top\">Input Files: </td>\n";
	os << "<td class=\"info\" colspan=\"3\"><tt>";

	for(k = 0; k < inputfiles_->length(); k++)
	{
		os << (*inputfiles_)[k];
		os << "<br/>\n";
	}

	os << "</tt></td></tr>\n";

	os << "<tr id=\"parameters3\" class=\"hideit\">\n";
	os << "<td class=\"info\" align=\"right\" valign=\"top\">Run Options: </td>\n";

	sprintf(fp, "<td class=\"info\" colspan=\"3\">%s</td>\n", options);
	os << fp;

	os << "</tr>\n</tbody>\n</table>\n\n</div>\n\n<br/>\n\n";

	//scores tab
	os << "<table cellspacing = \"0\"><tr>";
	os << "<td class = \"banner_cid\">&nbsp;&nbsp;Scores&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>";
	os << "</tr></table>\n\n";
	os << "<div class=\"formentry\">\n";
	os << "<table>";

	if(strlen(spectrum_) > 0)
	{
		if(display_fval_)
		{
			sprintf(fp, "<b><i>Spectrum:</i></b> %s\n<br/><b><i>Scores:</i></b> %s\n", format(spectrum_), scores_);
			os << fp;
		}
		else
		{
			sprintf(fp, "<b><i>Spectrum:</i></b> %s\n<br/><b><i>Scores:</i></b> unavailable\n", format(spectrum_));
			os << fp;
		}
	} //if real spec

	if(strlen(prob_) > 0)
	{
		os << "<br/>";
		Boolean abbrev = strlen(prob_) == 1 || prob_[0] == '-';
		if(abbrev)
		{
			sprintf(fp, "<b><i>Prob:</i></b> %s", prob_);
			os << fp;
			if(strlen(prob_) > 0 && prob_[0] == '-')
			{
				sprintf(fp, " (possibly correct)");
				os << fp;
			}
			else
			{
				if(display_fval_)
				{
					sprintf(fp, " (unlikely correct)");
					os << fp;
				}
				else
				{//if(strlen(scores.error) == 0) // just wasn't analyzed (not in .esi file)
					sprintf(fp, " (not analyzed)");
					os << fp;
				}
			}
		}
		else
		{
			if(adjusted_)
				sprintf(fp, "  <b><i>Prob:</i></b> %s (adj)", prob_);
			else
				sprintf(fp, "  <b><i>Prob:</i></b> %s", prob_);
			os << fp;
		}
	} //if real prob

	os << "</table>";
	os << "</div>\n\n<br/>\n\n";

	os << "<table><tr>\n";

	for(k = 0; k < model_tags_->length(); k++)
	{
		const char *szClass = (charge_ == k + 1) ? "nav" : "banner_cid";

		sprintf(fp, "<td class=\"%s\" id=\"chargeTab%dm_nav\"><a onclick=\"displayChargeInfo(%d)\">&nbsp;&nbsp;&nbsp;&nbsp;+%d&nbsp;&nbsp;&nbsp;&nbsp;</a></td>\n", szClass, k + 1, k, k + 1);
		os << fp;
	}

	os << "</tr></table>\n\n";

	os << "<div class=\"model\">\n";
	os << "<table>\n";

	//calls writeChargeModelsTab on each possible charge
	for(k = 0; k != MAX_CHARGE; k++)
	{
		writeChargeModelsTab(os, k + 1, &chargeData[k]);
	}

	os << "</table>\n</div><br/>\n\n";

	//model details tab
	os << "<table><tr>\n";
	for(k = 0; k < model_tags_->length(); k++)
	{
		const char *szClass = (charge_ == k + 1) ? "nav" : "banner_cid";

		sprintf(fp,
			"<td class = \"%s\" id = \"chargeTab%dt_nav\"><a onclick = \"displayChargeInfo(%d)\">&nbsp;&nbsp;&nbsp;&nbsp;+%d&nbsp;&nbsp;&nbsp;&nbsp;</a></td>\n",
			szClass, k + 1, k, k + 1);
		os << fp;
	}

	os << "<td id = \"showall_nav\" align = \"right\"><a onclick = \"displayAll()\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<u>Display All</u></a></td>\n";
	os << "</tr></table>\n\n";

	os << "<div class=\"formentry\">\n";
	os << "<table>\n";

	for(k = 0; k < model_tags_->length(); k++)
	{
		const char *szClass = (charge_ == k + 1) ? "showit" : "hideit";

		sprintf(fp, "<tr class = \"%s\" id = \"chargeTab%dt_state\"><td>\n", szClass, k + 1);
		os << fp;
		writeMixtureModelResults(os, (*model_tags_)[k]);
		os << "</td></tr>\n\n";
	}

	os << "</table>\n</div>\n\n";

	os << "</BODY>\n";
	os << "</HTML>\n";

	delete []fp;
	return;
}

void ModelParser::writeSensitivityTable(ostream& os, Array<sens_err*>* sensitivity)
{
	bool pos = false;
	int distr_start = 0;
	char text[5000];
	sprintf(text, "<table frame=\"border\" rules=\"all\" cellpadding=\"2\" bgcolor=\"white\" style=\"font-family: \'Courier New\', Courier, mono; font-size: 8pt;\">\n");
	os << text;
	sprintf(text, "<tr><td><b><i>Min Prob</i></b></td><td><b><i>Sensitivity</i></b></td><td><b><i>Error</i></b></td>\n"); //<td><b><i>est # corr</i></b></td></tr>");
	os << text;
	for(int k = 0; k < sensitivity->length(); k++)
	{
		sprintf(text, "<tr><td>%0.2f</td><td>%0.3f</td><td>%0.3f</td></tr>\n", (*sensitivity)[k]->minprob, (*sensitivity)[k]->sens, (*sensitivity)[k]->err);
		os << text;
	}
	sprintf(text, "</table>\n\n");
	os << text;
}

//function added by Richie Stauffer - August 2010
//it creates the charge models tab that replaced the model results tab
void ModelParser::writeChargeModelsTab(ostream& os, int charge, roc_error_data* chargeData)
{
	char *fp = new char[1000];
	const char *szClass = (charge_ == charge) ? "showit" : "hideit";\

	sprintf(fp, "<tr class=\"%s\" id=\"chargeTab%dm_state\"><td>\n", szClass, charge);
	os << fp;

	//Three display possibilities:
		//hasModel and hasData are false, draw nothing
		//hasModel is true and hasData is false, draw models only
		//hasModel and hasData are true, draw everything
	
	//when hasModel is false hasData should be false also
	
	/////////////////// Basic page layout:
	//MMM SSSSSS TTTT// M = model result graph
	//MMM SSSSSS TTTT// Z = zoomed model graph
	//    SSSSSS TTTT// S = sensitivity graph
	//ZZZ ## EEE TTTT// # = estimated num correct
	//ZZZ ## EEE TTTT// E = error table
	/////////////////// T = sensitivity table
	
	if (!chargeData->hasModel && !chargeData->hasData)
	{
		sprintf(fp, "<b><font color=\"red\">No models or data found for charge +%d.</font></b>\n\n", charge);
		os << fp;
	}
	else if (chargeData->hasModel && !chargeData->hasData) //this case only happens when an older pep.xml file is being read
	{	
		os << "<table>\n";
		os << "<tr><th align = \"left\">Model Results:</th><th align = \"left\">Sensitivity and Error:</th></tr><tr>\n";

		sprintf(fp, "<td valign = \"top\" rowspan = \"2\"><img src = \"%s\"/><br/>\n", makeTmpPNGFileSrcRef(pngModel_[charge - 1]).c_str());
		os << fp;
		sprintf(fp, "<img src = \"%s\"/></td>\n", makeTmpPNGFileSrcRef(pngModelZoom_[charge - 1]).c_str());
		os << fp;
		sprintf(fp, "<td valign = \"top\"><b><font color=\"red\">No data found for charge +%d.</font></b></td>\n\n</tr></table>\n", charge);
		os << fp;
	}
	else if (chargeData->hasData && chargeData->hasModel) //show everything
	{
		os << "<table>\n";
		os << "<tr><th align = \"left\">Model Results:</th><th align = \"left\">Sensitivity and Error:</th></tr><tr>\n";

		sprintf(fp, "<td valign = \"top\" rowspan = \"2\"><img src = \"%s\"/><br/>\n", makeTmpPNGFileSrcRef(pngModel_[charge - 1]).c_str());
		os << fp;
		sprintf(fp, "<img src = \"%s\"/></td>\n", makeTmpPNGFileSrcRef(pngModelZoom_[charge - 1]).c_str());
		os << fp;
		sprintf(fp, "<td colspan = \"2\"><img src = \"%s\"/></td>\n", makeTmpPNGFileSrcRef(pngSensGraph_[charge - 1]).c_str());
		os << fp;

		os << "<td rowspan = \"2\">";

		writeSensitivityTable(os, chargeData->sensitivity);

		os << "</td>\n</tr><tr>\n";
		sprintf(fp, "<td align = \"center\"><i>Est # corr</i><br/>%0.1f</td>\n", chargeData->charge_est_correct);
		os << fp;
		os << "<td align = \"center\">";

		os << "<table style = \"font-family: 'Courier New',Courier,mono; font-size: 8pt;\" bgcolor = \"white\" cellpadding = \"2\" frame = \"border\" rules = \"all\">\n";
		os << "<tr><td><b><i>error</i></b></td><td><b><i>MPT</i></b></td><td><b><i>est # corr</i></b></td></tr>\n";

		for(char i = 0; i != NUM_ERRORS; i++)
		{
			sprintf(fp, "<tr><td>%0.3f</td><td>%0.2f</td><td>%d</td></tr>\n", errors_[i], chargeData->errorMPT[i], chargeData->errorENC[i]);
			os << fp;
		}

		os << "</table></td>\n</tr></table>\n";
	}

	os << "</td></tr>\n\n";

	delete[] fp;
}

void ModelParser::writeMixtureModelResults(ostream& os, Array<Tag*>* tags)
{
	bool pos = false;
	int distr_start = 0;
	char text[5000];
	Tag* tag = NULL;
	sprintf(text, "<pre>");
	os << text;
	for(int k = 0; k < tags->length(); k++)
	{
		tag = (*tags)[k];

		if(tag->isStart() && !strcmp(tag->getName(), "mixture_model"))
		{
			sprintf(text, "<br/><br/>FINAL %s+ MODEL after %s iterations:<br/>", tag->getAttributeValue("precursor_ion_charge"), tag->getAttributeValue("num_iterations"));
			os << text;
			sprintf(text, "number of spectra: %s<br/>", tag->getAttributeValue("tot_num_spectra"));
			os << text;
			sprintf(text, "%s<br/>", tag->getAttributeValue("comments"));
			os << text;
			sprintf(text, "  prior: %s, total: %s<br/>", tag->getAttributeValue("prior_probability"), tag->getAttributeValue("est_tot_correct"));
			os << text;
		}
		else if(tag->isStart() && ! strcmp(tag->getName(), "mixturemodel_distribution"))
		{
			sprintf(text, "%s<br/>", tag->getAttributeValue("name"));
			os << text;
		}
		else if(tag->isStart() && ! strcmp(tag->getName(), "posmodel_distribution"))
		{
			distr_start = k;
			pos = True;
		}
		else if(tag->isStart() && ! strcmp(tag->getName(), "negmodel_distribution"))
		{
			distr_start = k;
			pos = False;
		}
		else if(tag->isEnd() && (! strcmp(tag->getName(), "posmodel_distribution") || ! strcmp(tag->getName(), "negmodel_distribution")))
		{
			writeDistributionResults(os, pos, distr_start, k, tags);
		}
	} //next tag

	sprintf(text, "</pre>");
	os << text;
}

void ModelParser::writeDistributionResults(ostream& os, Boolean pos, int start_ind, int end_ind, Array<Tag*>* tags)
{
	Boolean first = True;
	char type[150];
	type[0] = 0;
	Tag* tag = NULL;
	for(int k = start_ind; k < end_ind; k++)
	{
		tag = (*tags)[k];

		if(tag->isStart() && ((pos && !strcmp(tag->getName(), "posmodel_distribution")) ||
			(!pos && !strcmp(tag->getName(), "negmodel_distribution"))))
		{
			if(tag->getAttributeValue("type") != NULL)
			{
				strcpy(type, tag->getAttributeValue("type"));
				strcat(type, " ");
			}
			os << "  ";
			if(pos)
				os << "pos";
			else
				os << "neg";
			os << ": (" << type;
		}
		else if(tag->isStart() && ! strcmp(tag->getName(), "parameter"))
		{
			if(first)
				first = False;
			else
				os << ", ";
			os << tag->getAttributeValue("name") << " " << tag->getAttributeValue("value");
		}
	} // next tag
	os << ")<br/>";
}

char* ModelParser::format(char* spec)
{
	char output[SIZE_FILE*2];
	output[0] = 0;
	char insertion[] = "<br>";
	int k;
	int num_insertions = (int)(strlen(spec)-1) / MAX_SPEC_LEN;

	if(num_insertions < 1 || strlen(spec) - MAX_SPEC_LEN < 8)
		return spec; //nothing to do

	for(k = 0; k < num_insertions; k++)
	{
		strncat(output, spec + (k * MAX_SPEC_LEN), MAX_SPEC_LEN);
		output[(k+1)*MAX_SPEC_LEN + k*strlen(insertion)] = 0;
		strcat(output, insertion);
	}

	//last piece
	strcat(output, spec + num_insertions * MAX_SPEC_LEN);
	strcpy(spec, output);
	return spec;
}

