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

#include <string.h>
#include <stdio.h>
#include <fstream>
#include "common/TPPVersion.h" //contains version number, name, revision
#include "common/sysdepend.h"
#include "common/util.h"

#include "IModelParser.h"
//constructor
iModelParser::iModelParser(const char* xmlfile, const char* timestamp, const char* spectrum, const char* scores, char* prob) : Parser(NULL)
{
	timestamp_ = NULL;

	inputfiles_ = new Array<char*>;

	adjusted_ = False;

	if(timestamp != NULL)
	{
		timestamp_ = new char[strlen(timestamp) + 1];
		strcpy(timestamp_, timestamp);
	}

	//set text below images (spectrum_)
	spectrum_ = new char[strlen(spectrum) + 1];
	strcpy(spectrum_, spectrum);

	//set text below images (scores_)
	scores_ = new char[strlen(scores) + 1];
	strcpy(scores_, scores);

	//change : to =
	for(int k = 0; scores_[k]; k++)
		if(scores_[k] == ':')
			scores_[k] = '=';

	if(strlen(prob) > 0 && prob[strlen(prob) - 1] == 'a') //if last character in prob is 'a'
	{
		adjusted_ = True;
		prob[strlen(prob) - 1] = 0;
	}

	//set text below images (prob_)
	prob_ = new char[strlen(prob) + 1];
	strcpy(prob_, prob);

	est_tot_num_correct_psm_ = 0.0;
	est_tot_num_correct_pep_ = 0.0;

	//set initial error table values
	for (char i = 0; i != ERROR_NUM; i++)
	{
		errors_[i] = 0.025 * i;
		min_prob_threshes_[i] = 0.0;
		est_num_corrects_[i] = 0;
	}

	prophet_name_ = "interprophet";

	char *xf = strdup(xmlfile);
	
	fixPath(xf, 1); //tidy up path seperators etc - expect existence

	init(xf);

	free(xf);
}

//destructor
iModelParser::~iModelParser()
{
	delete[] timestamp_;
	delete[] spectrum_;
	delete[] scores_;
	delete[] prob_;

	//delete all png paths
	delete[] pngsensgraph_;

	int i;
	for (i = 0; i != MODEL_IMGS; i++)
		delete[] pngmodels_[i];
	for (i = 0; i != PERFORM_IMGS; i++)
		delete[] pngperforms_[i];

	if(inputfiles_ != NULL)
	{
		for(i = 0; i != inputfiles_->length(); i++)
			if((*inputfiles_)[i] != NULL)
				delete[] (*inputfiles_)[i];
		delete inputfiles_;
	}
}

