/*

Program       : ASAPCGIDisplayParser                                                       
Author        : Andrew Keller <akeller@systemsbiology.org> 
                Xiao-jun Li (xli@systemsbiology.org>                                                      
Date          : 11.27.02 

Displays ASAPRatio protein information from ProteinProphet XML

Copyright (C) 2003 Andrew Keller, Xiao-jun Li

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


#include "ASAPCGIDisplayParser.h"
#include "pwiz/utility/misc/random_access_compressed_ifstream.hpp" // for reading gzipped files with efficient seeks

// lookup by name:
ASAPCGIDisplayParser::ASAPCGIDisplayParser(const char* xmlfile, const char* protein) : Parser(NULL) { 
  heavy2light_ = False;
  protein_ = new char[strlen(protein)+1];
  strcpy(protein_, protein);
  inputfiles_ = NULL;
  inputfiles_array_ = NULL;
  pro_ratio_ = NULL;
  init(xmlfile);
}
// lookup by name, alos store index
ASAPCGIDisplayParser::ASAPCGIDisplayParser(const char* xmlfile, const char* protein, unsigned int index) : Parser(NULL) { 
  heavy2light_ = False;
  protein_ = new char[strlen(protein)+1];
  strcpy(protein_, protein);
  inputfiles_ = NULL;
  inputfiles_array_ = NULL;
  pro_ratio_ = NULL;
  index_ = index;
  init(xmlfile);
}
// lookup by index:
ASAPCGIDisplayParser::ASAPCGIDisplayParser(const char* xmlfile, unsigned int index) : Parser(NULL) {
  heavy2light_ = False;
  protein_ = NULL;
  inputfiles_ = NULL;
  inputfiles_array_ = NULL;
  pro_ratio_ = NULL;
  index_ = index;
  init(xmlfile);
}

void ASAPCGIDisplayParser::parse(const char* xmlfile) {
  //open file and pass along
  //  int line_width = 10000;
  char *nextline = new char[line_width_];
  char* data = NULL;
  Tag* tag;
  Boolean heavy2light = False;

  Boolean filterParams = False;
  Boolean quantHighBG = False;
  Boolean zeroBG = False;
  double mzBound = 0.5;
  bool wavelet = false;
  norm_ = NULL;
  pwiz::util::random_access_compressed_ifstream fin(xmlfile); // can read gzipped xml
  if(! fin) {
    cout << "ASAPCGIDisplayParser: error opening " << xmlfile << endl;
    exit(1);
  }


  //cout << "looking for " << protein_ << " in " << xmlfile_ << endl;

  Array<Tag*>* tags = NULL;

  TagFilter* summary_filter = new TagFilter("ASAP_analysis_summary", 1);
  TagFilter* ratio_filter = new TagFilter("ASAPCGIDisplay");

  Array<char*>* inputfiles = new Array<char*>;

  // construct a tmpfile name based on xmlfile
  std::string outfile = make_tmpfile_name(xmlfile);
  //cerr << "writing data to " << outfile << endl;
  ofstream fout(outfile.c_str());
  if(! fout) {
    cout << "cannot write output to file " << outfile << endl;
    exit(1);
  }
  pval_link_ = NULL;
  while(fin.getline(nextline, line_width_)) {

    data = strstr(nextline, "<");
    while(data != NULL) {
      tag = new Tag(data);
      
      if (tag->isStart() && ! strcmp(tag->getName(), "analysis_summary") 
	 && ! strcmp(tag->getAttributeValue("analysis"), "asapratio")) {
	filterParams = True;
      }

      if (filterParams && tag->isEnd() && ! strcmp(tag->getName(), "analysis_summary")) {
	filterParams = False;
      }
      if (filterParams && ! strcmp(tag->getName(), "parameter")) {
	if (! strcmp(tag->getAttributeValue("name"), "quantHighBG") && 
	    ! strcmp(tag->getAttributeValue("value"), "True")) {
	  quantHighBG = True;
	}
	else if (! strcmp(tag->getAttributeValue("name"), "zeroBG") && 
	    ! strcmp(tag->getAttributeValue("value"), "True")) {
	  zeroBG = True;
	}
	else if (! strcmp(tag->getAttributeValue("name"), "wavelet") && 
	    ! strcmp(tag->getAttributeValue("value"), "True")) {
	  wavelet = true;
	}
	else if (! strcmp(tag->getAttributeValue("name"), "mzBound")) {
	  mzBound = atof(tag->getAttributeValue("value"));
	  if (mzBound <= 0 || mzBound >= 1) {
	    mzBound = 0.5;
	  }
	}
	
      }

      if(! strcmp(tag->getName(), "ASAP_pvalue_analysis_summary") && tag->isStart() ) {
	  // get the parameters
	  pValueStrct params;
	  params.mean = atof(tag->getAttributeValue("background_ratio_mean"));
	  params.stddev = atof(tag->getAttributeValue("background_ratio_stdev"));
	  params.merr = atof(tag->getAttributeValue("background_fitting_error"));
	  const char* tmp = tag->getAttributeValue("analysis_distribution_file");
	  pval_link_ = new char[strlen(tmp)+1];
	  sprintf(pval_link_, "%s", tmp);
	  norm_ = new Normalization(params);
      }
      
      setFilter(tag);
      
      //tag->write(cout);

      // parse input file names (and infer peptide bof names accordingly)
      if(tag->isStart() && ! strcmp(tag->getName(), "protein_summary_header")) {
	const char* files = tag->getAttributeValue("source_files_alt");
	// parse through
	int i = 0;
	int last_i = 0;
	char* nextfile;
	while(files[i]) {
	  i++;
	  if(files[i] == '+' || !files[i]) {
	    nextfile = new char[i - last_i + 1];
	    for(int z = last_i; z < i; z++)
	      nextfile[z-last_i] = files[z];
	    nextfile[i - last_i] = 0;
	    inputfiles->insertAtEnd(nextfile);
	    last_i = i+1;
	  }
	} // while

	// now call to set
////////////////////////////////////////////////////
	inputfiles_ = new char* [inputfiles->length()+1];
	for(int k = 0; k < inputfiles->length(); k++)
	  inputfiles_[k] = (*inputfiles)[k];
	inputfiles_[inputfiles->length()] = strdup("");

	inputfiles_array_ = inputfiles;
	//for(int z = 0; z < inputfiles_array_->length(); z++)
	//  cout << "here: " << (*inputfiles_array_)[z] << endl;


////////////////////////////////////////////////////

      } // if protein summary header

      if(filter_) { 
	//tag->write(cout);
	  if(!protein_ && !strcmp(tag->getName(), "protein")) {
		protein_ = strdup(tag->getAttributeValue("protein_name"));
	  }
	if(! strcmp(tag->getName(), "ASAPRatio")) {
	  if(tag->isStart()) {
	    tags = new Array<Tag*>;
	    tags->insertAtEnd(tag);
	  }
	  else { // end tag
	    if(tags == NULL) {
	      cerr << "null tags" << endl;
	      exit(1);
	    }
	    //cout << "ready to get struct" << endl;
	    pro_ratio_ = getProDataStruct(tags);
	    if(pro_ratio_ == NULL)
	      cout << "NULL PRORATIO" << endl;
	    //cout << "dfone" << endl;

	    for(int k = 0; k < tags->length(); k++)
	      if((*tags)[k] != NULL)
		delete (*tags)[k];

	    if(tags != NULL) {
	      delete tags;
	      tags = NULL;
	    }

	    // print out HTML information.....
	    //writeProteinRatio(cout, pro_ratio_);
	    
	    fin.close();
	    return;
	  }
	} // if start of ratio
	else {
	  if(tags != NULL && tag->isStart())
	    tags->insertAtEnd(tag);
	  else delete tag;
	}
      } // if filter and ready....
      else {
	delete tag;
      }
      data = strstr(data+1, "<");
    }
  }

  fin.close();
  delete[] nextline;
}

proDataStrct* ASAPCGIDisplayParser::getProDataStrct() {
  return pro_ratio_;
}

Normalization* ASAPCGIDisplayParser::getNormalized() {
  return norm_;
}
char* ASAPCGIDisplayParser::getPvalLink() {
  return pval_link_;
}
Array<char*>* ASAPCGIDisplayParser::getInputFilesArray() {
  return inputfiles_array_;
}

char** ASAPCGIDisplayParser::getInputFiles() {
  return inputfiles_;
}

// from beginning to end of each protein
void ASAPCGIDisplayParser::setFilter(Tag* tag) {
  if(tag == NULL)
    return;

  if(filter_memory_) {
    filter_memory_ = False;
    filter_ = False;
   }

  const char* stro;
  
  if(protein_)	//lookup by protein name
  {
   if(tag->isStart() && ! strcmp(tag->getName(), "protein") && ! strcmp(tag->getAttributeValue("protein_name"), protein_)) {
     filter_ = True;
     //cout << "found it!" << endl;
   }
   else if(filter_ && ! strcmp(tag->getName(), "protein") && ! tag->isStart())
     filter_memory_ = True;
  }
  else			//lookup by index only if not found by name, since looking by index doesn't work for multi protein groups
  {
	if(tag->isStart() && ! strcmp(tag->getName(), "protein_group") && ( stro = tag->getAttributeValue("group_number")))
	{
		if((unsigned)atoi(stro) == index_)
		{
		  filter_ = True;
		  //cout << "found it!" << endl;
		}
		else if(filter_)
			filter_memory_ = True;
	}
  }

 }

 /*
   This function frees a proDataStrct.
 */
 void ASAPCGIDisplayParser::freeProDataStrct(proDataStrct* data)
 {
   int i, j;

   // sequences
   for (i = 0; i < data->dataNum; ++i) {
     // peaks
     for (j = 0; j < data->sequences[i].dataNum; ++j) {
       free(data->sequences[i].peaks[j].dataIndx);
       free(data->sequences[i].peaks[j].dataCnts);
     }
     free(data->sequences[i].peaks);

     // dataCnts
     free(data->sequences[i].dataCnts);
   } // for (i = 0; i < data.dataNum; ++i) {
   free(data->sequences);

   // dataCnts
   free(data->dataCnts);

   return;
 }

 char* ASAPCGIDisplayParser::getBofFile(char* interactfile) {
   char file_pre[] = "ASAPCGIDisplay";
   char file_suf[] = ".bof";
   char interact_pre[] = "interact";
   char* result = strstr(interactfile, interact_pre);
   if(result == NULL || strlen(result) != strlen(interactfile)) {
     cerr << "illegal interact file: " << interactfile << endl;
     exit(1);
   }
   result = strstr(interactfile, "-data.htm");
   char* output = NULL;
   if(result != NULL) {
     output = new char[strlen(file_pre) + strlen(file_suf) + strlen(interactfile) - strlen(interact_pre) - strlen(result) + 1];
     strcpy(output, file_pre);
     if(strlen(interactfile) - strlen(interact_pre) - strlen(result) > 1) {
       strcat(output, "_");
       strncat(output, interactfile + strlen(interact_pre) + 1, strlen(interactfile) - strlen(interact_pre) - strlen(result) - 1);
       output[strlen(file_pre) + strlen(interactfile) - strlen(interact_pre) - strlen(result) + 1] = 0;
     }
     strcat(output, file_suf);
     return output;
   }
   cerr << "could not parse interactfile " << interactfile << endl;
   exit(1);
 }


 proDataStrct* ASAPCGIDisplayParser::getProDataStruct(Array<Tag*>* tags) {
   //for(int k = 0; k < tags->length(); k++)
   // (*tags)[k]->write(cout);


  if(tags == NULL || tags->length() < 1) {
    //cout << "error in getProDataStruct" << endl;
    //exit(1);
    return NULL;
  }
  proDataStrct* data = (proDataStrct *)calloc(1, sizeof(proDataStrct));


  // first one should be ASAPCGIDisplay
  int index = 0;

  if(! (*tags)[index]->isStart() || strcmp((*tags)[index]->getName(), "ASAPRatio")) {
    cout << "first tag is not ASAPRatio" << endl;
    exit(1);
  }
  //(*tags)[index]->write(cout);
  data->ratio[0] = (double)atof((*tags)[index]->getAttributeValue("ratio_mean"));
  data->inv_ratio[0] = (double)atof((*tags)[index]->getAttributeValue("heavy2light_ratio_mean"));
  
  if(data->ratio[0] == -1.0)
    data->ratio[0] = -2.0;
  else if(data->ratio[0] >= 9999.0)
    data->ratio[0] = -1.0;

  if(data->inv_ratio[0] == -1.0)
    data->inv_ratio[0] = -2.0;
  else if(data->ratio[0] >= 9999.0)
    data->inv_ratio[0] = -1.0;
  

  data->ratio[1] = (double)atof((*tags)[index]->getAttributeValue("ratio_standard_dev"));
  data->inv_ratio[1] = (double)atof((*tags)[index]->getAttributeValue("heavy2light_ratio_standard_dev"));

  data->dataNum = atoi((*tags)[index]->getAttributeValue("ratio_number_peptides"));
  data->indx = atoi((*tags)[index]->getAttributeValue("status"));
  // sequences
  data->sequences = (seqDataStrct *)
    calloc(data->dataNum, sizeof(seqDataStrct));
  // dataCnts
  data->dataCnts = (int *) calloc(data->dataNum, sizeof(int));
  index++;


  // now go through each seq
  for(int k = 0; k < data->dataNum; k++) {
    //    cout << "working on seq " << (k+1) << " of " << data->dataNum << endl;
    index = setProSeqStruct(data, k, tags, index);
  } //  next seq
  if(index != (int)tags->length()) {
    cerr << "error, " << index << " != " << tags->length() << endl;
    exit(1);
  }
  return data;
}

