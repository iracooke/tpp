/*

Program       : MascotScoreParser                                                       
Author        : Andrew Keller <akeller@systemsbiology.org>                                                       
Date          : 11.27.02 

Primary data object holding all mixture distributions for each precursor ion charge

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

#include "MascotScoreParser.h"


MascotScoreParser::MascotScoreParser(char* xmlfile, int charge, MascotStarOption star) : Parser(NULL) {
  // default settings

  int min_adj_ionscores[] = { 25, 25, 25 };
  regression_complete_ = False;
  star_ = star;
  charge_ = charge;

  adj_ionscore_tot_ = 0;
  identity_tot_ = 0;
  identity_mean_ = 0.0;
  adj_ionscore_identity_mean_ = 0.0;
  adj_ionscore_identity_meansq_ = 0.0;
  adj_ionscore_identity_stdev_ = 0.0;
  adj_ionscore_homology_mean_ = 0.0;
  adj_ionscore_homology_meansq_ = 0.0;
  adj_ionscore_homology_stdev_ = 0.0;
  adj_ionscore_corr_ = 0.0;

  slope_ = 0.0;
  intercept_ = 0.0;

  num_stdevs_ = 1;
  min_adj_ionscore_ = min_adj_ionscores[charge_-1];

  init(xmlfile);
}

void MascotScoreParser::parse(const char* xmlfile) {
  char* engine = NULL;
  char* enzyme = NULL;
  char* massspec = NULL;
  Array<Tag*>* tags = NULL;
  Tag* tag = NULL;

  char* data = NULL;

  Boolean done = False;
  Boolean replace = False;

  char match[1000];
  sprintf(match, "assumed_charge=\"%d\"", charge_);

  //cout << "match: " << match << endl;

  Boolean analyze = False;

  double ionscore = -1.0;
  double homologyscore = -1.0;
  double identityscore = -1.0;
  Boolean star = True;
  
  pwiz::util::random_access_compressed_ifstream fin(xmlfile); // can read gzipped xml
  if(! fin) {
    cout << "error opening " << xmlfile << endl;
    exit(1);
  }
  char *nextline = new char[line_width_];
  while(! done && fin.getline(nextline, line_width_)) {

    if(analyze || strstr(nextline, match) != NULL) { // not yet done

      analyze = True;

      //cout << "next: " << nextline << endl;
    
      data = strstr(nextline, "<");
      while(data != NULL) {
	tag = new Tag(data);

	if(tag != NULL) {

	  setFilter(tag);
	  //tag->write(cout);

	  if(filter_) {
	    //tag->write(cout);
	    if(replace && tag->isStart() && ! strcmp(tag->getName(), "search_score") && ! strcmp(tag->getAttributeValue("name"), "identityscore")) {
	      identityscore = atof(tag->getAttributeValue("value"));
	    }
	    else if(replace && tag->isStart() && ! strcmp(tag->getName(), "search_score") && ! strcmp(tag->getAttributeValue("name"), "homologyscore")) {
	      homologyscore = atof(tag->getAttributeValue("value"));
	    }
	    else if(replace && tag->isStart() && ! strcmp(tag->getName(), "search_score") && ! strcmp(tag->getAttributeValue("name"), "ionscore")) {
	      ionscore = atof(tag->getAttributeValue("value"));
	    }
	    else if(replace && tag->isStart() && ! strcmp(tag->getName(), "search_score") && ! strcmp(tag->getAttributeValue("name"), "star")) {
	      star = ! strcmp(tag->getAttributeValue("value"), "1");
	    }
	    else if(tag->isStart() && ! strcmp(tag->getName(), "search_hit")) {
	      if(! strcmp(tag->getAttributeValue("hit_rank"), "1"))
		replace = True;
	      else
		replace = False;
	    }
	  } // if filter
	  //if(filter_memory_)
	  // tag->write(cout);
	  if(replace && filter_memory_) {

	    //process
	    identity_mean_ += identityscore;
	    identity_tot_++;


	    if(ionscore == -1.0) {
	      cerr << "error: -1.0 ion score" << endl;
	      exit(1);
	    }
	    if(homologyscore == -1.0) {
	      cerr << "error: -1.0 homology score" << endl;
	      exit(1);
	    }
	    if(identityscore == -1.0) {
	      cerr << "error: -1.0 identity score" << endl;
	      exit(1);
	    }

	    if(star_ == MASCOTSTAR_PENALIZE && ! star && ionscore - homologyscore > 0.0) {
	      adj_ionscore_identity_mean_ += (ionscore - identityscore);
	      adj_ionscore_identity_meansq_ += (ionscore - identityscore) * (ionscore - identityscore);
	      adj_ionscore_homology_mean_ += (ionscore - homologyscore);
	      adj_ionscore_homology_meansq_ += (ionscore - homologyscore) * (ionscore - homologyscore);
	      adj_ionscore_corr_ += (ionscore - identityscore) * (ionscore - homologyscore);
	      adj_ionscore_tot_++;
	    }

	    ionscore = -1.0;
	    homologyscore = -1.0;
	    identityscore = -1.0;
	    star = True;

	    analyze = False; // reset
	  }
	  delete tag;
	} // if not null

	data = strstr(data+1, "<");
      } // next tag
    } // not done

  } // next line
  fin.close();



  // now do the final calculations.....
  if(identity_tot_ > 1)
    identity_mean_ /= (identity_tot_ - 1);

 cout << "results for charge " << charge_ << ": " << identity_tot_ << " id tot and " << adj_ionscore_tot_ << " adj scores" << endl;
 
  adj_ionscore_tot_--;

  if(star_ == MASCOTSTAR_PENALIZE && adj_ionscore_tot_ >= min_adj_ionscore_) {




    adj_ionscore_identity_mean_ /= adj_ionscore_tot_;
    adj_ionscore_identity_stdev_ = sqrt((adj_ionscore_identity_meansq_ / adj_ionscore_tot_) - (adj_ionscore_identity_mean_ * adj_ionscore_identity_mean_));
    adj_ionscore_homology_mean_ /= adj_ionscore_tot_;
    adj_ionscore_homology_stdev_ = sqrt((adj_ionscore_homology_meansq_ / adj_ionscore_tot_) - (adj_ionscore_homology_mean_ * adj_ionscore_homology_mean_));

    //double corr = ((adj_ionscore_corr_ / (adj_ionscore_tot_-1)) - (adj_ionscore_identity_mean_ * adj_ionscore_homology_mean_))/((adj_ionscore_tot_ - 1) adj_ionscore_identity_stdev_ * adj_ionscore_homology_stdev_);

    double corr = (adj_ionscore_corr_ - (adj_ionscore_identity_mean_ * adj_ionscore_homology_mean_ * (adj_ionscore_tot_+1)))/(adj_ionscore_identity_stdev_ * adj_ionscore_homology_stdev_ * adj_ionscore_tot_);

    cout << "results for charge " << charge_ << ": " << adj_ionscore_identity_mean_ << " adj_ion_mean and " << adj_ionscore_homology_mean_ << " adj_ion_hom mean " << identity_mean_ << "id mean" << corr << " correlation (r) " << endl;

    if(adj_ionscore_homology_stdev_ > 0.0) {
      slope_ = corr * adj_ionscore_identity_stdev_ / adj_ionscore_homology_stdev_;
      regression_complete_ = True;

    }
    intercept_ = adj_ionscore_identity_mean_ - slope_ * adj_ionscore_homology_mean_;


    if(regression_complete_) {
      regression_error_ = adj_ionscore_identity_meansq_ - (2 * intercept_ * adj_ionscore_identity_mean_ * (adj_ionscore_tot_ + 1)) - (2 * slope_ * adj_ionscore_corr_) + (intercept_ * intercept_ * (adj_ionscore_tot_ + 1)) + (2 * slope_ * intercept_ * adj_ionscore_homology_mean_ * (adj_ionscore_tot_ + 1)) + (slope_ * slope_ * adj_ionscore_homology_meansq_);
      if(adj_ionscore_tot_ > 1)
	regression_error_ /= (adj_ionscore_tot_ - 1);
      regression_error_ = sqrt(regression_error_);


      cout << charge_ << "+ ion - id = " << slope_ << "*(ion - hom) + " << intercept_ << " with error = " << regression_error_ << endl;
      cout << "mean ion - id: " << adj_ionscore_identity_mean_ << ", mean ion - hom: " << adj_ionscore_homology_mean_ << endl;

    } // if complete
  }
  delete[] nextline;
}



void MascotScoreParser::setFilter(Tag* tag) {
  if(tag == NULL)
    return;

  if(filter_memory_) {
    filter_memory_ = False;
    filter_ = False;
  }

  if(! strcmp(tag->getName(), "spectrum_query")) {
    if(tag->isStart() && 
       tag->getAttributeValue("assumed_charge") != NULL &&
       atoi(tag->getAttributeValue("assumed_charge")) == charge_)       
      filter_ = True;
    else if(filter_ && tag->isEnd())
      filter_memory_ = True;
  }

}

double MascotScoreParser::getMeanIdentityScore() { 
  return identity_mean_;
}

double MascotScoreParser::getMaxAdjustedIonscoreIdentityScoreDiff(double adj_ionscore_homology) {
  if(! regression_complete_)
    return 999.0; // no real max
  

  return adj_ionscore_homology * slope_ + intercept_ + num_stdevs_ * regression_error_;

  //return 1.0;

}

void MascotScoreParser::setMascotScoreParserDescription(char* description, int descr_length) {
  if(regression_complete_)
    sprintf(description, "slope: %0.2f intercept: %0.2f regression error: %0.2f", slope_, intercept_, regression_error_);
  else
    sprintf(description, "no fvalue regression");

}
