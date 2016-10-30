// Tandem2xml.cxx
//     Main entry point for converting X!Tandem output to
//     pepXML.

#include <iostream>

using namespace std;

#include "Tandem2xml.h"
#include "TandemResultsParser.h"
#include "Parsers/Algorithm2XML/masscalc.h"
#include "common/constants.h"

#include "common/TPPVersion.h" // contains version number, name, revision

int main(int argc, char* argv[])
{
  hooks_tpp handler(argc,argv); // set up install paths etc

  if (argc < 2 || argc > 3)
    {
      cerr << " " << argv[0] << "(" << szTPPVersionInfo << ")" << endl;
      cerr << "Usage: " << argv[0] << " <input-file> [<output-file>]\n";
      exit(EXIT_FAILURE);
    }

  TandemResultsParser results;
  results.setFileName(argv[1]);
  if (argc == 3)
    results.setOutputFile(argv[2]);
  
  if (!results.writePepXML())
    exit(EXIT_FAILURE);

  return 0;
}

// Output helpers
string indentChars("   ");
string indentNL("\n");
string& nl()
{return indentNL; }
string& nlIn()
{
  indentNL.append(indentChars);
  return indentNL;
}
string& nlOut()
{
  int lenCur = (int) indentNL.size();
  int lenIndent = (int) indentChars.size();
  if (lenCur > lenIndent)
    indentNL.erase(lenCur - lenIndent);
  return indentNL;
}

/*
string XMLEscape(const string& s)
{
	string ret;
    for (size_t i = 0; i < s.length(); i++)
	{
        char ch = s.at(i);
        switch (ch)
        {
        case '<':
			ret.append("&lt;");
            break;
        case '>':
			ret.append("&gt;");
            break;
        case '&':
            ret.append("&amp;");
            break;
        case '"':
			ret.append("&quot;");
            break;
        case '\'':
            ret.append("&apos;");
            break;
        case '\n':
			ret.append(" ");
            break;
        case '\r':
			ret.append(" ");
            if (i+1 < s.length() && s.at(i+1) == '\n')
				i++;
            break;
        default:
			ret.append(1, ch);
            break;
        }
	}
	return ret;
}
*/
