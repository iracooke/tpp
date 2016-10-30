/*

Program       : Pep3D
Author        : Xiao-jun Li <xli@systemsbiology.org>                                                      
Date          : 10.08.02 

Header file for Pep3D.

Copyright (C) 2002 Xiao-jun Li

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

Xiao-jun Li
Institute for Systems Biology
401 Terry Avenue North 
Seattle, WA  98109  USA
xli@systemsbiology.org

*/
/*
  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
*/
#ifndef _PEP3D_FUNCTIONS_H_
#define _PEP3D_FUNCTIONS_H_


#include "mzParser.h"
#include "Quantitation/ASAPRatio/ASAPRatio_Fns/ASAPRatio_numFns.h"
/************************************************************************/
/*
  Structures
*/
/************************************************************************/

// field structure in .html
typedef struct {
  char * name;
  char * value;
} htmlFieldStrct;

/*
// structure of spectrum
typedef struct {
  int size;
  double * xval;
  double * yval;
} spectStrct;
*/

/************************************************************************/
/*
  Functions
*/
/************************************************************************/

///////////////////////////////////////////////////////////////////////
/*
  This function gets the queryString passed by CGI.
*/
char *getQueryString(void);

///////////////////////////////////////////////////////////////////////
/*
  This function gets a field value, specified by its corresponding name,
  from a queryString.
*/
char *getHtmlFieldValue(const char *fieldName, const char *queryString);

////////////////////////////////////////////////////////////////////////////
/*
  This function removes any html tag.
*/
void rmHtmlTag(char *string);

///////////////////////////////////////////////////////////////////////
/*
  This function gets rid of any space at the beginning or end of a string.
*/
void getRidOfSpace(char *string);

///////////////////////////////////////////////////////////////////////
/*
  This function extracts a segment from a string between "startTag[]"
  and "endTag[]". The "preTag[]" is on the upstream of and used to
  specify without ambiguity "startTag[]".
  Tags are excluded from the segment. 
*/
void getSegment(const char * string, const char * preTag, const char * startTag,  
		const char * endTag, char * segment);

////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
  This function gets the cid spectrum from .mzXML file.
*/
void getCidSpect(double *mz, double *et, spectStrct *cidSpect, char *xmlFile, int scanNum[2]);

///////////////////////////////////////////////////////////////////////////


#endif /* _PEP3D_FUNCTIONS_H_ */
/*
  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
*/

