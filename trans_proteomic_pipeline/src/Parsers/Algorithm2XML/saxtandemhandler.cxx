#include <iostream>

#include "saxtandemhandler.h"
#include <zlib.h> // handle gzipped input 4-10-09 bpratt

// Static callback handlers
static void startElementCallback(void *data, const XML_Char *el, const XML_Char **attr)
{
	((TANSAXHandler*) data)->startElement(el, attr);
}

static void endElementCallback(void *data, const XML_Char *el)
{
	((TANSAXHandler*) data)->endElement(el);
}

static void charactersCallback(void *data, const XML_Char *s, int len)
{
	((TANSAXHandler*) data)->characters(s, len);
}


TANSAXHandler::TANSAXHandler()
{
	m_parser = XML_ParserCreate(NULL);
	XML_SetUserData(m_parser, this);
	XML_SetElementHandler(m_parser, startElementCallback, endElementCallback);
	XML_SetCharacterDataHandler(m_parser, charactersCallback);
}


TANSAXHandler::~TANSAXHandler()
{
	XML_ParserFree(m_parser);
}


void TANSAXHandler::startElement(const XML_Char *el, const XML_Char **attr)
{
}

void TANSAXHandler::endElement(const XML_Char *el)
{
}


void TANSAXHandler::characters(const XML_Char *s, int len)
{
}

bool TANSAXHandler::parse()
{
	gzFile pfIn = gzopen(m_strFileName.data(), "rb");
	if (pfIn == NULL)
	{
		cerr << "Failed to open input file '" << m_strFileName << "'.\n";
		return false;
	}
	char buffer[8192];
	int readBytes = 0;
	bool success = true;
	while (success && (readBytes = (int) gzread(pfIn, buffer, sizeof(buffer))) > 0)
	{
		success = (XML_Parse(m_parser, buffer, readBytes, false) != 0);
	}
	success = success && (XML_Parse(m_parser, buffer, 0, true) != 0);

	gzclose(pfIn);

	if (!success)
	{
		XML_Error error = XML_GetErrorCode(m_parser);

		cerr << m_strFileName
			<< "(" << XML_GetCurrentLineNumber(m_parser) << ")"
			<< " : error " << (int) error << ": ";

		switch (error)
		{
			case XML_ERROR_SYNTAX:
			case XML_ERROR_INVALID_TOKEN:
			case XML_ERROR_UNCLOSED_TOKEN:
				cerr << "Syntax error parsing XML.";
				break;

			// TODO: Add more descriptive text for interesting errors.

			default:
				cerr << "XML Parsing error.";
				break;
		}
		cerr << "\n";

		return false;
	}

	return true;
}
