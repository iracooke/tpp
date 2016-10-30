/*
 Copyright (C) 2006 Ronald C Beavis, all rights reserved
 X! tandem 
 This software is a component of the X! proteomics software
 development project

Use of this software governed by the Artistic license, as reproduced here:

The Artistic License for all X! software, binaries and documentation

Preamble
The intent of this document is to state the conditions under which a
Package may be copied, such that the Copyright Holder maintains some 
semblance of artistic control over the development of the package, 
while giving the users of the package the right to use and distribute 
the Package in a more-or-less customary fashion, plus the right to 
make reasonable modifications. 

Definitions
"Package" refers to the collection of files distributed by the Copyright 
	Holder, and derivatives of that collection of files created through 
	textual modification. 

"Standard Version" refers to such a Package if it has not been modified, 
	or has been modified in accordance with the wishes of the Copyright 
	Holder as specified below. 

"Copyright Holder" is whoever is named in the copyright or copyrights 
	for the package. 

"You" is you, if you're thinking about copying or distributing this Package. 

"Reasonable copying fee" is whatever you can justify on the basis of 
	media cost, duplication charges, time of people involved, and so on. 
	(You will not be required to justify it to the Copyright Holder, but 
	only to the computing community at large as a market that must bear 
	the fee.) 

"Freely Available" means that no fee is charged for the item itself, 
	though there may be fees involved in handling the item. It also means 
	that recipients of the item may redistribute it under the same
	conditions they received it. 

1. You may make and give away verbatim copies of the source form of the 
Standard Version of this Package without restriction, provided that 
you duplicate all of the original copyright notices and associated 
disclaimers. 

2. You may apply bug fixes, portability fixes and other modifications 
derived from the Public Domain or from the Copyright Holder. A 
Package modified in such a way shall still be considered the Standard 
Version. 

3. You may otherwise modify your copy of this Package in any way, provided 
that you insert a prominent notice in each changed file stating how and 
when you changed that file, and provided that you do at least ONE of the 
following: 

a.	place your modifications in the Public Domain or otherwise make them 
	Freely Available, such as by posting said modifications to Usenet 
	or an equivalent medium, or placing the modifications on a major 
	archive site such as uunet.uu.net, or by allowing the Copyright Holder 
	to include your modifications in the Standard Version of the Package. 
b.	use the modified Package only within your corporation or organization. 
c.	rename any non-standard executables so the names do not conflict 
	with standard executables, which must also be provided, and provide 
	a separate manual page for each non-standard executable that clearly 
	documents how it differs from the Standard Version. 
d.	make other distribution arrangements with the Copyright Holder. 

4. You may distribute the programs of this Package in object code or 
executable form, provided that you do at least ONE of the following: 

a.	distribute a Standard Version of the executables and library files, 
	together with instructions (in the manual page or equivalent) on 
	where to get the Standard Version. 
b.	accompany the distribution with the machine-readable source of the 
	Package with your modifications. 
c.	give non-standard executables non-standard names, and clearly 
	document the differences in manual pages (or equivalent), together 
	with instructions on where to get the Standard Version. 
d.	make other distribution arrangements with the Copyright Holder. 

5. You may charge a reasonable copying fee for any distribution of 
this Package. You may charge any fee you choose for support of 
this Package. You may not charge a fee for this Package itself. 
However, you may distribute this Package in aggregate with other 
(possibly commercial) programs as part of a larger (possibly 
commercial) software distribution provided that you do not a
dvertise this Package as a product of your own. You may embed this 
Package's interpreter within an executable of yours (by linking); 
this shall be construed as a mere form of aggregation, provided that 
the complete Standard Version of the interpreter is so embedded. 

6. The scripts and library files supplied as input to or produced as 
output from the programs of this Package do not automatically fall 
under the copyright of this Package, but belong to whomever generated 
them, and may be sold commercially, and may be aggregated with this 
Package. If such scripts or library files are aggregated with this 
Package via the so-called "undump" or "unexec" methods of producing 
a binary executable image, then distribution of such an image shall 
neither be construed as a distribution of this Package nor shall it 
fall under the restrictions of Paragraphs 3 and 4, provided that you 
do not represent such an executable image as a Standard Version of 
this Package. 

7. C subroutines (or comparably compiled subroutines in other languages) 
supplied by you and linked into this Package in order to emulate 
subroutines and variables of the language defined by this Package 
shall not be considered part of this Package, but are the equivalent 
of input as in Paragraph 6, provided these subroutines do not change 
the language in any way that would cause it to fail the regression 
tests for the language. 

8. Aggregation of this Package with a commercial distribution is always 
permitted provided that the use of this Package is embedded; that is, 
when no overt attempt is made to make this Package's interfaces visible 
to the end user of the commercial distribution. Such use shall not be 
construed as a distribution of this Package. 

9. The name of the Copyright Holder may not be used to endorse or promote 
products derived from this software without specific prior written permission. 

10. THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED 
WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF 
MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. 

The End 
*/

