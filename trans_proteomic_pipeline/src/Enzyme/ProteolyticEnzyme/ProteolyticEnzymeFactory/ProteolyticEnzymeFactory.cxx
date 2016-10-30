#include "ProteolyticEnzymeFactory.h"

/*

Program       : EnzymeSpecificity for PeptideProphet                                                       
Author        : Andrew Keller <akeller@systemsbiology.org>                                                       
Date          : 11.27.02 

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

#define CATALOG_NAMES_INDEX 0
#define CATALOG_INDEPENDENT_INDEX 1
#define CATALOG_DESCRIPTION_INDEX 2
#define CATALOG_SPECIFICITYRULES_INDEX 3

  const char *ProteolyticEnzymeFactory::CATALOG_ENZYMES[][4] = 
  {
    /*
      For a more complete "standard" list see:

         http://www.ebi.ac.uk/ontology-lookup/browse.do?ontName=MS&termId=MS:1001045&termName=cleavage%20agent%20name

      {<name(s)>, <indepedent>, <description>, <specificity rule(s)>}
      <name(s)> : multiple names are separated by "\n"
      <specificity rule(s)> : multiple rules are separated by ";"
                              each rules is made up of 
                              "<cut>:<no cut>:<n term sense>:[<min spacing>]"
                              where <min spacing> = -1 indicate used default
                                                    blank used factory settings
                                                    numeric value to override
    */
      {"trypsin", "true", NULL, "KR|P|false|"}                            // default & original
    , {"stricttrypsin|trypsin/p", "true", NULL, "KR||false|"}                            // added 2007-10-12, DDS
    , {"argc|arg-c|arg_c", "true", NULL, "R|P|false|"}                          // added 2007-02-07, WCH
    , {"aspn|asp-n|asp_n", "true", NULL, "D||true|"}                            // original
    , {"chymotrypsin", "true", NULL, "FLMWY|P|false|"}                     // original 
    , {"clostripain", "true", NULL, "R|-|false|"}                         // original 
    , {"cnbr", "true", NULL, "M||false|"}                                // original 
    , {"elastase", "true", NULL, "AGILV|P|false|"}                        // original 
    , {"formicacid|formic_acid", "true", NULL, "D|P|false|"}              // added 2007-02-07, WCH
    , {"gluc|glu_c|v8-de", "true", NULL, "DE|P|false|"}                               // original
    , {"gluc_bicarb|v8-e", "true", NULL, "E|P|false|"}                         // original
    , {"iodosobenzoate", "true", NULL, "W|-|false|"}                      // original
    , {"lysc|lys-c|lys_c", "true", NULL, "K|P|false|"}                          // added 2007-02-07, WCH
    , {"lysc-p|lys-c/p", "true", NULL, "K||false|"}                       // added 2007-02-07, WCH
    , {"lysn|lys_n", "true", NULL, "K||true|"}                                  // original
    , {"lysn_promisc", "true", NULL, "KR||true|"}                         // original
    , {"ralphtrypsin", "true", NULL, "KRST|P|false|"}                             // original
    , {"nonspecific|no enzyme|no cleavage", "true", NULL, ""}             // updated
    , {"pepsina", "true", NULL, "FL|-|false|"}                            // added 2007-02-07, WCH
    , {"proline_endopeptidase", "true", NULL, "P|-|false|"}               // original
    , {"trypsin/chymotrypsin", "true", NULL, "YWFMLKR|P|false|"}
    , {"staph_protease", "true", NULL, "E|-|false|"}                      // original
    , {"tca", "true", NULL, "KR|P|false|,YWFM|P|false|,D||true|-1"}       // original
    , {"trypsin/cnbr|tryp-cnbr", "true", NULL, "KR|P|false|,M|P|false|"}  // original
    , {"trypsin_gluc", "true", NULL, "DEKR|P|false|"}                     // original
    , {"trypsin_k", "true", NULL, "K|P|false|"}                           // original
    , {"trypsin_r", "true", NULL, "R|P|false|"}                           // original
    , {"thermolysin", "true", NULL, "ALIVFM|DE|true|"}                     // added 2007-08-04, WCH, [Bill Vensel]
  };


ProteolyticEnzymeFactory::ProteolyticEnzymeFactory() { 
  min_spacing_ = 1;
}

ProteolyticEnzymeFactory::ProteolyticEnzymeFactory(int min_spacing) { 
  if (min_spacing < 0) 
    min_spacing_ = 0;
  else  
    min_spacing_ = min_spacing;
}

