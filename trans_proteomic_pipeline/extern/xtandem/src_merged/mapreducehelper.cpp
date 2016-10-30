/*

 MapReduce implementation of X!Tandem
 Copyright (C) 2010 Insilicos LLC All Rights Reserved

 Also includes MPI implementation based on X!!Tandem which is
 Copyright (C) 2007 Robert D Bjornson, all rights reserved

 All of which is based on the original X!Tandem project which is
 Copyright (C) 2003-2004 Ronald C Beavis, all rights reserved
 
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

#ifdef HAVE_MULTINODE_TANDEM // support for Hadoop and/or MPI?

#include <iostream>
#undef READ_GZIP // messes with ifstream etc
#include "stdafx.h"
#include <algorithm>
#include <set>
#include "msequence.h"
#include "msequencecollection.h"
#include "msequenceserver.h"
#include "msequtilities.h"
#include "mspectrum.h"
#include "loadmspectrum.h"
#include "xmlparameter.h"
#include "mscore.h"
#include "mprocess.h"
#include "saxbiomlhandler.h"
#include "mbiomlreport.h"
#include "mrefine.h"
#include "serialize.h"
#include "timer.h"
#include "mapreducehelper.h"

#include <unistd.h>
#include <string>
#include <sstream>

#define HEARTBEAT_PERIOD_SEC 60
#ifdef MSVC
#include "windows.h"
void mapreduce_heartbeat_until_exit() { // just emit heartbeat on stderr till thread kills us
	fprintf(stderr, "reporter:status:Begin_mapreduce_heartbeat_until_exit\n");
	fflush(stderr);
	while (true) {
		fprintf(stderr, "reporter:status:OK\n");
		fflush(stderr);
		Sleep(HEARTBEAT_PERIOD_SEC*1000); 
	}
}
#else
#include <pthread.h>
void mapreduce_heartbeat_until_exit() { // just emit heartbeat on stderr till thread kills us
	fprintf(stderr, "reporter:status:Begin_mapreduce_heartbeat_until_exit\n");
	fflush(stderr);
	while (true) {
		fprintf(stderr, "reporter:status:OK\n");
		fflush(stderr);
		sleep(HEARTBEAT_PERIOD_SEC); 
	}
}
#endif

#ifdef _DEBUG
void mapreducehelper::exit_mode_mapreduce() { // for debugging
	mode_mapreduce_ = NONE;
}
#endif


static mapreducehelper *mrh_singleton;
mapreducehelper &the_mapreducehelper() { // this is a singleton
	return *mrh_singleton;
}

mapreducehelper::mapreducehelper(int &argc, char **&argv) {
	mrh_singleton = this;
	mapreduce_out_ = NULL;
	mode_mapreduce_ = NONE;
	mode_bangbang_ = false;
	ID_ = -1;
	n_processes_written_ = 0;
	int mapreduce_argc = 0;
	const char *bangbang="-mpi";
	const char *mapper="-mapper";
	const char *reducer="-reducer";
	const char *test="-mapreduceinstalltest";
	const char *reportURL="-reportURL"; // this should appear only in final reducer call
	const char *modestr = "x!tandem";
	if ((argc > 1 && (!strcmp(argv[1],test))))	{
		std::cout << "mapreduce xtandem install appears to be good!\n";
		exit(0); // just making sure it can be run
	}
	if (argc > 1 && (!strcmp(argv[1],bangbang)))	{
		init_mode_bangbang(argc,argv);
		mapreduce_argc = 1; // need to eat that -mpi arg
	} else if ((argc > 1 && (!strncmp(argv[1],mapper,strlen(mapper))||!strncmp(argv[1],reducer,strlen(reducer)))))	{
		if (argc < 4) {
			cerr << "usage for mapreduce: " << argv[0] << " -<mapper|reduce><step> -<outdir> paramsFile [-reportURL <URL>]\n";
			exit(1);
		}
		bool ismapper = !strncmp(argv[1],mapper,strlen(mapper));
		const char *num = argv[1]+(int)(ismapper?strlen(mapper):strlen(reducer));
		step_mapreduce_ = atoi(num);
		modetext_ = argv[1]+1;
		// for local debugging, can specify -mapper1.2.3 to mean "pretend you are task 2 and 3 for MAPPER_1 step"
		const char *dot = num;
		while (dot = strchr(dot+1,'.')) {
			task_mapreduce_.push_back(atoi(dot+1));
		}
		modestr = argv[1];
		int totalproc = 0;
		bool final = false;
		std::stringstream msg;
		if (argc > 4) {  // final result URL given?
			if (strncmp(argv[4],reportURL,strlen(reportURL))) {
				fprintf(stderr, "unexpected argument \"%s\"\n",argv[4]);
				exit(1); // that was easy...
			}
			// looking for possible "-reportURLS3"
			if (!strcmp(argv[4]+strlen(reportURL),"S3")) {
				report_URL_ = "s3n://";
			} 
			report_URL_ += argv[5];
			argc = 4; // done with these args
			final = true;
			msg << "This reducer is running as final step.\n";
		}


		if (ismapper) {
			switch (step_mapreduce_) {
				case 1: 
					{
					// mapper1 expects one or more stdin lines
					// just emits a single line to tell reducer1 its here
					mode_mapreduce_ = MAPPER_1;
					std::string line;
					num_processes_ = 0;
					while (getline(cin,line)) {
						num_processes_++;
					}
					if (!num_processes_) {
						fprintf(stderr, msg.str().c_str());
					} else {
						cout << "1\tOK";
					}
					exit(0); // that was easy...
					} 
				    break;
				case 2:
					mode_mapreduce_ = MAPPER_2;
					break;
				case 3:
					mode_mapreduce_ = MAPPER_3;
					break;
				default:
					fprintf(stderr, "tandem: unknown mapper step %d, quit\n",step_mapreduce_);
					exit(1);
					break;
			}
		} else {
			// everybody else expects one or more stdin lines of form 
			// "<int key> <length of base64 string><tab><length of data after decompressing><tab><base64'ed data>"
			// or
			// "<int key> <filename reference>"
			switch (step_mapreduce_) {
				case 1:
					mode_mapreduce_ = REDUCER_1;
					break;
				case 2:
					mode_mapreduce_ = REDUCER_2;
					break;
				case 3:
					mode_mapreduce_ = REDUCER_3;
					break;
				case 99:  // special mode for performance comparison
					mode_mapreduce_ = NONE;
					argv[1] = argv[2];
					argc = 2;
					msg << "This reducer is running as a normal app, but with a Hadoop heartbeat on stderr.\n";
					break;
				default:
					fprintf(stderr, "unknown reducer step %d\n",step_mapreduce_);
					exit(1);
					break;
			   }
		}
		if (final) { // is this the final reducer step?  
			// then just write stderr to stdout, and stdout to usual
			std::streambuf *cerr_buffer = std::cerr.rdbuf(std::cout.rdbuf());
			mapreduce_out_ = &(std::cout);
		} else if (NONE != mode_mapreduce_) {
			// std::cout is for interprocess communication in mapreduce
			// so redirect current use of stdout to stderr instead
			std::streambuf *cout_buffer = std::cout.rdbuf(std::cerr.rdbuf());
			mapreduce_out_ = new ostream(cout_buffer);
		}
		fprintf(stderr, msg.str().c_str());
		fflush(stderr);

		if (NONE != mode_mapreduce_) {
			mapreduce_argc = 2;
			if (!strcmp(argv[2],"-")) { // use stdio
			} else if (!strcmp(argv[2],"hdfs")) {
				outdir_ = "hdfs:///home/hadoop";
			} else {
				// directory for passing large datasets using stdio references to files
				outdir_ = argv[2];
			}
		}
	}

	if (mapreduce_argc) {
		// now regularize commandline 
		argc-=mapreduce_argc;
		for (int arg=1;arg<argc;arg++) {
			argv[arg] = argv[arg+mapreduce_argc];
		}

		t_ = new timer(modestr);

	}
	if (mode_bangbang() || mode_mapreduce()) {
		// go run our hacked-up tandem main routine
		mapreducehandler(argc,argv);  // won't return except on error
	}
}

std::string mapreducehelper::timestamp() {
	time_t rawtime;
	time ( &rawtime );
	std::string result(ctime (&rawtime) );
	return result;
}

void mapreducehelper::copy_report_to_url(mprocess *p) {
	if (report_URL_.length()) {
		string strKey = "output, path";
		string fname;
		p->m_xmlValues.get(strKey,fname);
		if(fname.length()) {
			// tidy up paths in the file that we flattened
			string cmd = "sed -i \"s#__sl__#/#g\" ";
			cmd += fname;
			cmd += " ; sed -i \"s#__cln__#:#g\" ";
			cmd += fname;
			cmd += " ; hadoop dfs -cp file://";
			cmd += fname;
			cmd += " ";
			cmd += report_URL_;
			system(cmd.c_str());
		}
	}
}

std::string mapreducehelper::numprocs() const {
	std::ostringstream o;
	o << num_processes();
	return o.str();
}

eMapReduceMode mapreducehelper::mode_mapreduce() const { 
	// return nonzero if mode_mapreduce_ is mapper or reducer
	return mode_mapreduce_;
}

bool mapreducehelper::accept_this_mapreduce_input_line(const std::string &line) {
	// return TRUE is the input line is for this task (a local debug thing)
	// for local debugging, can specify -mapper1.2 to mean "pretend you are task 2 for MAPPER_1 step"
	if (task_mapreduce_.size() > 0) {
		int tasknum = atoi(line.c_str());
		for (size_t i=task_mapreduce_.size();i--;) {
			if (tasknum == task_mapreduce_[i]) {
				return true;
			}
		}
		return false;
	}
	return true;
}

void mapreducehelper::report() {
  t_->addsplit("all done");
  t_->print();
  }

// support for mapreducehelper::sort_process_spectra
static bool lessThanSpectrum(const mspectrum &_l,const mspectrum &_r)
{
//	return _l.m_dMH < _r.m_dMH;  // TODO: consider mass sort to localize database use?
	/* repro load order */
	double lhs = _l.m_tId;
	double rhs = _r.m_tId;
	int m,n;
	for (m=0;(lhs >= 100000000);m++) { // watch those speculative 3+ entries
		lhs -= 100000000;
	}
	lhs += 0.1 * m;
	for (n=0;(rhs >= 100000000);n++) { // watch those speculative 3+ entries
		rhs -= 100000000;
	}
	rhs += 0.1 * n;
	return lhs < rhs;
}

