/*

Program       : Protein                                                       
Author        : Andrew Keller <akeller@systemsbiology.org>                                                       
Date          : 11.27.02 

Protein sequence

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
Insitute for Systems Biology
1441 North 34th St. 
Seattle, WA  98103  USA
akeller@systemsbiology.org

*/

#include "Protein.h"


Protein::Protein(char* name) {
  name_ = new char[strlen(name)+1];
  strcpy(name_, name);
  name_[strlen(name)] = 0;
  //cout << "name: " << name_ << endl;
}

Protein::~Protein() {
  if(name_ != NULL)
    delete name_;
}


char* Protein::getSequence(char* database) {
  ifstream fdata(database);
  if(! fdata) {
    cerr << "cannot open database " << database << endl;
    exit(1);
  }
  const int line_size = 2000;
  char nextline[line_size];
  int prot_size = 50000;
  char* seq = new char[prot_size];
  const int prot_name_size = 200;
  char prot[prot_name_size];
  seq[0] = 0;
  prot[0] = 0;
  Boolean first = True;

  Boolean found = False;
  char* match;

  // /data/search/akeller/databases/halobacterium_111401_plus_human.prot
  while(fdata.getline(nextline, line_size)) {
    if(strlen(nextline) > 0 && nextline[0] == '>') {
      if(found)
	return seq;

      match = strstr(nextline, name_);
      if(match != NULL && strlen(match) == strlen(nextline) - 1 && (strlen(match) == strlen(name_) || match[strlen(name_)] == ' ' || match[strlen(name_)] == '\t')) 


	 //if(strlen(nextline+1) == strlen(name_) && strcmp(nextline+1, name_) == 0) // protein
	found = True;

    } // prot name
    else {
      if(found) {
	if(first) {
	  first = False;
	  seq[0] = 0;
	}
      }
      if(strlen(seq) + strlen(nextline) <= prot_size) {
	strcat(seq, nextline);
      }
    }

  } // next line of db

  fdata.close(); // done

  if(found)
    return seq;
  return NULL; // not found

}
