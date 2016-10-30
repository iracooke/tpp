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



#ifndef _INCLUDED_SAXHANDLER_H_
#define _INCLUDED_SAXHANDLER_H_


#include <string>
#include <string.h> // needed for strcmp decl in GCC4.3
#include <iostream>
#include <expat.h>

using std::string;
using std::cerr;
using std::endl;



/**
 * eXpat SAX parser wrapper.
 */
class PepXSAXHandler
{
 public:
  PepXSAXHandler();
  virtual ~PepXSAXHandler();

  /**
   * Receive notification of the start of an element.
   *
   * <p>By default, do nothing.  Application writers may override this
   * method in a subclass to take specific actions at the start of
   * each element (such as allocating a new tree node or writing
   * output to a file).</p>
   */
  inline virtual void startElement(const XML_Char *el, const XML_Char **attr);

  /**
   * Receive notification of the end of an element.
   *
   * <p>By default, do nothing.  Application writers may override this
   * method in a subclass to take specific actions at the end of
   * each element (such as finalising a tree node or writing
   * output to a file).</p>
   */
  inline virtual void endElement(const XML_Char *el);

  /**
   * Receive notification of character data inside an element.
   *
   * <p>By default, do nothing.  Application writers may override this
   * method to take specific actions for each chunk of character data
   * (such as adding the data to a node or buffer, or printing it to
   * a file).</p>
   */
  //virtual void characters(const XML_Char *s, int len);

  /**
   * Open file and stream data to the SAX parser.  Must call
   * setFileName before calling this function.
   */
  void parse();
  void bufParse(const char* buffer, int len, int final);
  void jumpParse(int offset);

  void resetParser(void);


  long getByteOffset(void) {
    return XML_GetCurrentByteIndex(m_parser);
  }

  // in end element, currentByteIndex points to start of end element;
  // current byte count should contain the length of the end tag.
  long getByteEndOffset(void) {
    return 
      XML_GetCurrentByteIndex(m_parser) +
      XML_GetCurrentByteCount(m_parser);
  }


  long getCurrentLineNumber(void) {
    return XML_GetCurrentLineNumber(m_parser);
  }


  long getCurrentColumnNumber(void) {
    return XML_GetCurrentColumnNumber(m_parser);
  }


  void stopParsing(void) {
    int a=XML_StopParser(m_parser, false);
    if (a != XML_STATUS_OK) {
      cerr << "error stopping parser" << endl;
    }
  }



  inline void setFileName(const char* fileName)
  {
    m_strFileName = fileName;
  }

  // Helper functions
  inline bool isElement(const char *n1, const XML_Char *n2)
  {	return (strcmp(n1, n2) == 0); }

  inline bool isAttr(const char *n1, const XML_Char *n2)
  {	return (strcmp(n1, n2) == 0); }

  inline const char* getAttrValue(const char* name, const XML_Char **attr)
  {
    for (int i = 0; attr[i]; i += 2)
      {
	if (isAttr(name, attr[i]))
	  return attr[i + 1];
      }

    return "";
  }

 protected:
  XML_Parser m_parser;

  string  m_strFileName;
};



#endif // header guards
