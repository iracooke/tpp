// -*- mode: c++ -*-



/*
    Program: PepXMLViewer.cgi
    Description: CGI program to display pepxml files in a web browser
    Date: July 31 2006

    Copyright (C) 2006  Natalie Tasman (original author), ISB Seattle

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

    Natalie Tasman
    Institute for Systems Biology
    401 Terry Avenue North
    Seattle, WA  98109  USA
    
    email (remove underscores):
    n_tasman at systems_biology dot org
*/



#include <stdexcept>

#include "random_access_gzFile.h"

#include "PepXUtility.h"

#include "PepXSAXHandler.h"

using namespace std;
using namespace boost;



// Static callback handlers
static void startElementCallback(void *data, const XML_Char *el, const XML_Char **attr)
{
  ((PepXSAXHandler*) data)->startElement(el, attr);
}

static void endElementCallback(void *data, const XML_Char *el)
{
  ((PepXSAXHandler*) data)->endElement(el);
}

/*
  static void charactersCallback(void *data, const XML_Char *s, int len)
  {
  ((PepXSAXHandler*) data)->characters(s, len);
  }
*/

PepXSAXHandler::PepXSAXHandler()
{
  m_parser = XML_ParserCreate(NULL);
  XML_SetUserData(m_parser, this);
  XML_SetElementHandler(m_parser, startElementCallback, endElementCallback);
  //XML_SetCharacterDataHandler(m_parser, charactersCallback);
}


void 
PepXSAXHandler::resetParser(void) {
  XML_ParserReset(m_parser,
		  "UTF-8");
  XML_SetUserData(m_parser, this);
  XML_SetElementHandler(m_parser, startElementCallback, endElementCallback);
}




PepXSAXHandler::~PepXSAXHandler()
{
  XML_ParserFree(m_parser);
}


void PepXSAXHandler::startElement(const XML_Char *el, const XML_Char **attr)
{
}

void PepXSAXHandler::endElement(const XML_Char *el)
{
}


/*
  void PepXSAXHandler::characters(const XML_Char *s, int len)
  {
  }
*/
void PepXSAXHandler::parse() {

//  XML_ParsingStatus status;
  // check if we've been stopped, first

  /*
    enum XML_Parsing {
    XML_INITIALIZED,
    XML_PARSING,
    XML_FINISHED,
    XML_SUSPENDED
    };

    typedef struct {
    enum XML_Parsing parsing;
    XML_Bool finalBuffer;
    } XML_ParsingStatus;

    XML_GetParsingStatus(m_parser,
    &status);

  */



  random_access_gzFile* pfIn = random_access_gzopen(m_strFileName.data());
  if (pfIn == NULL)
    {
      cerr << "Failed to open input file '" << m_strFileName << "'.\n";
      throw runtime_error(string("Parser failed to open input file ") + m_strFileName);
    }

  int BUF_SIZE = 8192;


  /*
  // avoid double copying
  void* buffer;
  */

  char buffer[8192];



  int readBytes = 0;
  bool success = true;
  //buffer = XML_GetBuffer(m_parser, BUF_SIZE);
  while (success && (readBytes = (int) random_access_gzread(pfIn, buffer, sizeof(buffer))) != 0)
    //while (success && (readBytes = (int) fread(buffer, 1, BUF_SIZE, pfIn)) != 0)
    {
      success = (XML_Parse(m_parser, buffer, readBytes, false) != 0);
      
      //success = (XML_ParseBuffer(m_parser, readBytes, false) != 0);
      //if (success) {buffer = XML_GetBuffer(m_parser, BUF_SIZE);}
      
    }
  
  success = success && (XML_Parse(m_parser, buffer, 0, true) != 0);
  //success = success && (XML_ParseBuffer(m_parser, readBytes, true) != 0);

  random_access_gzclose(pfIn);

  if (!success)
    {
      XML_Error error = XML_GetErrorCode(m_parser);


      // nt: I believe this is a false error, possibly from one extra call
      if (error == XML_ERROR_FINISHED) {
	return;
      }

	
      if (error == XML_ERROR_ABORTED) {
	//cerr << "caught stopped parser parse call: ok" << endl;
	return; // ok
      }



      cerr << m_strFileName
	   << "(" << XML_GetCurrentLineNumber(m_parser) << ")"
	   << " : error " << (int) error << ": ";

      switch (error)
	{
	case XML_ERROR_SYNTAX:
	case XML_ERROR_INVALID_TOKEN:
	case XML_ERROR_UNCLOSED_TOKEN:
	  cerr << "Syntax error parsing XML.";
	  cerr << XML_ErrorString(error);
	  cerr << "at l:" << XML_GetCurrentLineNumber(m_parser)
	       << ", c:" << XML_GetCurrentColumnNumber(m_parser);


	  break;

	  // TODO: Add more descriptive text for interesting errors.

	default:
	  cerr << "XML Parsing error.";
	  cerr << XML_ErrorString(error);
	  cerr << "at l:" << XML_GetCurrentLineNumber(m_parser)
	       << ", c:" << XML_GetCurrentColumnNumber(m_parser);


	  break;
	}
      cerr << "\n";

      string errStr="XML parsing error: ";
      errStr += XML_ErrorString(error);
      errStr += ", at xml file line ";
      string lineNum;
      toString(XML_GetCurrentLineNumber(m_parser), lineNum);
      errStr += lineNum;
      errStr += ", column ";
      string colNum;
      toString(XML_GetCurrentColumnNumber(m_parser), colNum);
      errStr += colNum;
      throw runtime_error(errStr);
    }

  
  
  
}





