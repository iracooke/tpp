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

#include "stdio.h"
#include "string.h"
#include "common/util.h"
#include "IModelParser.h"
#include "common/TPPVersion.h" //contains version number, name, revision

void EXTRACT_QUERY_STRING(char* xmlile, char* timestamp, char* spec, char* scores, char* prob);
void EXTRACT_QUERY_STRING_helper(char* szQuery, char* xmlile, char* timestamp, char* spec, char* scores, char* prob);

//main
int main(int argc, char** argv)
{
	hooks_tpp handler(argc, argv); //set up install paths, etc

	char xmlfile[TEXT_SIZE2];
	char timestamp[TEXT_SIZE2];
	char spec[TEXT_SIZE2];
	char scores[TEXT_SIZE2];
	char prob[TEXT_SIZE1];

	*xmlfile = 0;
	*timestamp = 0;
	*spec = 0;
	*scores = 0;
	*prob = 0;

	if(argc == 1) //make sure format is correct, 1 = CGI format
	{
		cout << "Content-type: text/html" << endl << endl;
		EXTRACT_QUERY_STRING(xmlfile, timestamp, spec, scores, prob);
	}
	else //this should be changed over to cgi format: extracting arguments from posted information
		EXTRACT_QUERY_STRING_helper(argv[1], xmlfile, timestamp, spec, scores, prob);

	iModelParser *iModel = new iModelParser(xmlfile, timestamp, spec, scores, prob); //create new iModelParser which does everything on it's own
	delete iModel; //then delete it

	return 0;
}

void EXTRACT_QUERY_STRING(char* xmlfile, char* timestamp, char* spec, char* scores, char* prob)
{
	char *pStr = getenv("REQUEST_METHOD");

	if (pStr == NULL)
	{
		printf(" Error - this is a CGI program that cannot be\nrun from the command line.\n\n");
		exit(EXIT_FAILURE);
	}
	else if (!strcmp(pStr, "GET"))
	{
		char *szQuery;

		szQuery = getenv("QUERY_STRING");

		if (szQuery == NULL)
		{
			printf("<P>No query information to decode.\n</BODY>\n</HTML>\n");
			exit(EXIT_FAILURE);
		}

		EXTRACT_QUERY_STRING_helper(szQuery, xmlfile, timestamp, spec, scores, prob);
	}
	else
	{
		printf(" Error not called with GET method\n");
		exit(1);
	}
}

void EXTRACT_QUERY_STRING_helper(char *szQuery, char* xmlfile, char* timestamp, char* spec, char* scores, char* prob)
{
	char szWord[TEXT_SIZE2];

	for (int i = 0; *szQuery != '\0'; i++)
	{
		getword(szWord, szQuery, '=');
		plustospace(szWord);
		unescape_url(szWord);

		if (!strcmp(szWord, "Xmlfile"))
		{
			getword(szWord, szQuery, '&');
			plustospace(szWord);
			unescape_url(szWord);
			sscanf(szWord, "%s", xmlfile);

			fixPath(xmlfile, 1); //tidy up path seperators etc - expect existence
		}
		else if (!strcmp(szWord, "Timestamp")) //odd order
         {
			getword(szWord, szQuery, '&');
            sscanf(szWord, "%s", timestamp);
			plustospace(timestamp);
			unescape_url(timestamp);
         }
		else if (!strcmp(szWord, "Spectrum"))
		{
			getword(szWord, szQuery, '&');
			plustospace(szWord);
			unescape_url(szWord);
			sscanf(szWord, "%s", spec);
		}
		else if (!strcmp(szWord, "Scores")) //odd order, but only way it works
		{
			getword(szWord, szQuery, '&');
			sscanf(szWord, "%s", scores);
			plustospace(scores);
			unescape_url(scores);
		}
		else if (!strcmp(szWord, "Prob"))
		{
			getword(szWord, szQuery, '&');
			plustospace(szWord);
			unescape_url(szWord);
			sscanf(szWord, "%s", prob);
		}
		else
		{
			getword(szWord, szQuery, '&');
			plustospace(szWord);
			unescape_url(szWord);
		}
	}
}