void mapreducehelper::sort_process_spectra(mprocess &process) { // mimic load order of single thread operation
	sort(process.m_vSpectra.begin(),process.m_vSpectra.end(),lessThanSpectrum);
}

void mapreducehelper::send_process_to_next_task(mprocess *pProcess,int tasknum) {
	// write an output line with 
	//    key="<destination tasknum>" and value= serialized process
	// or
	//    key="<destination tasknum>" and value= filename containing serialized process
	// returns file pointer for pipe if this involves a file creation command
	bool filebased = (the_mapreducehelper().outdir_.length()>0);
	int len; // will be set to size of serialized packet
	char *serialized=serialize_process(*pProcess,&len,!filebased);  // don't base64 for filebased
	char buf[128];
	sprintf(buf,"%05d",tasknum);
	std::cerr << "output a process with " << pProcess->m_vSpectra.size() << " spectra for task " << buf << "\n";
	if (filebased) { // file-based
		char fname[128];
		sprintf(fname,"%s.%05d.%05d",the_mapreducehelper().modetext_.c_str(),the_mapreducehelper().ID_,the_mapreducehelper().n_processes_written_++);
		std::string odir(the_mapreducehelper().outdir_);
		std::string oname;
		bool local;
		if (local=(0!=strncmp(odir.c_str(),"hdfs:",5))) {
			oname = odir;
			oname += "/";
			oname += fname;
		} else {
			oname = fname;
		}
		// write local file
		ofstream o(oname.c_str(),ios::binary);
		if (o.is_open()) {
			const char *tab = strchr(serialized,'\t');
			tab = strchr(tab+1,'\t')+1;
			// format is <length of compressed data>/t<length of uncompressed data>\t<compressed data>
			o.write(serialized,(tab-serialized)+(size_t)atol(serialized));
			o.close();
		} else {
			cerr << "failed to write file " << oname << "\n";
			exit(1);
		}
#ifndef MSVC  // hadoop and linux stuff here
		if (!local) { // copy out to HDFS
			std::string cmd("hadoop dfs -put ");
			cmd += oname;
			cmd += " ";
			cmd += odir;
			cmd += " ; rm "; // delete local file after copy to HDFS
			cmd += oname;
			cerr << cmd << "\n";
			popen(cmd.c_str(),"r");
			// report the HDFS copy as the filename
			oname = odir;
			oname += "/";
			oname += fname;		
		}
#endif
		*(the_mapreducehelper().mapreduce_out_) << buf << "\tFILE=" << oname << "\n";
	} else {
		*(the_mapreducehelper().mapreduce_out_) << buf << "\t" << serialized << "\n";
	}
	delete[] serialized;
	delete pProcess;
}