void iModelParser::parse(const char* xmlfile)
{
	Tag* tag = NULL;
	char* data = NULL;

	Boolean analyze = False;

	//collects sensitivity graph data from xml file
	Array<sensTableDataPoint*>* sensitivity = new Array<sensTableDataPoint*>;

	sensTableDataPoint* next_sens = NULL; //single data set, used to help transfer data

	//collects n-score graph data from xml file
	nScoreDataSet* nScoreData[MODEL_IMGS];

	for (char i = 0; i != MODEL_IMGS; i++)
		nScoreData[i] = new nScoreDataSet();

	nScoreDataPoint* next_score = NULL; //single data set, used to help transfer data

	pwiz::util::random_access_compressed_ifstream fin(xmlfile); //can read gzipped files

	if(!fin)
	{
		cerr << "iModelParser: error opening" << xmlfile << endl;
		exit(1);
	}

	Boolean done = False, collect = False;

	int error_index = 0;

	char options[SIZE_FILE];
	*options = 0;

	char* nextline = new char[line_width_];

	N_Score curNScore = NONE;

	while(!done && fin.getline(nextline, line_width_))
	{
		data = strstr(nextline, "<");

		while(data != NULL)
		{
			tag = new Tag(data);

			if(tag != NULL)
			{
				if(filter_)
					analyze = True;

				//assume there won't be an "interprophet_summary" tag, work around by using analysis_summary
				//set filter function content copied up here since it also uses analysis_summary tag
				if(filter_memory_) //filter_ and filter_memory_ come from parent parser class
				{
					filter_memory_ = False;
					filter_ = False;
				}

				if(tag->isStart() && !strcmp(tag->getName(), "interprophet_summary"))
				{
					//set estimated total correct assignments
					if (tag->getAttributeValue("est_tot_num_correct_psm") == NULL)
						est_tot_num_correct_psm_ = 0;
					else
						est_tot_num_correct_psm_ = atof(tag->getAttributeValue("est_tot_num_correct_psm"));

					if (tag->getAttributeValue("est_tot_num_correct_pep") == NULL)
						est_tot_num_correct_pep_ = 0;
					else
						est_tot_num_correct_pep_ = atof(tag->getAttributeValue("est_tot_num_correct_pep"));

					//set run options (part of run parameters)
					if (tag->getAttributeValue("options") == NULL)
						strcpy(options, "no options found in XML file");
					else
						strcpy(options, tag->getAttributeValue("options"));
				}
				if(tag->isStart() && !strcmp(tag->getName(), "analysis_summary"))
				{
					if(tag->isEnd() && filter_)
						filter_memory_ = True;
					else if(tag->isStart()
					&& !strcmp(tag->getAttributeValue("analysis"), prophet_name_)
					&& (timestamp_ == NULL || !strcmp(tag->getAttributeValue("time"), timestamp_)))
					{
						filter_ = True;

						if(timestamp_ == NULL)
						{
							//set analysis date, timestamp_ (part of run parameters)
							timestamp_ = new char[strlen(tag->getAttributeValue("time")) + 1];
							strcpy(timestamp_, tag->getAttributeValue("time"));
						}
					}
				}

				if(analyze && tag->isEnd() && !strcmp(tag->getName(), "analysis_summary"))
				{
					analyze = False; //done
					done = True;
					break; //no more need to read summary
				}

				if(analyze) //grab desired data
				{
					if(tag->isStart() && !strcmp(tag->getName(), "inputfile"))
					{
						char* next_file = new char[strlen(tag->getAttributeValue("name")) + 1];
						strcpy(next_file, tag->getAttributeValue("name"));
						inputfiles_->insertAtEnd(next_file);
					}
					else if(!strcmp(tag->getName(), "roc_data_point"))
					{
						next_sens = new sensTableDataPoint;
						next_sens->minprob = atof(tag->getAttributeValue("min_prob"));
						next_sens->sens = atof(tag->getAttributeValue("sensitivity"));
						next_sens->err = atof(tag->getAttributeValue("error"));

						//set large table data (sensitivity table)
						sensitivity->insertAtEnd(next_sens);
					}
					else if(error_index < ERROR_NUM && tag->isStart() && !strcmp(tag->getName(), "error_point"))
					{
						double nexterr = atof(tag->getAttributeValue("error"));
						double maxdiff = 0.005;

						if(nexterr == errors_[error_index] ||
						(nexterr > errors_[error_index] && nexterr - errors_[error_index] <= maxdiff) ||
						(nexterr < errors_[error_index] && errors_[error_index] - nexterr <= maxdiff))
						{
							//set small table MPT and est # corr columns
							min_prob_threshes_[error_index] = atof(tag->getAttributeValue("min_prob"));
							est_num_corrects_[error_index] = atoi(tag->getAttributeValue("num_corr"));
							error_index++;
						}
					}

					if(tag->isStart() && !strcmp(tag->getName(), "mixturemodel"))
					{
						collect = True;

						char nScoreName[N_SCORE_SIZE]; //scores should be 4 chars long counting \0
						strcpy(nScoreName, tag->getAttributeValue("name"));

						if(!strcmp(nScoreName, "NSS"))
							curNScore = NSS;
						else if(!strcmp(nScoreName, "NRS"))
							curNScore = NRS;
						else if(!strcmp(nScoreName, "NSE"))
							curNScore = NSE;
						else if(!strcmp(nScoreName, "NSI"))
							curNScore = NSI;
						else if(!strcmp(nScoreName, "NSM"))
							curNScore = NSM;
						else if(!strcmp(nScoreName, "NSP"))
							curNScore = NSP;
						else if(!strncmp(nScoreName, "FPK", 3))
							curNScore = FPKM;
						else if(!strncmp(nScoreName, "LEN", 3))
							curNScore = LENGTH;
						else
						{
							if(!strcmp(nScoreName, "TopCat")) //TopCat score is recognized but nothing is done with it yet
								printf("<!--TopCat score found.-->");
							else
								printf("Warning: \"%s\" score not recognized!", nScoreName);

							curNScore = NONE;
						}

						//set nScoreData score value
						if (curNScore != NONE)
						{
							//parse score value from scores_
						  char scoreVal[strlen(scores_) + 1];
						  strcpy(scoreVal, scores_);
						  
						  char* tmpScore = strstri(scoreVal, nScoreName);
						  char* tmp2 = strchr(tmpScore, '=') + 1;
						  char* tmp3 = strchr(tmp2, ' ');
						  
						  if(tmp3 != NULL)
						    *tmp3 = 0;

						  nScoreData[curNScore]->scoreVal = atof(tmp2);
						  
						  //nScoreData[curNScore]->posBW = atof(tag->getAttributeValue("pos_bandwidth"));
						  //nScoreData[curNScore]->negBW = atof(tag->getAttributeValue("neg_bandwidth"));
						  
						  
						}
					}
					else if(tag->isEnd() && !strcmp(tag->getName(), "mixturemodel"))
						collect = False;

					if (collect && curNScore != NONE && !strcmp(tag->getName(), "point"))
					{
						next_score = new nScoreDataPoint();
						next_score->nScore = atof(tag->getAttributeValue("value"));
						next_score->pos = atof(tag->getAttributeValue("pos_dens"));
						next_score->neg = atof(tag->getAttributeValue("neg_dens"));
						next_score->ln = log(next_score->pos / next_score->neg);

						//check if the observed values are present
						const char* nObs = tag->getAttributeValue("neg_obs_dens"); //keep in this order, neg obs first in xml
						const char* pObs = tag->getAttributeValue("pos_obs_dens");

						drawPobs_ = pObs != NULL;
						drawNobs_ = nObs != NULL;
						next_score->posObs = drawPobs_ ? atof(pObs) : 0;
						next_score->negObs = drawNobs_ ? atof(nObs) : 0;

						nScoreData[curNScore]->dataPoints->insertAtEnd(next_score);
					}
				}

				if(!collect)
					delete tag;
			}

			data = strstr(data + 1, "<");
		}
	}

	tag = NULL;
	delete[] tag;

	fin.close();
	delete[] nextline;

	data = NULL;
	//done reading xml file, now write graphing data to szTmpDataFile and call plotModel to create png's

	if(timestamp_ != NULL && strcmp(prob_, "-nofigs"))
	{
		char* filePath = new char[SIZE_FILE];
		strcpy(filePath, xmlfile);
		

		char* performPath = setFilePaths(filePath);
		removeOldFiles(filePath);

		char* szTmpDataFile = new char[SIZE_FILE];
		sprintf(szTmpDataFile, "%s.data", filePath);

		//write data files
		FILE* fpTmp;

		char
		gnuplotBlack = -1,
		gnuplotRed = 1,
		gnuplotGreen = 2,
		gnuplotBlue = 3;
		//if needed pink/purple = 4

		time_t tStartTime = time((time_t *)NULL);
		srandom((int)(strlen(filePath) + tStartTime + filePath[strlen(filePath) - 3]));

		long rndPath = random();

		//create sensitivity graph data file and png
		if(sensitivity->length())
		{
			if((fpTmp = fopen(szTmpDataFile, "w")) == NULL)
				printf("Error - cannot create sensitivity graph data file: %s.", szTmpDataFile);
			else
			{
				for(int i = 0; i != sensitivity->length(); i++) //open and write file
					fprintf(fpTmp, "%0.2f\t%0.3f\t%0.3f\n", (*sensitivity)[i]->minprob, (*sensitivity)[i]->sens, (*sensitivity)[i]->err);
				fclose(fpTmp); //close file

				pngsensgraph_ = plotSensitivity(filePath, rndPath, gnuplotRed, gnuplotGreen);
			}
		}
		else
			pngsensgraph_ = NULL;

		//create n-score graph data files and png's

		char* scoreName = new char[N_SCORE_SIZE];
		char* scoreTitle = new char[TEXT_SIZE1];

		for(char i = 0; i != MODEL_IMGS; i++)
		{
			if(nScoreData[i]->dataPoints->length()) //if number of data points is not 0
			{
				if((fpTmp = fopen(szTmpDataFile, "w")) == NULL)
					printf("Error - cannot create N-Score graph data file: %s.", szTmpDataFile);
				else
				{
					for(int j = 0; j != nScoreData[i]->dataPoints->length(); j++) //open and write file
					{
						fprintf(fpTmp, "%0.4f\t%0.4f\t%0.4f\t%0.4f\t%0.4f\t%0.4f\n", //rounding to 4 decimal places
						(*nScoreData[i]->dataPoints)[j]->nScore,
						(*nScoreData[i]->dataPoints)[j]->pos,
						(*nScoreData[i]->dataPoints)[j]->neg,
						(*nScoreData[i]->dataPoints)[j]->ln,
						(*nScoreData[i]->dataPoints)[j]->posObs,
						(*nScoreData[i]->dataPoints)[j]->negObs);
					}
					fclose(fpTmp); //close file

					switch ((N_Score)i)
					{
						case NSS:
							strcpy(scoreName, "NSS");
							strcpy(scoreTitle, "NSS (Number of Sibling Searches)");
						break;
						case NRS:
							strcpy(scoreName, "NRS");
							strcpy(scoreTitle, "NRS (Number of Replicate Spectra)");
						break;
						case NSE:
							strcpy(scoreName, "NSE");
							strcpy(scoreTitle, "NSE (Number of Sibling Experiments)");
						break;
						case NSI:
							strcpy(scoreName, "NSI");
							strcpy(scoreTitle, "NSI (Number of Sibling Ions)");
						break;
						case NSM:
							strcpy(scoreName, "NSM");
							strcpy(scoreTitle, "NSM (Number of Sibling Modifications)");
						break;
						case NSP:
							strcpy(scoreName, "NSP");
							strcpy(scoreTitle, "NSP (Number of Sibling Peptides)");
						break;
						case FPKM:
							strcpy(scoreName, "FPK");
							strcpy(scoreTitle, "FPKM (Fragments per Kilobase)");
						break;
						case LENGTH:
							strcpy(scoreName, "LEN");
							strcpy(scoreTitle, "LENGTH (Peptide Length)");
						break;
						default:
							strcpy(scoreName, "???"); //unknown score
							strcpy(scoreTitle, "Unknown Score");
						break;
					}

					pngmodels_[i] = plotModel(filePath, scoreName, scoreTitle, nScoreData[i]->scoreVal, rndPath,
					gnuplotBlue, gnuplotRed, gnuplotBlack, gnuplotGreen); //plot model
				}
			}
			else
				pngmodels_[i] = NULL;
		}

		
		//load performance png's from file path of xml file if they are present

		char* tmpPath = new char[SIZE_FILE];
		char* fileName = new char[TEXT_SIZE1];
		char* xmlCopy = new char[SIZE_FILE];
		strcpy(xmlCopy, xmlfile);

		*strstr(xmlCopy, ".xml") = 0;
		strcat(xmlCopy, "_");

		for (char i = 0; i != PERFORM_IMGS; i++)
		{
			strcpy(tmpPath, xmlCopy);

			switch(i)
			{
				case 0: strcpy(fileName, "FDR"); break;
				case 1: strcpy(fileName, "FDR_10pc"); break;
				case 2: strcpy(fileName, "IPPROB"); break;
				case 3: strcpy(fileName, "PPPROB"); break;
				case 4: strcpy(fileName, "CORR"); break;
				case 5: strcpy(fileName, "ROC"); break;
			}
			strcat(fileName, ".png");

			strcat(tmpPath, fileName);

			if ((fpTmp = fopen(tmpPath, "r")) == NULL)
				pngperforms_[i] = NULL;
			else
			{
			  //strcpy(tmpPath, performPath);
			  //strcat(tmpPath, fileName);

				pngperforms_[i] = new char[SIZE_FILE];
				strcpy(pngperforms_[i], tmpPath);
#ifdef USING_RELATIVE_WEBSERVER_PATH 			
//				translate_absolute_filesystem_path_to_relative_webserver_root_path(tmpPath);
//				sprintf(pngperforms_[i], "%s%s",getWebserverUrl(), tmpPath);
#endif
				fclose(fpTmp);
			}
		}

		delete[] tmpPath;
		delete[] fileName;
		delete[] xmlCopy;

		delete[] scoreName;
		delete[] scoreTitle;

		delete[] filePath;
		delete[] performPath;

		unlink(szTmpDataFile);

		delete[] szTmpDataFile;
	}

	writeModelResults(cout, options, sensitivity);
}