int ASAPCGIDisplayParser::setProSeqStruct(proDataStrct* data, int seq_ind, Array<Tag*>* tags, int tag_ind) {
  // first one should be ASAPCGIDisplay
  int index = tag_ind;

  if(strcmp((*tags)[index]->getName(), "ASAP_Seq")) {
    cout << "first tag is not ASAP_Seq" << endl;
    (*tags)[tag_ind]->write(cerr);
    exit(1);
  }
  seqDataStrct* seq = data->sequences + seq_ind;
  seq->ratio[0] = (double)atof((*tags)[index]->getAttributeValue("ratio_mean"));
  seq->inv_ratio[0] = (double)atof((*tags)[index]->getAttributeValue("heavy2light_ratio_mean"));
  // revert changes
  seq->ratio[1] = (double)atof((*tags)[index]->getAttributeValue("ratio_standard_dev"));
  seq->inv_ratio[1] = (double)atof((*tags)[index]->getAttributeValue("heavy2light_ratio_standard_dev"));
  seq->dataNum = atoi((*tags)[index]->getAttributeValue("datanum"));
  seq->indx = atoi((*tags)[index]->getAttributeValue("status"));
  seq->weight = atof((*tags)[index]->getAttributeValue("weight"));
  //cout << "added wt " << seq->weight << " to seq  strcut" << endl;
  if((*tags)[index]->getAttributeValue("light_sequence") != NULL) {
    strcpy(seq->lightSeq, (*tags)[index]->getAttributeValue("light_sequence"));
    //cout << "added seq " << seq->lightSeq << " to seq  strcut" << endl;
  }

  data->dataCnts[seq_ind] = atoi((*tags)[index]->getAttributeValue("include"));
  // peaks
  seq->peaks = (dataStrct *)
    calloc(seq->dataNum, sizeof(dataStrct));
  // dataCnts
  seq->dataCnts = (int *) calloc(seq->dataNum, sizeof(int));
  index++;
  // now go through each peak
  for(int k = 0; k < seq->dataNum; k++) {
    //cout << "working on peak " << (k+1) << " of " << seq->dataNum << endl;
    index = setProPeakStruct(seq, k, tags, index);
  } //  next seq
  //cout << "index: " << index << endl;
  return index;
}