// File version: 2006-02-17

/*
 * SAXSapHandler.cpp contains the override methods necessary for compatibility with
 * BIOML file formats.
 */

#include "stdafx.h"
#include "msequence.h"
#include "saxsaphandler.h"

SAXSapHandler::SAXSapHandler( )
	: SAXHandler()
{
		m_bProtein = false;
		m_bAa = false;
		m_mapSap.clear();
}

SAXSapHandler::~SAXSapHandler()
{
}

void SAXSapHandler::startElement(const XML_Char *el, const XML_Char **attr)
{
	if(isElement("protein", el)){
		m_bProtein = true;
		m_mapItem.clear();
		m_strId = getAttrValue("id", attr);
	}
	else if(isElement("aa", el)){
		char res = '\0';
		char mut = '\0';
		m_bAa = true;
		string strValue;
		strValue = getAttrValue("at",attr);
		m_pairItem.first = atoi(strValue.c_str());
		strValue = getAttrValue("mut",attr);
		mut = strValue[0];
		m_pairItem.second.m_strId = getAttrValue("id",attr);
		strValue = getAttrValue("type",attr);
		res = strValue[0];
		double dMod = atof(getAttrValue("mod",attr));
		if(mut != '\0' && res != '\0' && checkMut(res,mut) && m_pairItem.first > 0)	{
			m_pairItem.second.m_dMod = 0.0;
			m_pairItem.second.m_cRes = res;
			m_pairItem.second.m_cMut = mut;
			m_pairItem.second.m_iPos = m_pairItem.first;
			m_mapItem.insert(m_pairItem);
		}
		else if (dMod != 0.0 && mut == '\0' && res != '\0' && m_pairItem.first > 0){
			m_pairItem.second.m_dMod = dMod;
			m_pairItem.second.m_cRes = res;
			m_pairItem.second.m_cMut = res;
			m_pairItem.second.m_iPos = m_pairItem.first;
			m_mapItem.insert(m_pairItem);
		}
	}
}

void SAXSapHandler::endElement(const XML_Char *el)
{
	if(isElement("protein", el)){
		m_bProtein = false;
		pair <string,multimap <int,SavInfo> > pairValue;
		pairValue.first = m_strId;
		pairValue.second.clear();
		map<string,multimap <int,SavInfo> >::iterator itValue;
		itValue = m_mapSap.find(pairValue.first);
		if(itValue == m_mapSap.end())	{
			m_mapSap.insert(pairValue);
			itValue = m_mapSap.find(pairValue.first);
		}
		itValue = m_mapSap.find(pairValue.first);
		multimap<int,SavInfo>::iterator itMap = m_mapItem.begin();
		while(itMap != m_mapItem.end())	{
			itValue->second.insert(*itMap);
			itMap++;
		}
	}
	else if(isElement("aa", el)){
		m_bAa = false;
	}
}

bool SAXSapHandler::checkMut(char _r,char _m)	
{
	if(_r == 'K' && (_m == 'Q' || _m == 'E'))	{
		return false;
	}
	if(_r == 'Q' && (_m == 'K' || _m == 'E'))	{
		return false;
	}
	if(_r == 'E' && (_m == 'K' || _m == 'Q'))	{
		return false;
	}
	if(_r == 'N' && _m == 'D')	{
		return false;
	}
	if(_r == 'D' && _m == 'N')	{
		return false;
	}
	if(_r == 'I' && _m == 'L')	{
		return false;
	}
	if(_r == 'L' && _m == 'I')	{
		return false;
	}
	if(_r == 'F' && _m == 'M')	{
		return false;
	}
	if(_r == 'M' && _m == 'F')	{
		return false;
	}
	return true;
}