void ProteolyticEnzymeFactory::showEnzymesCatalog() {
#if 0
  char *enzymes[] = {
      "trypsin"                  // default & original
    , "stricttrypsin"                  // default & original
    , "argc"                     // added 2007-02-07, WCH
    , "aspn"                     // original
    , "chymotrypsin"             // original 
    , "clostripain"              // original
    , "cnbr"                     // original
    , "elastase"                 // original
    , "formicacid"               // added 2007-02-07, WCH
    , "gluc"                     // original 
    , "gluc_bicarb"              // original
    , "iodosobenzoate"           // original
    , "lysc"                     // added 2007-02-07, WCH
    , "lysc-p"                   // added 2007-02-07, WCH
    , "lysn"                     // original 
    , "lysn_promisc"             // original 
    , "ralphtrypsin"             // original 
    , "trypsin/chymotrypsin"
    , "nonspecific"              // original
    , "pepsina"                  // added 2007-02-07, WCH
    , "protein_endopeptidase"    // original
    , "staph_protease"           // original
    , "tca"                      // original
    , "trypsin/cnbr"             // original
    , "trypsin_gluc"             // original
    , "trypsin_k"                // original 
    , "trypsin_r"                // original
  };
  for (int i=0; i<(sizeof(enzymes)/sizeof(char *)); i++) {
    ProteolyticEnzyme* enzyme = getProteolyticEnzyme(enzymes[i]);
    enzyme->write(cout);
    delete enzyme;
  }
#else
  for (int i=0; i<(sizeof(ProteolyticEnzymeFactory::CATALOG_ENZYMES)/sizeof(ProteolyticEnzymeFactory::CATALOG_ENZYMES[0])); i++) {
    ProteolyticEnzyme* enzyme = getEnzymeFromCatalog(i, NULL);
    enzyme->write(cout);
    delete enzyme;
  }
#endif
}

// factory for producing enzyme digestions
// register all new enzyme digestions here
// please include/remove the name(s) into char *enzymes[] of showEnzymesCatalog()
ProteolyticEnzyme* ProteolyticEnzymeFactory::getProteolyticEnzyme(const char* input_name) {
  ProteolyticEnzyme* enz = NULL;
  // go to lower case
  Boolean isSemiSpecific = False;
  char *name = NULL;
  ProteolyticEnzyme::parseEnzymeName(input_name, &name, isSemiSpecific);
  const char *fidelity = (isSemiSpecific?ProteolyticEnzyme::CONSTANT_FIDELITY_SEMISPECIFIC_:NULL);

#if 0
  if(name == NULL) {// default
    enz = new ProteolyticEnzyme("trypsin", NULL, True, NULL);
    enz->enterSpecificity("KR", "P", False, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "trypsin") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("KR", "P", False, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "ralphtrypsin") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("KRST", "P", False, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "stricttrypsin") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("KR", "", False, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "argc") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("R", "P", False, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "aspn") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("D", "", True);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "chymotrypsin") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("YWFM", "P", False, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "clostripain") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("R", "-", False, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "cnbr") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("M", "P", False, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "elastase") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("GVLIA", "P", False, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "formicacid") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("D", "P", False, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "gluc") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("DE", "P", False, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "gluc_bicarb") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("E", "P", False, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "iodosobenzoate") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("W", "-", False, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "lysc") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("K", "P", False, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "lysc-p") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("K", "", False, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "lysn") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("K", "", True, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "lysn_promisc") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("KR", "", True, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "nonspecific") == 0) {
    enz = new ProteolyticEnzyme(name, NULL, True, NULL);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "pepsina") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("FL", "-", False, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "protein_endopeptidase") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("P", "-", False, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "staph_protease") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("E", "-", False, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "tca") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("KR", "P", False, min_spacing_);
    enz->enterSpecificity("YWFM", "P", False, min_spacing_);
    enz->enterSpecificity("D", "", True);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "trypsin/cnbr") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("KR", "P", False, min_spacing_);
    enz->enterSpecificity("M", "P", False, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "trypsin_gluc") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("DEKR", "P", False, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "trypsin_k") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("K", "P", False, min_spacing_);
    enz->fixSpecificity();
  }
  else if(strcmp(name, "trypsin_r") == 0) {
    enz = new ProteolyticEnzyme(name, fidelity, True, NULL);
    enz->enterSpecificity("R", "P", False, min_spacing_);
    enz->fixSpecificity();
  }
  // enzyme not registered
#else
  if(name == NULL) {// default
    enz = getEnzymeFromCatalog("trypsin", NULL);
  } else {
    enz = getEnzymeFromCatalog(name, fidelity);
  }

#endif

  delete[] name;
  return enz;
}