int ASAPCGIDisplayParser::setProPeakStruct(seqDataStrct* seq, int peak_ind, Array<Tag*>* tags, int tag_ind) {
  // first one should be ASAPCGIDisplay
  int index = tag_ind;

  if(strcmp((*tags)[index]->getName(), "ASAP_Peak")) {
    cout << "first tag is not ASAP_Seq for index " << index << endl;
    (*tags)[tag_ind]->write(cerr);
    exit(1);
  }
  //cout << "peak name: " << (*tags)[index]->getName() << "<p>" << endl;
  //cout << "Peak num: " << peak_ind << " of tot: " << seq->dataNum << "<p>" << endl;

  dataStrct* peak = seq->peaks + peak_ind;
  peak->ratio[0] = (double)atof((*tags)[index]->getAttributeValue("ratio_mean"));
  peak->inv_ratio[0] = (double)atof((*tags)[index]->getAttributeValue("heavy2light_ratio_mean"));
  //cout << "done1" << endl;
  peak->ratio[1] = (double)atof((*tags)[index]->getAttributeValue("ratio_standard_dev"));
  peak->inv_ratio[1] = (double)atof((*tags)[index]->getAttributeValue("heavy2light_ratio_standard_dev"));
  //cout << "done2" << endl;
  peak->dataNum = atoi((*tags)[index]->getAttributeValue("datanum"));
  peak->bofIndx = atoi((*tags)[index]->getAttributeValue("peptide_binary_ind"));
  
  //DDS:
  //peak->msms_run_idx = atoi((*tags)[index]->getAttributeValue("peptide_binary_ind"));

  peak->indx = atoi((*tags)[index]->getAttributeValue("status"));
  //cout << "done3" << endl;
  peak->weight = (double)atof((*tags)[index]->getAttributeValue("weight"));
  //cout << "done3.1" << endl;
  seq->dataCnts[peak_ind] = atoi((*tags)[index]->getAttributeValue("include"));
  //cout << "done3.2" << endl;
  // peaks
  peak->dataIndx = (int *)calloc(peak->dataNum, sizeof(int));
  //cout << "done3.3" << endl;
  // dataCnts
  peak->dataCnts = (int *) calloc(peak->dataNum, sizeof(int));
  //cout << "done4" << endl;
  index++;
  // now go through each dta
  for(int k = 0; k < peak->dataNum; k++) {
    //cout << "wording on data " << (k+1) << " out of " << peak->dataNum << endl;
    index = setProDtaStruct(peak, k, tags, index);
  } //  next seq
  //cout << "index: " << index << endl;
  return index;
}