int mapreducehelper::read_processes(std::vector<mprocess *> &result) { // return number of processes read
	int nread = 0;
	std::vector<FILE *> pipes;
	std::vector<std::string> fnames;
	while (true) {
		std::string line;
		do {
			getline(cin,line);
		} while(line.length() && !accept_this_mapreduce_input_line(line)); // allow local debug to mimic multi node
		if (line.length()) { 
			nread++;
			the_mapreducehelper().ID_ = atoi(line.c_str()); // sent to us, so this must be our ID
			// skip the key and the tab
			const char *processdata = strchr(line.c_str(),'\t')+1;
			if (!strncmp(processdata,"FILE=",5)) { // file reference
				std::string fname(processdata+5);
#ifndef MSVC  // hadoop and linux stuff here
				if (!strncmp(fname.c_str(),"hdfs:",5)) {
					std::string cmd("hadoop dfs -get ");
					cmd += fname;
					cmd += " ";
					fname = fname.substr(fname.find_last_of("/")+1);
					cmd += fname;
					cerr << timestamp() << cmd << "\n";
					pipes.push_back(popen(cmd.c_str(),"r"));
				}
#endif
				fnames.push_back(fname);
			} else {
				mprocess *process = new mprocess;
				result.push_back(process);
				deserialize_process(processdata,true,*process); // stdio based
			}
		} else {
			break;
		}
	}
	// deal with any files we had to copy in from HDFS
	for (size_t n=0;n<fnames.size();n++) {
#ifndef MSVC  // hadoop and linux stuff here
		if (pipes.size()) {
			pclose(pipes[n]); // block on completion of hdfs copy
		}
#endif
		std::string &fname=fnames[n];
		ifstream ifs(fname.c_str(),ios::binary);
		std::ostringstream oss;
		oss << ifs.rdbuf();
		if (!oss.str().length()) {
			cerr << "failed to read file "<<fname<<"\n";
			exit(1);
		}
		mprocess *process = new mprocess;
		result.push_back(process);
		deserialize_process(oss.str().c_str(),false,*process);
		if (pipes.size()) {
			remove(fname.c_str()); // get rid of intermediate file if we won't need it again
		}
	}
	return nread;
}


// stuff for X!!Tandem MPI implementation
bool mapreducehelper::mode_bangbang() const { // return true iff mpi mode
	return mode_bangbang_;
}
int mapreducehelper::mode_bangbang_client() const { // return MPI rank, or 0 if not MPI mode
	return mode_bangbang()?mpi_rank():0;
}

#endif // HAVE_MULTINODE_TANDEM