//this correctly sets filePath and returns the path of the performance pngs to be loaded
char* iModelParser::setFilePaths(char* filePath)
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

	//set up correct filePath
	replace_path_with_webserver_tmp(filePath, sizeof(filePath)); //do this in designated tmpdir, if any

	strcat(filePath, ".XXXXXX");
	safe_fclose(FILE_mkstemp(filePath)); //create then close a uniquely named file

	unlink(filePath); //we only wanted its name, we never actually use it

	return pngPath;
}

void iModelParser::removeOldFiles(char* filePath)
{
	//remove any aging tmpfiles that may be around for any reason
	std::string pngMask(filePath);
	int rslash = findRightmostPathSeperator(pngMask);

	if (rslash > 0)
		pngMask = pngMask.substr(0, rslash + 1);

	pngMask += "*.ISB.png";
	remove_files_olderthan(pngMask, 600); //kill any png files more than 10 minutes old (600 seconds)
}

char* iModelParser::plotSensitivity(char* filePath, long random, char sensColor, char errColor)
{
	FILE* fpPlotCode;

	char* pngFilePath = new char[SIZE_FILE];
	char* dataFilePath = new char[SIZE_FILE]; //by this point data file exists and is populated, but it's file path is known
	char* plotFilePath = new char[SIZE_FILE];

	char plotCommand[TEXT_SIZE3];

	sprintf(pngFilePath, "%s.%ld.ISB.png", filePath, random);
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

		//sprintf(plotCommand, "cd %s; %s %s", plotFilePath, GNUPLOT_BINARY, plotFileName);
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

//draws an N-score graph
char* iModelParser::plotModel(char* filePath, char* scoreName, char* scoreTitle, double scoreVal, long random,
char posColor, char negColor, char lnColor, char scoreColor)
{
	FILE* fpPlotCode;

	char* pngFilePath = new char[SIZE_FILE];
	char* dataFilePath = new char[SIZE_FILE]; //by this point data file exists and is populated, but it's file path is known
	char* plotFilePath = new char[SIZE_FILE];

	char plotCommand[TEXT_SIZE3];

	//assume filePath = c:Inetpub/wwwroot/ISB/data/class/iProphet/xtandem/interact.iproph.pep.xml.a#####

	sprintf(pngFilePath, "%s.%ld.%s.ISB.png", filePath, random, scoreName);
	sprintf(plotFilePath, "%s.gp", filePath);
	sprintf(dataFilePath, "%s.data", filePath);

	//plot command
	//cd c:/Inetpub/wwwroot/ISB/data/class/iProphet/xtandem; wgnuplot.exe interact.iproph.pep.xml.a02644.gp; rm -f interact.iproph.pep.xml.a02644.gp
	//return path
	//c:/Inetpub/wwwroot//ISB/data/class/iProphet/xtandem/interact.iproph.pep.xml.a02644.31533.ISB.png

  	if ((fpPlotCode = fopen(plotFilePath, "w")) == NULL)
		printf("Error - cannot create gnuplot file %s.", plotFilePath);
	else
	{
		//gnuplot code
		fprintf(fpPlotCode, "set terminal png truecolor size %d, %d\n", IMG_W, IMG_H);
		fprintf(fpPlotCode, "set output \"%s\"\n", pngFilePath);
		fprintf(fpPlotCode, "set border 1\n");
		fprintf(fpPlotCode, "set xzeroaxis linetype -1 linewidth 1.0\n");
		fprintf(fpPlotCode, "set x2zeroaxis linetype 0 linewidth 1.0\n");
		fprintf(fpPlotCode, "set xlabel \"%s\"\n", scoreTitle);
		fprintf(fpPlotCode, "set ylabel \"Density\"\n");
		fprintf(fpPlotCode, "set y2label \"ln(P/N)\"\n");
		fprintf(fpPlotCode, "set xtics border nomirror out\n");
		fprintf(fpPlotCode, "set ytics border nomirror out\n");
		fprintf(fpPlotCode, "set y2tics border nomirror out\n");
		fprintf(fpPlotCode, "set mxtics\n");
		fprintf(fpPlotCode, "set mytics\n");
		fprintf(fpPlotCode, "set my2tics\n");
		fprintf(fpPlotCode, "set key bottom right\n");
		fprintf(fpPlotCode, "set arrow 1 from %f, 0 to %f, 100 nohead lw 2 lc %d\n", scoreVal, scoreVal, scoreColor);
		fprintf(fpPlotCode, "plot 0 title \"%s = %g\" lw 2 lc %d, \\\n", scoreName, scoreVal, scoreColor);
		fprintf(fpPlotCode, "\"%s\" using 1:2 title \"Positive Model (P)\" with line lw 2 lc %d, \\\n", dataFilePath, posColor);
		fprintf(fpPlotCode, "\"%s\" using 1:3 title \"Negative Model (N)\" with line lw 2 lc %d, \\\n", dataFilePath, negColor);
		if(drawPobs_)
			fprintf(fpPlotCode, "\"%s\" using 1:5 title \"Positive Observed\" with boxes lc %d fs transparent solid 0.2 noborder, \\\n", dataFilePath, posColor);
		if(drawNobs_)
			fprintf(fpPlotCode, "\"%s\" using 1:6 title \"Negative Observed\" with boxes lc %d fs transparent solid 0.2 noborder, \\\n", dataFilePath, negColor);
		fprintf(fpPlotCode, "\"%s\" using 1:4 title \"ln(P/N)\" axes x1y2 with line lc %d\n", dataFilePath, lnColor);

		fclose(fpPlotCode);

		char* plotFileName = findRightmostPathSeperator(plotFilePath);
		*plotFileName++ = '\0';

		//write and call plot command
		sprintf(plotCommand, "cd %s; %s %s; rm -f %s", plotFilePath, GNUPLOT_BINARY, plotFileName, plotFileName);
		//sprintf(plotCommand, "cd %s; %s %s", plotFilePath, GNUPLOT_BINARY, plotFileName);
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

//main html generating function
void iModelParser::writeModelResults(ostream& os, char* options, Array<sensTableDataPoint*>* sensitivity)
{
	char *fp = new char[TEXT_SIZE2];
	sprintf(fp, "<html>\n<head>\n<title>%s (%s), %s</title>\n\n", szVERSION, szTPPVersionInfo, szAUTHOR);
	os << fp;

	//style-sheet
	os << "<!--Style-sheet-->\n";
	os << "<style type = \"text/css\">\n";
	os << ".hideit\n{\n\tdisplay: none;\n}\n";
	os << ".showit\n{\n\tdisplay: table-row;\n}\n";
	os << "body\n{\n\tfont-family: Helvetica, sans-serif;\n}\n";
	os << "table\n{\n\tborder-collapse: collapse;\n\tborder-color: #000000;\n}\n";
	os << ".banner_cid\n{\n\tbackground: #0e207f;\n\tborder: 2px solid #0e207f;\n\tcolor: #eeeeee;\n\tfont-weight: bold;\n}\n";
	os << ".formentry\n{\n\tbackground: #eeeeee;\n\tborder: 2px solid #0e207f;\n\tcolor: black;\n\tpadding: 1em;\n}\n";
	os << ".model\n{\n\tbackground: #ffffff;\n\tborder: 2px solid #0e207f;\n\tcolor: black;\n\tpadding: 1em;\n}\n";
	os << ".info\n{\n\tcolor: #333333;\n\tfont-size: 10pt;\n}\n";
	os << ".infoplus\n{\n\tborder-top: 1px solid black;\n\tcolor: #333333;\n\tfont-size: 10pt;\n}\n";
	os << "</style>\n\n";

	//javascript
	os << "<!--Javascript variables and functions-->\n";
	os << "<script language = \"JavaScript\">\n";
	os << "var showParams = true;\n";
	os << "function toggleParams()\n{\n";
	os << "\tvar new_state = 'hideit';\n\n";
	os << "\tif (showParams)\n\t{\n";
	os << "\t\tnew_state = 'showit';\n";
	os << "\t\tdocument.getElementById('paramshead').innerHTML = ':   [ - ]';\n";
	os << "\t}\n\telse\n";
	os << "\t\tdocument.getElementById('paramshead').innerHTML = '... [ + ]';\n\n";
	os << "\tshowParams = !showParams;\n\n";
	os << "\tfor(var i = 1; i != 4; i++)\n";
	os << "\t\tdocument.getElementById(\"parameters\"+i).className = new_state;\n";
	os << "}\n";
	os << "</script>\n";

	os << "</head>\n\n";

	//main body code
	os << "<body bgcolor = \"#c0c0c0\" onload = \"self.focus();\" link = \"#0000FF\" vlink = \"#0000FF\" alink = \"#FF0000\">\n\n";

	os << "<table cellspacing = \"0\"><tr>\n<td class = \"banner_cid\">&nbsp;&nbsp;iProphet<sup><small>TM</small></sup> Analysis Results&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>\n</tr></table>\n";
	os << "<div class = \"formentry\">\n\n";

	//analysis results tab
	os << "<!--Analysis Results Tab-->\n";
	os << "<table cellpadding = \"2\" border = \"0\"><tbody>\n";
	os << "<tr>\n";

	//write main graph path to html file
	if (pngsensgraph_ != NULL)
	{
		sprintf(fp, "<td rowspan = \"1\" valign = \"top\"><img border = \"1\" src = \"%s&keep=0\"></td>\n", makeTmpPNGFileSrcRef(pngsensgraph_).c_str());
		os << fp;
	}

	//main graph/table text
	os << "<td rowspan = \"1\" valign = \"top\">\n";

	//write total correct assignments to html
	sprintf(fp, "Estimated total number of correct PSM assignments in dataset: <b>%0.1f</b><br/><br/>\n", est_tot_num_correct_psm_);
	os << fp;

	sprintf(fp, "Estimated total number of correct Distinct Peptide Sequence assignments in dataset: <b>%0.1f</b><br/><br/>\n", est_tot_num_correct_pep_);
	os << fp;
	//write sensitivity to html
	sprintf(fp, "<font color = \"red\">Sensitivity:</font> fraction of all correct assignments (<font color = \"DarkBlue\">%0.1f</font>) passing MPT filter<br/><br/>\n", est_tot_num_correct_psm_);
	os << fp;
	sprintf(fp, "<font color = \"green\">Error</font>: fraction of peptide assignments passing MPT filter that are incorrect<br/><br/>\n");
	os << fp;
	os << "MPT = Minimum Probability Threshhold to Accept<br/><br/>\n";

	//write error table (small one) to html
	os << "<!--Small table-->\n";
	os << "<center>\n";
	os << "<table style = \"font-family: 'Courier New', Courier, mono; font-size: 8pt;\" bgcolor = \"white\" cellpadding = \"2\" frame = \"border\" rules = \"all\">\n";
	os << "<tr><td><b><i>error</i></b></td><td><b><i>MPT</i></b></td><td><b><i>est # corr</i></b></td></tr>\n";

	for(char i = 0; i != ERROR_NUM; i++) //generate small table
	{
		sprintf(fp, "<tr><td>%0.3f</td><td>%0.2f</td><td>%d</td></tr>\n", errors_[i], min_prob_threshes_[i], est_num_corrects_[i]);
		os << fp;
	}

	os << "</table>\n</center>\n</td>\n\n";

	//main table
	os << "<!--Main table-->\n";
	os << "<td rowspan = \"1\">\n";

	//write large table to html
	writeSensitivityTable(os, sensitivity); //generate main table

	os << "</td>\n</tr>\n<tr>\n\n";

	//information below tables
	os << "<!--Information below tables-->\n";
	os << "<td colspan = \"3\">\n";

	sprintf(fp, "<div class = \"infoplus\">iProphetParser (%s), %s<br/>\n",szTPPVersionInfo, szAUTHOR);
	os << fp;
	os << "<a onclick = \"toggleParams()\">Input Files and Run Parameters<font color = \"blue\" id = \"paramshead\">... [ + ]</font></a>\n";
	os << "</div>\n</td>\n</tr>\n\n";

	os << "<tr id = \"parameters1\" class = \"hideit\">\n";
	//write analysis date to html
	os << "<td class = \"info\" align = \"right\" valign = \"top\">Analysis Date: </td>\n";
	sprintf(fp, "<td class = \"info\" colspan = \"1\"><b>%s</b></td>\n", timestamp_);
	os << fp;

	os << "</tr>\n";
	os << "<tr id = \"parameters2\" class = \"hideit\">\n";

	os << "<td class = \"info\" align = \"right\" valign = \"top\">Input Files: </td>\n";
	os << "<td class = \"info\" colspan = \"3\"><tt>\n";

	//write input file paths to html (part of run parameters)
	for(char i = 0; i != inputfiles_->length(); i++)
	{
		os << (*inputfiles_)[i];
		os << "<br/>\n";
	}

	os << "</tt></td></tr>\n";

	os << "<tr id = \"parameters3\" class = \"hideit\">\n";
	//write run options to html
	os << "<td class = \"info\" align = \"right\" valign = \"top\">Run Options: </td>\n";

	sprintf(fp, "<td class = \"info\" colspan = \"3\">%s</td>\n", options);
	os << fp;

	os << "</tr>\n</tbody>\n</table>\n\n</div>\n\n<br/>\n\n";

	//iprophet models tab
	os << "<!--iProphet Models Tab-->\n";
	os << "<table cellspacing = \"0\"><tr>\n";
	os << "<td class = \"banner_cid\">&nbsp;&nbsp;iProphet Models&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>\n";
	os << "</tr></table>\n\n";

	os << "<div class = \"model\">\n\n";

	//write	model png paths to html, check if null
	for(char i = 0; i != MODEL_IMGS; i++)
	{
		if (pngmodels_[i] != NULL)
		{
			sprintf(fp, "<td><img src = \"%s&keep=0\" \"border = 0\"/></td>\n", makeTmpPNGFileSrcRef(pngmodels_[i]).c_str());
			os << fp;
		}
	}

	os << "<br/>\n";

	//text below model graphs
	os << "<!--Text below model graphs-->\n";
	os << "<div class = \"infoplus\">\n";
	os << "<table cellpadding = \"10\"><tr>\n";

	//the spectrum, scores, and prob values will probably change eventually

	//spectrum and scores
	if(strlen(spectrum_) > 0)
	{
		sprintf(fp, "<td>");
		os << fp;

		//write text below images to html
		sprintf(fp, "<b><i>spectrum:</i></b> %s</td>\n<td><b><i>scores:</i></b> %s</td>\n", format(spectrum_), scores_);
		os << fp;
	}

	//prob
	if(strlen(prob_) > 0)
	{
		sprintf(fp, "<td>");
		os << fp;

		Boolean abbrev = strlen(prob_) == 1 || *prob_ == '-';

		if(abbrev)
		{
			sprintf(fp, "<b><i>iprob:</i></b> %s", prob_);
			os << fp;

			if(strlen(prob_) > 0 && *prob_ == '-')
				sprintf(fp, " (possibly correct)");
			else
				sprintf(fp, " (unlikely correct)"); //fv

			os << fp;
		}
		else
		{
			if(adjusted_)
				sprintf(fp, "<b><i>iprob:</i></b> %s (adj)", prob_);
			else
				sprintf(fp, "<b><i>iprob:</i></b> %s", prob_);

			os << fp;
		}

		sprintf(fp, "</td>\n");
		os << fp;
	}

	os << "</tr></table></div>\n\n</div>\n\n<br/>\n\n";

	Boolean drawPerformTab = false;
	for(char i = 0; i != PERFORM_IMGS; i++)
	{
		if(pngperforms_[i] != NULL)
		{
			drawPerformTab = true;
			break;
		}
	}

	if (drawPerformTab)
	{
		//iprophet performance tab
		os << "<!--iProphet Performance Tab-->\n";
		os << "<table cellspacing = \"0\"><tr>";
		os << "<td class = \"banner_cid\">&nbsp;&nbsp;iProphet Performance&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>";
		os << "</tr></table>\n\n";

		os << "<div class = \"model\">\n";

		//write	performance png paths to html, check if null
		for (char i = 0; i != PERFORM_IMGS; i++)
		{
		  if (pngperforms_[i] != NULL)
		  {
		    sprintf(fp, "<td><a href = \"%s&keep=1\"><img src = \"%s&keep=1\" width = \"%d\" height = \"%d\" border = \"0\"/></a></td>\n",
			    makeTmpPNGFileSrcRef(pngperforms_[i]).c_str(), makeTmpPNGFileSrcRef(pngperforms_[i]).c_str(), IMG_W, IMG_H);
		    // pngperforms_[i], pngperforms_[i], IMG_W, IMG_H);
		    os << fp;
		  }
		}
	}

	os << "</div>\n";
	os << "</body>\n";
	os << "</html>\n";

	//html code finished

	delete []fp;
}

void iModelParser::writeSensitivityTable(ostream& os, Array<sensTableDataPoint*>* sensitivity)
{
	char text[TEXT_SIZE3];

	sprintf(text, "<table frame = \"border\" rules = \"all\" cellpadding = \"2\" bgcolor = \"white\" style = \"font-family: \'Courier New\', Courier, mono; font-size: 8pt;\">\n");
	os << text;
	sprintf(text, "<tr><td><b><i>Min Prob</i></b></td><td><b><i>Sensitivity</i></b></td><td><b><i>Error</i></b></td>\n");
	os << text;

	for(int k = 0; k < sensitivity->length(); k++)
	{
		sprintf(text, "<tr><td>%0.2f</td><td>%0.3f</td><td>%0.3f</td></tr>\n", (*sensitivity)[k]->minprob, (*sensitivity)[k]->sens, (*sensitivity)[k]->err);
		os << text;
	}

	sprintf(text, "</table>\n\n");
	os << text;
}

char* iModelParser::format(char* spec) //used to format spectrum_
{
	char output[SIZE_FILE * 2];
	*output = 0;
	char insertion[] = "<br>";
	int k;
	int num_insertions = (int)(strlen(spec) - 1) / TEXT_SIZE1;

	if(num_insertions < 1 || strlen(spec) - TEXT_SIZE1 < 8)
		return spec; //nothing to do

	for(k = 0; k < num_insertions; k++)
	{
		strncat(output, spec + (k * TEXT_SIZE1), TEXT_SIZE1);
		output[(k + 1) * TEXT_SIZE1 + k * strlen(insertion)] = 0;
		strcat(output, insertion);
	}

	//last piece
	strcat(output, spec + num_insertions * TEXT_SIZE1);
	strcpy(spec, output);

	return spec;
}