int ASAPCGIDisplayParser::setProDtaStruct(dataStrct* peak, int dta_ind, Array<Tag*>* tags, int tag_ind) {
  // first one should be ASAPCGIDisplay
  int index = tag_ind;
  //cout << "IIIIIIII<p>" << endl;
  if(strcmp((*tags)[index]->getName(), "ASAP_Dta")) {
    //cout << "first tag is not ASAP_Seq for index " << index << endl;
    (*tags)[tag_ind]->write(cerr);
    exit(1);
  }
  if(index >= (int)tags->length()) {
    cout << "error: tags length " << tags->length() << " less than index " << index << endl;
    exit(1);
  }

  //cout << (*tags)[index]->getName() << endl;

  peak->dataCnts[dta_ind] = atoi((*tags)[index]->getAttributeValue("include"));
  peak->dataIndx[dta_ind] = atoi((*tags)[index]->getAttributeValue("peptide_index"));
  index++; // done

  //cout << "done!" << endl;
  return index;
}

void ASAPCGIDisplayParser::writeProteinRatio(ostream& os, proDataStrct* pro_ratio) {
  Tag* protag = new Tag("ASAPRatio", True, False);
  char next[50];
  
  double ratiomean = (double)pro_ratio->ratio[0];
  double ratio_inv_mean = (double)pro_ratio->inv_ratio[0];

  if(ratiomean == -2.0) {
    sprintf(next, "%0.2f", -1.0);
    protag->setAttributeValue("ratio_mean", next);
    protag->setAttributeValue("heavy2light_ratio_mean", next);
  }
  else if(ratiomean == -1.0) {
    strcpy(next, "999.");
    protag->setAttributeValue("ratio_mean", next);
    strcpy(next, "0.");
    protag->setAttributeValue("heavy2light_ratio_mean", next);

  }
  else {
    sprintf(next, "%0.2f", ratiomean);
    protag->setAttributeValue("ratio_mean", next);
    sprintf(next, "%0.2f", ratio_inv_mean);
    protag->setAttributeValue("heavy2light_ratio_mean", next);

  }
  //sprintf(next, "%0.2f", (double)pro_ratio->ratio[0]);
 
  sprintf(next, "%0.2f", (double)pro_ratio->ratio[1]);
  protag->setAttributeValue("ratio_standard_dev", next);
  sprintf(next, "%0.2f", (double)pro_ratio->inv_ratio[1]);
  protag->setAttributeValue("heavy2light_ratio_standard_dev", next);
  sprintf(next, "%d", pro_ratio->dataNum);
  protag->setAttributeValue("ratio_number_peptides", next);
  sprintf(next, "%d", pro_ratio->indx);
  protag->setAttributeValue("status", next);
  protag->write(os);
  // now the protein stuff
  delete protag;
  for(int k = 0; k < pro_ratio->dataNum; k++) {
    Tag* seqtag = new Tag("ASAP_Seq", True, False);
    sprintf(next, "%d", pro_ratio->sequences[k].indx);
    seqtag->setAttributeValue("status", next);
    sprintf(next, "%d", pro_ratio->dataCnts[k]);
    seqtag->setAttributeValue("include", next);
    sprintf(next, "%d", pro_ratio->dataNum);
    seqtag->setAttributeValue("datanum", next);
    sprintf(next, "%0.4f", (double)pro_ratio->sequences[k].ratio[0]);
    seqtag->setAttributeValue("ratio_mean", next);
    sprintf(next, "%0.4f", (double)pro_ratio->sequences[k].ratio[1]);
    seqtag->setAttributeValue("ratio_standard_dev", next);
    sprintf(next, "%0.4f", (double)pro_ratio->sequences[k].inv_ratio[0]);
    seqtag->setAttributeValue("heavy2light_ratio_mean", next);
    sprintf(next, "%0.4f", (double)pro_ratio->sequences[k].inv_ratio[1]);
    seqtag->setAttributeValue("heavy2light_ratio_standard_dev", next);
    sprintf(next, "%0.4f", pro_ratio->sequences[k].weight);
    seqtag->setAttributeValue("weight", next);
    seqtag->setAttributeValue("light_sequence", pro_ratio->sequences[k].lightSeq);
    seqtag->write(os);
    delete seqtag;

    pro_ratio->sequences[k].sort_for_output(); // place in a logical output order
    for(int j = 0; j < pro_ratio->sequences[k].dataNum; j++) {
      Tag* peaktag = new Tag("ASAP_Peak", True, False);
      sprintf(next, "%d", pro_ratio->sequences[k].peaks[j].indx);
      peaktag->setAttributeValue("status", next);
      sprintf(next, "%d", pro_ratio->sequences[k].dataCnts[j]);
      peaktag->setAttributeValue("include", next);
      sprintf(next, "%d", pro_ratio->sequences[k].dataNum);
      peaktag->setAttributeValue("datanum", next);
      sprintf(next, "%0.4f", (double)pro_ratio->sequences[k].peaks[j].ratio[0]);
      peaktag->setAttributeValue("ratio_mean", next);
      sprintf(next, "%0.4f", (double)pro_ratio->sequences[k].peaks[j].ratio[1]);
      peaktag->setAttributeValue("heavy2light_ratio_standard_dev", next);
      sprintf(next, "%0.4f", (double)pro_ratio->sequences[k].peaks[j].inv_ratio[0]);
      peaktag->setAttributeValue("heavy2light_ratio_mean", next);
      sprintf(next, "%0.4f", (double)pro_ratio->sequences[k].peaks[j].inv_ratio[1]);
      peaktag->setAttributeValue("ratio_standard_dev", next);
      sprintf(next, "%0.1f", (double)pro_ratio->sequences[k].peaks[j].weight);
      peaktag->setAttributeValue("weight", next);
      
      //DDS:
      //sprintf(next, "%d", pro_ratio->sequences[k].peaks[j].msms_run_idx);
      sprintf(next, "%d", pro_ratio->sequences[k].peaks[j].bofIndx);
      peaktag->setAttributeValue("peptide_binary_ind", next);

      peaktag->write(os);
      delete peaktag;

      for(int i = 0; i < pro_ratio->sequences[k].peaks[j].dataNum; i++) {
	Tag* dtatag = new Tag("ASAP_Dta", True, True);
	sprintf(next, "%d", pro_ratio->sequences[k].peaks[j].dataIndx[i]);
	seqtag->setAttributeValue("peptide_index", next);
	sprintf(next, "%d", pro_ratio->sequences[k].peaks[j].dataCnts[i]);
	seqtag->setAttributeValue("include", next);

	dtatag->write(os);
	delete dtatag;
      } // next dta

      // close it
      peaktag = new Tag("ASAP_Peak", False, True);
      peaktag->write(os);
      delete peaktag;
    } // next peak

    // close it
    seqtag = new Tag("ASAP_Seq", False, True);
    seqtag->write(os);
    delete seqtag;

  } // next seq

  protag = new Tag("ASAPRatio", False, True);
  protag->write(os);
  delete protag;
}