ProteolyticEnzyme* ProteolyticEnzymeFactory::getEnzymeFromCatalog(const char* name, const char* fidelity) const {
  ProteolyticEnzyme* enz = NULL;

  // let's look up the entry
  for (int i=0; i<(sizeof(ProteolyticEnzymeFactory::CATALOG_ENZYMES)/sizeof(ProteolyticEnzymeFactory::CATALOG_ENZYMES[0])); i++) {
    if (isEnzymeMatched(name, CATALOG_ENZYMES[i][CATALOG_NAMES_INDEX])) {
      enz = getEnzymeFromCatalog (i, fidelity);
      break;
    }
  }

  return enz;
}

ProteolyticEnzyme* ProteolyticEnzymeFactory::getEnzymeFromCatalog(int index, const char* fidelity) const {
  if (index >= (sizeof(ProteolyticEnzymeFactory::CATALOG_ENZYMES)/sizeof(ProteolyticEnzymeFactory::CATALOG_ENZYMES[0]))) {
    return NULL;
  }

  return getProteolyticEnzyme (CATALOG_ENZYMES[index][CATALOG_NAMES_INDEX], 
    fidelity, 
    CATALOG_ENZYMES[index][CATALOG_INDEPENDENT_INDEX],
    CATALOG_ENZYMES[index][CATALOG_DESCRIPTION_INDEX],
    CATALOG_ENZYMES[index][CATALOG_SPECIFICITYRULES_INDEX]);
}

ProteolyticEnzyme* ProteolyticEnzymeFactory::getProteolyticEnzyme(const char *name, 
  const char* fidelity, const char* independent, const char *description, 
  const char *specificities) const {

  // {"trypsin", "True", NULL, "KR:P:False:"}
  string standardName = getEnzymeStandardName(name);
  ProteolyticEnzyme* enz = new ProteolyticEnzyme(standardName.c_str(), fidelity, 
    strcmp("true", independent) ? False : True, description);

  // load the specificity rules
  bool endOfRules=false;
  string rules(specificities);
  do {
    //"KR|P|false|,YWFM|P|false|,D||true|-1"

    string rule;
    size_t separatorPos = rules.find(',');
    endOfRules = (separatorPos == rules.npos);

    if (endOfRules) {
      rule = rules;
    } else {
      rule = rules.substr(0, separatorPos);
      rules = rules.substr(separatorPos+1);
    }

    string cut;
    separatorPos = rule.find('|');
    if (separatorPos==rule.npos) {
      cut = rule;
    } else {
      cut = rule.substr(0, separatorPos);
      rule = rule.substr(separatorPos+1);
    }

    string nocut;
    separatorPos = rule.find('|');
    if (separatorPos==rule.npos) {
      nocut = rule;
    } else {
      nocut = rule.substr(0, separatorPos);
      rule = rule.substr(separatorPos+1);
    }

    string value;
    separatorPos = rule.find('|');
    if (separatorPos==rule.npos) {
      value = rule;
    } else {
      value = rule.substr(0, separatorPos);
      rule = rule.substr(separatorPos+1);
    }
    Boolean n_sense = strcmp("true", value.c_str()) ? False : True;

    separatorPos = rule.find('|');
    if (separatorPos==rule.npos) {
      value = rule;
    } else {
      value = rule.substr(0, separatorPos);
      rule = rule.substr(separatorPos+1);
    }

    if (value == "-1") {
      enz->enterSpecificity(cut.c_str(), nocut.c_str(), n_sense);
    } else {
      if (value == "") {
        enz->enterSpecificity(cut.c_str(), nocut.c_str(), n_sense, min_spacing_);
      } else {
        enz->enterSpecificity(cut.c_str(), nocut.c_str(), n_sense, atoi(value.c_str()));
      }
    }
  } while (!endOfRules);
  enz->fixSpecificity();

  return enz;
}

string ProteolyticEnzymeFactory::getEnzymeStandardName(const char *names) const {
  string standardName(names);
  size_t separatorPos = standardName.find('|');
  if (separatorPos != standardName.npos) {
    standardName = standardName.substr(0, separatorPos);
  }

  return standardName;
}

bool ProteolyticEnzymeFactory::isEnzymeMatched(const char *findName, const char *names) const {
  bool nameFound=false;
  bool endOfList=false;
  string nameList(names);
  do {
    string singleName;
    size_t separatorPos = nameList.find('|');
    endOfList = (separatorPos == nameList.npos);

    if (endOfList) {
      singleName = nameList;
    } else {
      singleName = nameList.substr(0, separatorPos);
      nameList = nameList.substr(separatorPos+1);
    }
    nameFound = (singleName==findName);
  } while (!nameFound && !endOfList);

  return nameFound;
}