void PepXSAXHandler::bufParse(const char* buffer, int len, int final) {

  bool success = true;
  //cerr << "in bufParse: read buffer";
  //cerr << " at l:" << XML_GetCurrentLineNumber(m_parser);
  //cerr << endl << " ***" << buffer << "***" << endl << endl << endl;

  success = (XML_Parse(m_parser, buffer, len, final) != 0);


  if (!success)
    {
      XML_Error error = XML_GetErrorCode(m_parser);
      
      //cerr << "Buffer parsing; " 
      //<< "(" << XML_GetCurrentLineNumber(m_parser) << ")"
      //<< " : error " << (int) error << ": ";

      switch (error)
	{
	case XML_ERROR_SYNTAX:
	case XML_ERROR_INVALID_TOKEN:
	case XML_ERROR_UNCLOSED_TOKEN:
	  cerr << "Syntax error parsing XML.";
	  cerr << XML_ErrorString(error);
	  cerr << "at l:" << XML_GetCurrentLineNumber(m_parser)
	       << ", c:" << XML_GetCurrentColumnNumber(m_parser);



	  break;

	  // TODO: Add more descriptive text for interesting errors.

	default:
	  cerr << "XML Parsing error.";
	  cerr << XML_ErrorString(error);
	  cerr << "at l:" << XML_GetCurrentLineNumber(m_parser)
	       << ", c:" << XML_GetCurrentColumnNumber(m_parser);
	  break;
	}
      cerr << "\n";

      string errStr="XML parsing error: ";
      errStr += XML_ErrorString(error);
      errStr += ", at xml file line ";
      string lineNum;
      toString(XML_GetCurrentLineNumber(m_parser), lineNum);
      errStr += lineNum;
      errStr += ", column ";
      string colNum;
      toString(XML_GetCurrentColumnNumber(m_parser), colNum);
      errStr += colNum;
      throw runtime_error(errStr);
    }

}




void PepXSAXHandler::jumpParse(int offset) {

  random_access_gzFile* pfIn = random_access_gzopen(m_strFileName.data());
  if (pfIn == NULL) {
    cerr << "Failed to open input file '" << m_strFileName << "'.\n";
    throw runtime_error(string("Parser failed to open input file ") + m_strFileName);

  }

  random_access_gzseek(pfIn, offset, SEEK_SET);


  char buffer[8192];
  int readBytes = 0;
  bool success = true;

  // prime the pump
  const char* s = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
  success = (XML_Parse(m_parser, s, (int)strlen(s), false) != 0);
  if (!success) {
    cerr << "didn't take fake start l1" << endl;
    goto BAD;
  }
  s = "<node>\n";
  success = (XML_Parse(m_parser, s, (int)strlen(s), false) != 0);
  if (!success) {
    cerr << "didn't take fake start l2" << endl;
    goto BAD;
  }


  while (success && (readBytes = (int) random_access_gzread(pfIn, buffer, sizeof(buffer))) != 0) {
    debug(debugout << "jumparse: read " << buffer << endl << "----------" << endl); 
    success = (XML_Parse(m_parser, buffer, readBytes, false) != 0);
  }
  random_access_gzclose(pfIn);

  if (!success && 
      XML_GetErrorCode(m_parser) == XML_ERROR_ABORTED) {
    // finish off
    //cerr << "jumpparse: stoped with abort (good)" << endl;
    return; // ok
  }

  
 BAD:

  if (!success)
    {
      XML_Error error = XML_GetErrorCode(m_parser);

      if (error == XML_ERROR_ABORTED) {
	//cerr << "caught stopped parser parse call: ok" << endl;
	return; // ok
      }



      cerr << m_strFileName
	   << "(" << XML_GetCurrentLineNumber(m_parser) << ")"
	   << " : error " << (int) error << ": ";

      switch (error)
	{
	case XML_ERROR_SYNTAX:
	case XML_ERROR_INVALID_TOKEN:
	case XML_ERROR_UNCLOSED_TOKEN:
	  cerr << "Syntax error parsing XML.";
	  cerr << XML_ErrorString(error);
	  cerr << "at l:" << XML_GetCurrentLineNumber(m_parser)
	       << ", c:" << XML_GetCurrentColumnNumber(m_parser);


	  break;

	  // TODO: Add more descriptive text for interesting errors.

	default:
	  cerr << "XML Parsing error.";
	  cerr << XML_ErrorString(error);
	  cerr << "at l:" << XML_GetCurrentLineNumber(m_parser)
	       << ", c:" << XML_GetCurrentColumnNumber(m_parser);


	  break;
	}
      cerr << "\n";
      string errStr="XML parsing error: ";
      errStr += XML_ErrorString(error);
      errStr += ", at xml file line ";
      string lineNum;
      toString(XML_GetCurrentLineNumber(m_parser), lineNum);
      errStr += lineNum;
      errStr += ", column ";
      string colNum;
      toString(XML_GetCurrentColumnNumber(m_parser), colNum);
      errStr += colNum;
      throw runtime_error(errStr);
    }

  return;
}
