#ifndef SPEC_PARSER_H
#define SPEC_PARSER_H

#include <stdio.h>
#include <math.h>
#include <time.h>

#include "Parsers/Parser/Parser.h"
#include "Parsers/Parser/TagFilter.h"
#include "common/constants.h"
//#include "Quantitation/ASAPRatio/UniquePeptide/UniquePeptide.h"
#include "Quantitation/ASAPRatio/ASAP_structs.h"
#include "Parsers/Algorithm2XML/SearchResult/ProDataComponent.h"
#include "Parsers/Algorithm2XML/SearchResult/SearchResult.h"
#include "Parsers/Algorithm2XML/SearchResult/SequestResult.h"
#include "Parsers/Algorithm2XML/SearchResult/MascotResult.h"
#include "Parsers/Algorithm2XML/SearchResult/CometResult.h"
#include "Quantitation/ASAPRatio/ASAPRatio_Fns/ASAPRatio_numFns.h"
#include "Validation/MixtureDistribution/MixtureDistrFactory.h"

class SpectrumParser : public Parser {

 public:

  SpectrumParser(const char* xmlfile, const char* basename, int startscan, int endscan, int charge);
  ~SpectrumParser();
  void setFilter(Tag* tag);
  void write(ostream& os);
  Boolean found();

  //double getRatioSum();
  //double getRatioSquareSum();
  //int getRatioNum();
  //RatioStruct getRatio();
  //void setPepDataStruct(Array<Tag*>* tags, int elution, long scan, int precursor_charge);
  //proDataStrct* getProDataStruct();
  //char* display(int seq, int pk, int data);
  //int getResultIndex(int seq, int pk, int data, Boolean advance);
  //void write(ostream& os);

 protected:

  void parse(const char* xmlfile);
  void cleanup(Array<char*>* data);
  //int getTimestampIndex(char* timestamp);
  SearchResult* getSearchResult(Array<Tag*>* tags, char* engine);
  //pepDataStrct getPepDataStrct(int seq, int pk, int data);
  //void setDataRadioTag(char* tag, int seq, int pk, int data, int value);
  //void setPeakRadioTag(char* tag, int seq, int pk, int value);
  //void setSeqRadioTag(char* tag, int seq, int value);
  //void setASAPRatioTag(char* text, double mean, double error);
  //void update();
  //double PadeApprx(double x, double *xa, double *ya, int size);
  //void DixonTest(double *data, int *outliers, int size);
  //void findMeanAndStdDevWeight(double *mean, double *error,
  //			       double *data, double *weight, int size);
  //void getDataRatio(double *ratio, double *error, double confL, 
  //		    double *data, double *dataErrs, 
  //		    double *dataWghs, int *dataIndx, int dataSize,
  //		    int testType);
  //void updatePeakStrctRatio(int seq, int pk);
  //void updateSeqStrctRatio(int seq);


  int current_index_;

  int result_index_;
  int pepdata_index_;

  //  Boolean peptideListMember(char* pep, double* wt);
  //  Boolean possiblePeptideListMember(char* data);

  //  Array<UniquePeptide*>* peptides_;
  //  double min_probability_;
  //  double min_weight_;

  //  double ratio_sum_;
  //  double ratio_square_sum_;
  //  int ratio_num_;
  //  Boolean heavy2light_;
  //  proDataStrct* protein_;
  Array<char*>* xmlfiles_;

  Boolean found_;

  MixtureDistrFactory* factory_;
  //char* xmlfile_;

  Array<char*>* databases_;
  Array<char*>* basenames_;
  Array<char*>* pepproph_timestamps_;
  Array<char*>* iproph_timestamps_;
  Array<char*>* asap_timestamps_;
  Array<Boolean>* asap_quantHighBGs_;
  Array<Boolean>* asap_zeroBGs_;
  Array<double>* asap_mzBounds_;
  Array<bool>* asap_wavelets_;
  //Array<int>* elutions_;
#ifndef USE_STD_MODS
  Array<char*>* aa_modifications_;
  Array<char*>* term_modifications_;
#endif
  Array<char*>* misc_run_conditions_;


  ProDataComponent* prodata_component_;

  pepDataStrct data_;

  char* basename_;
  int start_scan_;
  int end_scan_;
  int charge_;

  ModelOptions  modOpts_;
  ScoreOptions  scoreOpts_;


};











#endif
