//
// Copyright (c) 2006, 2007 Insilicos LLC and LabKey Software. All rights reserved.
//
// Ported from the Perl module "ProteinProphet.pl", which is 
// copyright Andy Keller and the Institute for Systems Biology.
//
// This library is free software; you can redistribute it and/or 
// modify it under the terms of the GNU Lesser General Public 
// License as published by the Free Software Foundation; either 
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public 
// License along with this library; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
// 
// Brian Pratt 
// Insilicos LLC 
// www.insilicos.com
//
//
// DESCRIPTION:
//
// C++ version of ProteinProphet, originated by Jeff Howbert for Insilicos LLC
//
// NOTES:
//
//
//
// TODO:
//
//

#ifdef _MSC_VER
#pragma warning(disable: 4503)
#pragma warning(disable: 4786)
#endif

#include <math.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <boost/cregex.hpp>          // for John Maddock's regular expression library, now part of boost
#include <stdio.h>

#include "ppSpectrum.h"
#include "ppPeptide.h"
#include "ppProtein.h"
#include "fast_map.h"

#include "common/util.h"
#include "common/TPPVersion.h"
#include "common/ResidueMass/ResidueMass.h"
#include "Validation/Distribution/GammaDistribution.h"

#include "Validation/InterProphet/InterProphetParser/KDModel.h"

#include "gzstream.h"  // for writing gzipped protxml

#ifdef _MSC_VER
#pragma warning(disable:4284) // don't bark about "return type for 'jm::auto_array<char>::operator ->' is 'char *' (ie; not a UDT or reference to a UDT.  Will produce errors if applied using infix notation)"
#endif
#include "CheapRegEx.h"       // for our regex for simple cases
#include <ctime>
#include "Parsers/Parser/TagListComparator.h"
#include "pwiz/utility/misc/random_access_compressed_ifstream.hpp"

typedef std::map<ppProtein *, ppProtein *> ProteinProteinMap;

typedef std::map< int, int > MassCounts;                    // hash: key = mass found on input of an amino acid/terminal mod (rounded to int); value = number of times that mass found
typedef std::set< std::string > StringSet;  
typedef std::map< std::string, double> PeptideMassMap;  
typedef std::map< int, double > ModsMap;
struct CoveredProt
{
   ppProtein *protein; // note we don't alloc or delete this
   int db_index;
};
struct SensitivityAndError
{
   double threshold;
   double sensitivity;
   double error;
};

#ifndef GNUPLOT_BINARY
#ifdef WINDOWS_CYGWIN
#define GNUPLOT_BINARY "gnuplot.exe"
#else
#define GNUPLOT_BINARY "gnuplot"
#endif
#endif

//##############################################################################
// start of function prototypes

const std::string validateExecutableFile( const std::string& filename );
void validateSubstitutionAAs ( int* ptr );			//&& arg0 = pointer to a hash?
int multiiterate( int max_num_iters );
bool multiupdate();
// const std::string& getFirstLine( const std::string& file );		//&& not ever called    //&& arg0 = filename?;   appears to return 'output' = first line of file
// void readDegens( const std::string& degenfile );		         //&& not ever called    //&& arg0 = filename?
const std::string getDateTime();								            // returns formatted date/time string
void readDataFromXML( const std::string& xmlfile );
// bool readData( const std::string& datafile_input, int index ); //&& not used if XML_INPUT
void setInitialWts();
double computeProteinProb( const ppProtein *prot );
double computeGroupProteinProb( const ppProtein *prot, const  std::map< std::string, double > & gr_pep_wts) ;
bool updateProteinProbs();
double numProteins( double min_prot_prob, bool include_groups );
void sens_err( double min_prot_prob, bool include_groups, double& tot_corr, double& tot_incorr );
void writeErrorAndSens( const std::string &datafile, bool include_groups );
void writeScript( const std::string& outfile, const std::string &datafile, double num );
void setWritePermissions( const std::string& file );
void ProteinProbEM();
// const std::string getBofFile( const std::string& xmlfile );    //&& not ever called
void writeXMLOutput( double min_prot_prob, double min_pep_prob, const std::string& xmlfile );
void getAllProteinConfs( double min_prot_prob, double min_pep_prob);
void spaceXML( int num, ogzstream& outfile );
void writeGroupXML( int group_num, double min_pep_prob, int index, ogzstream& outfile );
void computeGroupConfidence( int group_num, double min_pep_prob, int index );
void getUniqueStrippedPeps( const std::vector< ppPeptide* >& peps, std::string &output );
int getTotalNumPeps( const ppProtein *prot );
int getTotalUniqPeps( const ppProtein *prot );
int getTotalNumPeps( const ppProtein * prot, const std::vector< ppPeptide* >& peps );
int getTotalNumDistinctPeps( const ppProtein *prot, const std::vector< ppPeptide* >& peps );
void writeProteinXML( ppProtein * entry, const std::string& index, double min_pep_prob, const std::string& prot_id, ogzstream& outfile );
void computeProteinConfidence( ppProtein * entry, const std::string& index, double min_pep_prob, const std::string& prot_id);
void writeExcelOutput(double min_prot_prob, double min_pep_prob, const std::string &excelfile);
void parseIPI(    const std::string& annot,
                  std::string& protein_description,
                  std::string& ipi,
                  std::string& refseq,
                  std::string& swissprot,
                  std::string& ensembl,
                  std::string& trembl);
boost::RegEx rX_flybase("(FBgn\\S+)\\]");                      //&& Perl regex: $annot =~ /(FBgn\S+)\]/
void writeCoverageInfo( const std::string& outfile, double min_prot_prob, int max_num_prots );
void computeCoverage( double min_prot_prob, int max_num_prots );
double computeMU( );
void getProteinLens();
void getProteinMWs();
void readCoverageResults( std::string& infile );
double getCoverageForEntry( const ppProtein * entry, int max_num_prots );
bool updateProteinWeights( bool include_groups );
int iterate1( int max_iters );
int final_iterate( int max_iters );
void setPepMaxProb( bool use_nsp, bool use_fpkm, bool use_joint_probs, bool compute_spectrum_cnts );
void setExpectedNumSiblingPeps();
void setExpectedNumInstances();
double getExpNumInstances( ppPeptide* pep, const ppProtein * prot);
double getSharedProtProb( ppPeptide* pep, const ppProtein * protein );

double getFPKM( ppPeptide* pep, const ppProtein * protein );


void setFPKMPeps();

double getSharedGroupProtProb( ppPeptide* pep, const ppProtein * prot, const  std::map< std::string, double > & gr_pep_wts);

int getSharedProtIndex( double sharedprotprob );

double getNSPAdjustedProb( double prob, int nsp_index );
bool updateNSPDistributions();
int getFPKMProtIndex( double fpkm );
double getFPKMAdjustedProb( double prob, int fpkm_index );
bool updateFPKMDistributions();

const std::string equivalentPeptide(   std::map< char, char >& substitution_aas, 
                                       std::map< std::string, StringSet >& equivalent_peps,
                                       const std::string& pep );
const std::string equivIProphPeptide(   std::map< char, char >& substitution_aas, 
					std::map< std::string, StringSet >& equivalent_peps,
					//	std::map< std::string, ppPeptideSet >& equiv_pepmasses,
					const std::string& pep, const std::string& mpep, bool update );
unsigned long countTheoretPeps(unsigned long protLen);
void computeFinalProbs();
void getSignPeps( const ppProtein * prot, double min_wt, double min_prob, orderedPeptideList& peps );
int isSubOrSuperset2(   const ppProtein * first_prot,
                        const orderedPeptideList& first_array,
                        const ppProtein * second_prot,
                        const orderedPeptideList& second_array,
                        bool use_ntt_info );
bool equalList( const orderedPeptideList& first_array, const orderedPeptideList& second_array );
bool equalList2(  const ppProtein * first_prot,
                  const orderedPeptideList& first_array,
                  const ppProtein * second_prot,
                  const orderedPeptideList& second_array );
void findDegenGroups3( double min_wt, double min_prob, bool use_ntt_info );
double getDegenWt( const ppProtein *prot, ppPeptide& pep );
void computeDegenWts();
double computeDegenProtProb( const ppProtein *prot );
void computeProtNSP( const ppProtein *prot );
void computeGroupProtNSP( const ppProtein *prot, const std::map< std::string, double > & gr_pep_wts );

void computeProtFPKM( const ppProtein *prot );


std::string strip( const std::string& pep );
void findCommonPeps( const std::vector< ppPeptide* >& peps, std::map< std::string, std::string >& pep_grp_desigs );
void getNumInstancesPeptide( double min_prob, bool use_nsp );
int getNumberDegenProteins( ppPeptide* pep );                 //&& argument should be const, but gives compile errors
int getNumInstancesIndex( double niprob );
void getAllSharedPeptides( ppProtein * cluster, std::vector< ppPeptide* >& shared_peps );
void getAllFPKMs( ppProtein * cluster);
void getBatchAnnotation( const std::string& db_name );                                          //&& not used if XML_INPUT
// const std::string getAnnotation( const std::string& prot, const std::string& db_name, int );    //&& not used if XML_INPUT
int maxPeptideLength();

bool hasIndependentEvidence( const ppProtein * prot, double min_prob );
bool hasIndependentEvidence( const ppProtein * prot, double min_prob, double min_indep_ratio);

bool group( const ppProtein * prot_entry1, const ppProtein * prot_entry2 );
void findGroups();
double computeGroupProb( int ind );

int maxNTT( const ppProtein * prot_entry, ppPeptide* pep );                             //&& second argument should be const, but gives compile errors
double weightedPeptideProbs( const ppProtein *prot, double unique_wt );
int numUniquePeps( const ppProtein * prot );
// int numTrypticEnds( const std::string& peptide );                                   //&& not ever called
void orderGroups();
// const std::string getASAPIndex( const std::string& entry, a pointer? );             //&& call requires non-existent executable
// const std::string getASAPRatio( int index, const std::string& entry, a pointer? );  //&& call requires non-existent executable
// void getDegeneracies( const std::string& file );                                    //&& not used if XML_INPUT
const std::string Bool2Alpha( bool yesno );
bool isSTYMod( const std::string& pep );
void interpretStdModifiedPeptide( const std::string& std_pep, std::string& pep_seq, double& nterm, double& cterm, ModsMap& mods );
const std::string streamlineStdModifiedPeptide( const std::string& std_pep );
void makeStdModifiedPeptide( std::string& peptide, double nterm, double cterm, ModsMap& mods, double error ); 
void appendStdModifiedAminoAcid(std::string &appendToPep, const std::string& aa, double mass, double error );
// const std::string generateSubstitutedPeptide( const std::string& pep, pointer?, pointer? );  //&& deprecated and not called anywhere
bool withinError( double first, double second, double error);
const std::string getTPPVersionInfo();
void writeHashContentsToStdOutput();
void cleanUpPathSeps( std::string& path_name );
std::string& cleanUpProteinDescription( std::string& prot_descr );
bool fileExists( std::string& path_name );
void splitString( const std::string& concat_string, std::vector< std::string >& split_strings );
void splitString( const char * concat_string, std::vector< std::string >& split_strings );
void splitStringSorted( const char * concat_string, std::vector< std::string >& split_strings );
bool compareProteinProbabilitiesDesc( ppProtein* first_prot, ppProtein* second_prot );   //&& arguments should be const, but gives compile errors
bool compareProteinProbabilitiesAsc( ppProtein* first_prot, ppProtein* second_prot );    //&& arguments should be const, but gives compile errors
bool compareCoveredProteinDBIndices( const CoveredProt& first_covered_prot, const CoveredProt& second_covered_prot );
bool compareGroupIndexProbabilities( int first_group_index, int second_group_index );
bool compareGroupMemberProbabilities( const ppProtein * first_protein, const ppProtein * second_protein);
bool comparePeptideNamesAsc( const ppPeptide* first_pep, const ppPeptide* second_pep );             //&& arguments should be const, but gives compile errors

static void format_int(int i, std::string &str ) {
   char buf[100];
   snprintf(buf,sizeof(buf),"%d",i);
   str = buf;
}

static std::string join(char c,std::vector<std::string> &strs) {
   std::string result;
   for (int i=0;i<(int)strs.size();i++) {
      if (i) {
         result += c;
      }
      result += strs[i];
   }
   return result;
}

// total number of function protoptypes = 92
 
// end of function prototypes
//##############################################################################

//##############################################################################
// start of global variables
//
//     C  O  N  F  I  G  U  R  A  T  I  O  N     A  R  E  A
//
// ISB-CYGWIN Release
//   This is a kludge to hardcode isb-cygwin specific defaults for the parameters in this section. 
//   In the future these options should be automatically filled in by a true build system. See the code which 
//   immediately follows this section for the isb-cygwin defaults.

// ALL NON-ISB USERS: SET THIS VALUE TO 0
bool ISB_VERSION = 0;							//&& bool or int??

bool bWarnedNoNTT = false;

// forward declarations (for scoping reasons they are declared here)	//&& comment may not be relevant for C++

std::string LD_LIBRARY_PATH;
std::string DTD_FILE;
std::string XSL_MAKER;
//std::string CGI_HOME;
std::string TOP_PATH;
//std::string CGI_BIN;
std::string BINARY_DIRECTORY;
std::string SERVER_ROOT;
std::string WEBSERVER_URL;
std::string TPPVersionInfo;

bool IPROPHET = false;
bool EXCEL_PEPTIDES = false; // unless specified
double EXCEL_MINPROB = 0.2; // group prob unless specified

//bool XML_INPUT = true;
int RUN_INDEX = 0;
bool IPI_DATABASE = false;
bool DROSOPHILA_DATABASE = false;

bool ACCURACY_MODE = false;            // also triggers paper figures logic
bool PAPER_FIGURES = false;            // for paper figures

int PRINT_PROT_COVERAGE = 5;				// whether or not to print out (average) coverage for each entry,
      											// and if so, max number of degenerate protein members to use as the average for degenerate groups
bool USE_ALT_DEGEN_ENC = true;		   // read degen from file (use with $USE_ALT_DEGEN_ENC in mixture_aft.pl)  //&& this is original cryptic comment
std::string USE_ALT_DEGEN_SUF = ".dgn";			// degen file suffix

bool MALDI = false;						   // #@ARGV > 1 && $ARGV[1] eq 'MALDI';  # interprets spectrum names differently, all 1+ charge
											      //&& value used as bool, assigned as bool or string ?!? - need to sort out
std::string MALDI_TAG = "<!-- MALDI -->";

bool SILENT = true;							// default
bool MERGE_SUBSETS = true;					// whether to have subsets be subsumed by supersets (those prots including all prots and more)
bool DEBUG = false;
std::string STD = "nothingevertomatch";			// #'KTGQAPGFSYTDANK';		//&& huh?
//double WT_POWER = 1.0;						//&& used as an exponent in weight calculations (BSP: useless at this value 1.0)
bool UNIQUE_2_3 = true;						// whether or not to count 2+ and 3+ spectra assigned to same peptide as distinct
bool USE_NSP = true;						   // whether to learn NSP distributions and use to compute peptide probs
double NSP_PSEUDOS = 0.005;				// use for nsp distributions in each bin

bool USE_FPKM = false;						   // whether to learn NSP distributions and use to compute peptide probs
double FPKM_PSEUDOS = 0.005;				// use for nsp distributions in each bin


double DEGEN3_MINWT = 0.2;					// #0.5;
double DEGEN3_MINPROB = 0.2;				// #0.5;
double PROB_ADJUSTMENT = 0.999;			// #0.99; #adjustment to prob (use 1.0 for no adjustment)
double SMOOTH = 0.25;						// #0.5;  #whether or not to smooth nsp distributions, how much to weight neighbors
bool OCCAM = true;							// whether or not to allocate edge wts according to protein probabilities (for fewest total
bool SOFT_OCCAM = false;									      //		number of proteins that explain observed data (set to false for maximum prot list)
bool PLOT_PNG = true;                  //  whether or not to run gnuplot to create a png file
bool ANNOTATION = true;						// whether to write protein annotation info below
bool USE_WT_PRIORS = false;				// whether or not to have minimum wt to each protein
std::string ENZYME_TAG = "ENZYME=";
double MIN_DATA_PROB = 0.05;				// below this prob, data is excluded from analysis
double INIT_MIN_DATA_PROB = MIN_DATA_PROB;
bool COMPUTE_TOTAL_SPECTRUM_COUNTS = true;	// whether or not to keep track of the percent of total spectra (est correct) that correspond to each protein
double total_spectrum_counts = 0.0;
double ASAP_MIN_WT = 0.5;					// peps must be above
double ASAP_MIN_PEP_PROB = 0.5;			// peps must be >=
//std::string peptide;
int charge = 0;                        //&& it may be this could be local in various functions
bool track_peptide_origin = false;		// set to true for extended info not in official xsd
int hit_rank = 0;							   // for identifying peptide source
std::string PROCESS_TAG = "<!-- MODEL ANALYSIS -->";	// labels first line in output to indicate prior analysis
bool BATCH_ANNOTATION = true;			// whether or not to rederive all annotation for database in memory
bool USE_STD_MOD_NAMES = true;
bool OMIT_CONST_STATICS = true;
int constant_static_tots = 0;
double MODIFICATION_ERROR = 0.5;
bool DEGEN_USE_NTT = true;					// for degen groups		//&& but after being passed, it sometimes is compared to 2??  need to check whether only bool
double ORIG_MIN_WT = 0.0;
double ORIG_MIN_PROB = 0.1;
double MIN_WT = ORIG_MIN_WT;
double MIN_PROB = ORIG_MIN_PROB;
double FIN_MIN_PROT_PROB = 0.0;
double FIN_MIN_PEP_PROB = 0.0;
double FINAL_PROB_MIN_WT = MIN_WT;
double FINAL_PROB_MIN_PROB = MIN_PROB;
double num_prots = 0.0;							   // total probability of all proteins, including groups
double num_prots1 = 0.0;							// total probability of all proteins, excluding groups   //&& does not appear to be used

double MIN_INDEP_RAT = 0;
double MU_FACTOR = 1;

unsigned long TOT_DB_LEN = 0;
unsigned long TOT_DB_SEQ = 0;
double MU;

int NODATA = -1;							         //&& need to double-check usage
int singly_datanum = 0;
#define MAX_PP_PREC_CHARGE 7
int datanum[ MAX_PP_PREC_CHARGE-1 ]; // slots for 2+, 3+, 4+, 5+
std::vector<int> ignored_datanum; // track what chargestates were ignored
std::string database;
std::string source_files;
std::string source_files_alt;
std::string OUTFILE; 

bool ICAT = false;
bool GLYC = false;
bool ASAP = false;
bool ASAP_PROPHET = false;					// new and improved
bool XPRESS = false;
bool ASAP_IND = false;
bool LAST_ASAP_IND = false;
bool ASAP_INIT = true;
bool ASAP_REFRESH = false;
bool XML = true;
bool PROTLEN = true;
bool CONFEM = false;
bool SHOW_UNMAPPED = false;
bool NORMPROTLEN = false;
bool ALLPEPS = false;
bool LOGPROBS = false;
bool GROUPWTS = true;
bool INSTANCES = false;
bool PROTMW = false;
bool DEGEN = true;							// unless proven otherwise
bool WINDOWS = false;						// unless proven otherwise
bool USE_GROUPS = true; 
bool ASAP_EXTRACT = false;
bool STY_MOD = false;
bool USE_INTERACT = false;
bool ACCEPT_ALL = false;
bool EXCLUDE_ZEROS = false; 
bool XPRESS_ALL = false;
bool LIBRA = false;
std::string LIBRA_CHANNEL;
bool DB_REFRESH = false;					// database refresh for xml input

int first_iters = 0;
int fourth_iters = 0;
int final_iters = 0;
std::string options;								// holds concatenated list of command line options

int NUM_FPKM_BINS = 10;	
const int initNUM_FPKM_BINS = 10 ;	 							      // number of FPKM distribution bins
double pos_fpkm_prot_distrs[ initNUM_FPKM_BINS ];			   // positive FPKM distributions
double neg_fpkm_prot_distrs[ initNUM_FPKM_BINS ];			   // negative FPKM distributions
int NUM_FPKM_PROT_PROB_THRESHES = NUM_FPKM_BINS - 1;								      // number of FPKM distribution bins
const int initNUM_FPKM_PROT_PROB_THRESHES = initNUM_FPKM_BINS - 1;

const int statNUM_FPKM_BINS = 10 ;	 							      // number of FPKM distribution bins
const int statNUM_FPKM_PROT_PROB_THRESHES = statNUM_FPKM_BINS - 1;



int NUM_NSP_BINS = 10;	
const int initNUM_NSP_BINS = 10 ;	 							      // number of NSP distribution bins
double pos_shared_prot_distrs[ initNUM_NSP_BINS ];			   // positive NSP distributions
double neg_shared_prot_distrs[ initNUM_NSP_BINS ];			   // negative NSP distributions
int NUM_SHARED_PROT_PROB_THRESHES = NUM_NSP_BINS - 1;								      // number of NSP distribution bins
const int initNUM_SHARED_PROT_PROB_THRESHES = initNUM_NSP_BINS - 1;

const int statNUM_NSP_BINS = 8 ;	 							      // number of NSP distribution bins
const int statNUM_SHARED_PROT_PROB_THRESHES = statNUM_NSP_BINS - 1;

Boolean static_nsp = False;
Boolean static_fpkm = False;


double fpkm_prot_prob_threshes[ initNUM_FPKM_PROT_PROB_THRESHES ] =	// upper thresholds of first ( NUM_NSP_BINS - 1 ) NSP bins
  { 2, 4, 6, 8, 10, 12.5, 15, 17.5, 20 };


double static_fpkm_prot_prob_threshes[ statNUM_FPKM_PROT_PROB_THRESHES ] =	// upper thresholds of first ( NUM_NSP_BINS - 1 ) NSP bins
  { 2, 4, 6, 8, 10, 12.5, 15, 17.5, 20 };


double shared_prot_prob_threshes[ initNUM_SHARED_PROT_PROB_THRESHES ] =	// upper thresholds of first ( NUM_NSP_BINS - 1 ) NSP bins
  { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9 };


double static_shared_prot_prob_threshes[ statNUM_SHARED_PROT_PROB_THRESHES ] =	// upper thresholds of first ( NUM_NSP_BINS - 1 ) NSP bins
  { 0.1, 0.25, 0.5, 1.0, 2.0, 5.0, 15 };

int NUM_NI_BINS = 10;	
const int initNUM_NI_BINS = 10 ;	 							      // number of NSP distribution bins
double pos_ni_distrs[ initNUM_NI_BINS ];			   // positive NSP distributions
double neg_ni_distrs[ initNUM_NI_BINS ];			   // negative NSP distributions
int NUM_NI_PROB_THRESHES = NUM_NI_BINS - 1;								      // number of NSP distribution bins
const int initNUM_NI_PROB_THRESHES = initNUM_NI_BINS - 1;

double ni_prob_threshes[ initNUM_NI_PROB_THRESHES ] =	// upper thresholds of first ( NUM_NSP_BINS - 1 ) NSP bins
  { 1, 2, 3, 4, 5, 6, 7, 8, 9 };


unsigned long mean_prot_len;

//  { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1 };

//	  1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2,
//	  2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9, 3,
//	  3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 3.8, 3.9, 4 };

int NSP_BIN_EQUIVS[ initNUM_NSP_BINS ];                      // # enforce monotonic ratio of pos NSP to neg NSP in successive bins...

int FPKM_BIN_EQUIVS[ initNUM_FPKM_BINS ];                      // # enforce monotonic ratio of pos FPKM to neg FPKM in successive bins...

int NI_BIN_EQUIVS[ initNUM_NI_BINS ];   

const int NUM_SENS_ERR_THRESH = 16;                      // number of thresholds for sensitivity and error rate calculations
const double sens_err_thresh[] =                         // upper thresholds for sensitivity and error rate calculations
   { 0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.95, 0.96, 0.97, 0.98, 0.99, 1 };

std::vector< std::vector< ppProtein * > > protein_groups;        // vector of vectors holding each group's protein members
std::vector< double > group_probs;                       // group probabilities
std::vector< int > grp_indices;                          // group indices

std::vector< SensitivityAndError > sensAndErr;           // sensitivities and error rates for correct protein predictions at various probability thresholds



typedef fast_map<ppSpectrum*> spectrumMap;
spectrumMap spectra;           // hash: key = spectrum name; value = pointer to corresponding Spectrum object
spectrumMap singly_spectra;    // hash: key = spectrum name; value = pointer to corresponding Spectrum object.
                               //          For separate storage of spectra from singly charged precursor ions.

ppSpectrum *getSpectrum(const char *name) {
   spectrumMap::iterator specIter = spectra.find( name );
   if ( specIter == spectra.end() )        // spectrum does not exist in hash for spectra
   {
      char *specname = strdup(name);
      spectra.insert( std::make_pair( specname, new ppSpectrum( specname ) ) );   //&& add test for successful insert?
      specIter = spectra.find( specname );
   }
   return specIter->second;
}
ppSpectrum *getSpectrum(const std::string &name) {
   return getSpectrum(name.c_str());
}

ppSpectrum *getSinglySpectrum(const char *name) {
   spectrumMap::iterator specIter = singly_spectra.find( name );
   if ( specIter == singly_spectra.end() )        // spectrum does not exist in hash for spectra
   {
      char *specname = strdup(name);
      singly_spectra.insert( std::make_pair( specname, new ppSpectrum( specname ) ) );   //&& add test for successful insert?
      specIter = singly_spectra.find( specname );
   }
   return specIter->second;
}
ppSpectrum *getSinglySpectrum(const std::string &name) {
   return getSinglySpectrum(name.c_str());
}
                                 
//
// peptide container
//
typedef fast_map<ppPeptide*> peptideMap;
peptideMap peptides;   // hash: key = peptide name; value = pointer to corresponding Peptide object
ppPeptide *getPeptideByName(const char *name) {
   peptideMap::iterator pepIter = peptides.find( name );
   if ( pepIter == peptides.end() )        // peptide does not exist in hash for peptides
   {
      char *pepname = strdup(name);
      peptides.insert( std::make_pair( pepname, new ppPeptide( pepname ) ) );   //&& add test for successful insert?
      pepIter = peptides.find( pepname );
   }
   return pepIter->second;
}
ppPeptide *getPeptideByName(const std::string &name) {
   return getPeptideByName(name.c_str());
}
bool peptideExists(const char *name) {
   return peptides.find(name) != peptides.end();
}
bool peptideExists(const std::string &name) {
   return peptideExists(name.c_str());
}

//
// protein container
//
typedef fast_map<ppProtein *> proteinMap;
proteinMap proteins;   // hash: key = protein name; value = pointer to corresponding Protein object
ppProtein *getProteinByName(const char *name) {
   proteinMap::iterator protIter = proteins.find( name );
   if ( protIter == proteins.end() )        // protein does not exist in hash for proteins
   {
      char *protname = strdup(name);
      proteins.insert( std::make_pair( protname, new ppProtein( protname ) ) );   //&& add test for successful insert?
      protIter = proteins.find( protname );
   }
   return protIter->second;
}
ppProtein *getProteinByName(const std::string &name) {
   return getProteinByName(name.c_str());
}

// check to see if named protein exists, return ptr to it if it does, else NULL
ppProtein * proteinExists(const char *name) {
   proteinMap::iterator protIter = proteins.find( name );
   return (proteins.find(name) != proteins.end()) ? protIter->second : NULL;
}
ppProtein * proteinExists(const std::string &name) {
   return proteinExists(name.c_str());
}

// provide a routine to get a persistant copy of protein name, ideally shared with others
// for minimum memusage
const char *getPersistantProteinName(const char *proteinName) { 
   const proteinMap::iterator protIter = proteins.find( proteinName );
   if ( protIter == proteins.end() )  {      // protein does not exist in hash for proteins
      return strdup(proteinName);
   }
   return protIter->second->getName();
}
const char *getPersistantProteinName(const std::string &proteinName) { 
   return getPersistantProteinName(proteinName.c_str());
}

// provide a routine to get a persistant copy of peptide name, ideally shared with others
// for minimum memusage
const char *getPersistantPeptideName(const char *peptideName) { 
   const peptideMap::iterator pepIter = peptides.find( peptideName );
   if ( pepIter == peptides.end() )  {      // protein does not exist in hash for proteins
      return strdup(peptideName);
   }
   return pepIter->second->getName();
}


std::set< std::string > degen_proteins;               // set:  key = concatenated names of a group of degenerate proteins

std::map< std::string, MassCounts > constant_static_mods;   // hash: key = amino acid or terminal modification symbol; value = hash MassCounts (see above)

std::map< char, char > substitution_aas;			      // hash: key = amino acid symbol to replace; value = new amino acid symbol
std::map< std::string, StringSet > equivalent_peps;  // hash of sets: key = equivalent peptide; value = set ActualPeps (see above)
std::map< std::string, PeptideMassMap > equiv_pepmasses;

std::map< std::string, int > ENZYMES;                 // hash: key = enzyme name; value = count of references to it in input files //&& (?)
typedef std::map< std::string, std::vector< double > > ModMassMap;
ModMassMap MODIFICATION_MASSES;     // hash: key = amino acid modification symbol;
                                                                        //       value = vector containing various masses found for mod.
                                                                        //       For both static and optional mods.

static time_t then;
static int myexit(int code) {
  // std::cout << "ran " << time(NULL)-then << endl;
  std::cout.flush();
  exit(WEXITSTATUS(code));
  return code; // never gets here, of course
}

static void checked_system(const std::string &command) {
   int code = verified_system( command.c_str() ); // system() with verbose error check
   if (code) {
      myexit(code);
   }
}

//************ variables rendered obsolescent at global scope by inclusion in new classes *************
/*
// into class Spectrum
my %specprobs = ();
my %singly_specprobs = ();
my %specpeps = ();
my %singly_specpeps = ();

// into class Peptide
my %unique = (); # whether or not pep is unique (corresponds to a single protein in dataset)
my %spectrum_counts = ();
my %pep_max_probs;            // as ProteinMaxProb
my %pep_orig_max_probs;       // as ProteinOrigMaxProb
my %pep_prob_ind = ();        // as ProteinNTT
my %pep_wts = ();             // as ProteinPepWt
my %orig_pep_wts = ();        // as ProteinOrigPepWt
my %pep_nsp = ();             // as ProteinNSPBin
my %estNSP = ();              // as ProteinEstNSP
my %numInstances;             // as ProteinNumInsts
	
// into class Protein
my %prot_peps = ();           // as set PeptidesInProt
my %protein_probs = ();		   // as Probability
my %final_prot_probs;         // as FinalProbability
my %coverage = (); # protein coverage
my %group_members = (); # whether or not a protein entry is a member of group    // as GroupMembership (this is an int, not a bool as Perl comment implies)
my %subsumed = ();
my %member; # hash by protein pointer to hash of prots in group      //&& ?? doesn't seem to get used this way; incremented as though a counter of number of proteins in a group;
                                                                     //&& when tested, it is only for existence of entry, so could be a bool
my %degen;                                                           //&& incremented as though a counter of number of proteins in a group;
                                                                     //&& when tested, it is only for existence of entry, so could be a bool

*/
//*********************************************************************

//************ variables apparently never actually used *************
/*
my $IPI_EXPLORER_PRE = 'http://srs.ebi.ac.uk/srs7bin/cgi-bin/wgetz?-id+m_RJ1KrMXG+-e+[IPI:' . "'"; #'IPI00011028']
my $IPI_EXPLORER_SUF = "'" . ']';

my $E_EXPLORE = 1;
my $E_EXPLORER_PRE = 'http://www.ensembl.org/';
string E_EXPLORER_MID( "Homo_sapiens" );			//&& referenced in readData but then nothing done with it
string E_EXPLORER_MID_MOUSE( "Mus_musculus" );		//&& referenced in readData but then nothing done with it
my $E_EXPLORER_SUF = '/protview?peptide=';

my @db_prots = (); # stores all entries in database
my $START = 0;

my %mod_index;       //&& deprecated??
my %degen_info;
*/
//*********************************************************************

// end of global variables

static void usage() {
		std::cerr <<  "usage:\tProteinProphet <interact_pepxml_file1> [<interact_pepxml_file2>[....]] <output_protxml_file> (ICAT) (GLYC) (XPRESS) (ASAP_PROPHET) (FPKM) (NONSP) (ACCURACY) (ASAP) (PROTLEN) (NOPROTLEN) (IPROPHET) (NORMPROTLEN) (GROUPWTS) (NOGROUPWTS) (INSTANCES) (REFRESH) (DELUDE) (NOOCCAM) (SOFTOCCAM) (NOPLOT) (PROTMW)\n";
      std::cerr <<  "\t\tNOPLOT: do not generate plot png file\n";
		std::cerr <<  "\t\tNOOCCAM: non-conservative maximum protein list\n";
		std::cerr <<  "\t\tSOFTOCCAM: peptide weights are apportioned equally among proteins within each Protein Group (less conservative protein count estimate)\n";
		std::cerr <<  "\t\tICAT: highlight peptide cysteines\n";
		std::cerr <<  "\t\tGLYC: highlight peptide N-glycosylation motif\n";
		std::cerr <<  "\t\tMINPROB: peptideProphet probabilty threshold (default=" << MIN_DATA_PROB <<") \n";
		std::cerr <<  "\t\tMININDEP: minimum percentage of independent peptides required for a protein (default=0) \n";
		std::cerr <<  "\t\tNOGROUPWTS: check peptide's Protein weight against the threshold (default: check peptide's Protein Group weight against threshold)   \n";
		std::cerr <<  "\t\tACCURACY: equivalent to MINPROB0\n";
		std::cerr <<  "\t\tASAP: compute ASAP ratios for protein entries\n\t\t\t(ASAP must have been run previously on interact dataset)\n";
		std::cerr <<  "\t\tREFRESH: import manual changes to AAP ratios\n\t\t\t(after initially using ASAP option)\n";
		std::cerr <<  "\t\tNORMPROTLEN: Normalize NSP using Protein Length\n";
		std::cerr <<  "\t\tLOGPROBS: Use the log of the probabilities in the Confidence calculations\n";
		std::cerr <<  "\t\tCONFEM: Use the EM to compute probability given the confidence \n";
		std::cerr <<  "\t\tALLPEPS: Consider all possible peptides in the database in the confidence model\n";
		std::cerr <<  "\t\tMUFACTOR: Fudge factor to scale MU calculation (default 1)\n";
		std::cerr <<  "\t\tUNMAPPED: Report results for UNMAPPED proteins\n";
		//		std::cerr <<  "\t\tPROTLEN: Report protein length\n";
		std::cerr <<  "\t\tNOPROTLEN: Do not report protein length\n";
		std::cerr <<  "\t\tINSTANCES: Use Expected Number of Ion Instances to adjust the peptide probabilities prior to NSP adjustment\n";
		std::cerr <<  "\t\tFPKM: Model protein FPKM values\n";
		std::cerr <<  "\t\tPROTMW: Get protein mol weights\n";
		std::cerr <<  "\t\tIPROPHET: input is from iProphet\n";
		std::cerr <<  "\t\tASAP_PROPHET: *New and Improved* compute ASAP ratios for protein entries\n\t\t\t(ASAP must have been run previously on all input interact datasets with mz/XML raw data format)\n";    
		std::cerr <<  "\t\tDELUDE: do NOT use peptide degeneracy information when assessing proteins\n";
   		std::cerr <<  "\t\tEXCELPEPS: write output tab delim xls file including all peptides\n";
		std::cerr <<  "\t\tEXCELxx: write output tab delim xls file including all protein (group)s \n\t\t\t\twith minimum probability xx, where xx is a number between 0 and 1\n";

		std::cerr <<  "\n";
		myexit(1);
}

// consistency check
void consistency_check() {
   printf("consistency check...");
	peptideMap::const_iterator pep_iter = peptides.begin();
   while ( pep_iter != peptides.end() )
   { // for each peptide
      const ppPeptide * pep = pep_iter ->second;
      const ppParentProteinList& parent_proteins_list = pep->getParentProteinList();
      ppParentProteinList::const_iterator parent_proteins_iter = parent_proteins_list.begin();
      while ( parent_proteins_iter != parent_proteins_list.end() )
      {  // for each parent protein peptide thinks it has
         ppProtein *p = parent_proteins_iter -> first;
         // does protein think it has this peptide?
         assert(p->hasPeptide(pep_iter->second));
         ++parent_proteins_iter;
      }
      ++pep_iter;
   }
	proteinMap::const_iterator prot_iter = proteins.begin();
   while ( prot_iter != proteins.end() )
   { // for each protein
      const orderedPeptideList& peps = prot_iter->second->getPeptidesInProt() ;          
      orderedPeptideList::const_iterator peps_iter = peps.begin();
      while ( peps_iter != peps.end() )
      {  // for each peptide protein thinks it has
         ppPeptide *p = *peps_iter;
         assert(p->hasParentProtein(prot_iter->second));
         ++peps_iter;
      }
      ++prot_iter;
   }
   printf("consistency check OK\n");
}


//
// commandline arg processing
//
bool process_arg(const char *arg) {
   //		$SILENT = 0 if($ARGV[$k] eq '-verbose');
   if(!strcmp(arg,"-verbose")) {
      SILENT = 0 ;
   }
   // $PLOT_PNG = 0 if($ARGV[$k] eq 'NOPLOT');
   else if(!strcmp(arg,"NOPLOT")) {
      PLOT_PNG = 0 ;
   } 
   //		$OCCAM = 0 if($ARGV[$k] eq 'NOOCCAM');
   else if(!strcmp(arg,"NOOCCAM")) {
      OCCAM = 0 ;
   } 
   else if(!strcmp(arg,"SOFTOCCAM")) {
      SOFT_OCCAM = true ;
   } 
   //		$ICAT = 1 if($ARGV[$k] eq 'ICAT');
   else if(!strcmp(arg,"ICAT")) {
      ICAT = 1 ;
   }
   //		$ACCURACY_MODE = 1 if($ARGV[$k] eq 'ACCURACY');
   else if(!strcmp(arg,"ACCURACY")) {
      ACCURACY_MODE = 1 ;
   }
   //		$GLYC = 1 if($ARGV[$k] eq 'GLYC');
   else if(!strcmp(arg,"GLYC")) {
      GLYC = 1 ;
   }
   //		$ASAP = 1 if($ARGV[$k] eq 'ASAP');
   else if(!strcmp(arg,"ASAP")) {
      ASAP = 1 ;
   }
   //		$ASAP_REFRESH = 1 if($ARGV[$k] eq 'REFRESH');
   else if(!strcmp(arg,"REFRESH")) {
      ASAP_REFRESH = 1 ;
   }
   //		$XML = 1 if($ARGV[$k] eq 'XML');
   else if(!strcmp(arg,"XML")) {
      XML = 1 ;
   }
   else if (!strcmp(arg,"PROTLEN")) {
      PROTLEN = true;
   }
   else if(!strcmp(arg,"NOPROTLEN")) {
     PROTLEN = false ;
   }
   else if(!strcmp(arg,"CONFEM")) {
      CONFEM = true ;
   }
   else if(!strcmp(arg,"NORMPROTLEN")) {
      NORMPROTLEN = true ;
   }
   else if(!strcmp(arg,"ALLPEPS")) {
      ALLPEPS = true ;
   }
   else if(!strcmp(arg,"UNMAPPED")) {
      SHOW_UNMAPPED = true ;
   }
   else if(!strcmp(arg,"LOGPROBS")) {
      LOGPROBS = true ;
   }
   else if(!strcmp(arg,"PROTMW")) {
      PROTMW = true ;
   }
   else if(!strcmp(arg,"GROUPWTS")) {
      GROUPWTS = true ;
   }
   else if(!strcmp(arg,"NOGROUPWTS")) {
      GROUPWTS = false ;
   }
   else if(!strcmp(arg,"INSTANCES")) {
      INSTANCES = true ;
   }
   //		$USE_NSP = 0 if($ARGV[$k] eq 'NONSP');
   else if(!strcmp(arg,"NONSP")) {
      USE_NSP = 0 ;
   }
   else if(!strcmp(arg,"FPKM")) {
      USE_FPKM = true ;
   }
   //		$DEGEN = 0 if($ARGV[$k] eq 'DELUDE');
   else if(!strcmp(arg,"DELUDE")) {
      DEGEN = 0 ;
   }
   //		$WINDOWS = 1 if($ARGV[$k] eq 'WINDOWS');
   else if(!strcmp(arg,"WINDOWS")) {
      WINDOWS = 1 ;
   }
   //		$USE_GROUPS = 0 if($ARGV[$k] eq 'NOGROUPS');
   else if(!strcmp(arg,"NOGROUPS")) {
      USE_GROUPS = 0 ;
   }
   //		$PRINT_PROT_COVERAGE = 0 if($ARGV[$k] eq 'NOCOVERAGE');
   else if(!strcmp(arg,"NOCOVERAGE")) {
      PRINT_PROT_COVERAGE = 0 ;
   }
   //		$ASAP_EXTRACT = 1 if($ARGV[$k] eq 'EXTRACT');
   else if(!strcmp(arg,"EXTRACT")) {
      ASAP_EXTRACT = 1 ;
   }
   //		$XPRESS = 1 if($ARGV[$k] eq 'XPRESS');
   else if(!strcmp(arg,"XPRESS")) {
      XPRESS = 1 ;
   }
   //		$ASAP_PROPHET = 1 if($ARGV[$k] eq 'ASAP_PROPHET');
   else if(!strcmp(arg,"ASAP_PROPHET")) {
      ASAP_PROPHET = 1 ;
   }
   //		$ACCEPT_ALL = 1 if($ARGV[$k] eq 'ACCEPT_ALL');
   else if(!strcmp(arg,"ACCEPT_ALL")) {
      ACCEPT_ALL = 1 ;
   }
   //		$DB_REFRESH = 1 if($ARGV[$k] eq 'DB_REFRESH');
   else if(!strcmp(arg,"DB_REFRESH")) {
      DB_REFRESH = 1 ;
   }
   //		$EXCLUDE_ZEROS = 1 if($ARGV[$k] eq 'EXCLUDE_ZEROS');
   else if(!strcmp(arg,"EXCLUDE_ZEROS")) {
      EXCLUDE_ZEROS = 1 ;
   }
   //		$XPRESS_ALL = 1 if($ARGV[$k] eq 'XPRESS_ALL');
   else if(!strcmp(arg,"XPRESS_ALL")) {
      XPRESS_ALL = 1 ;
   }
   //		if($ARGV[$k] =~ /^LIBRA(\d+)$/) {
   else if (!strncmp(arg,"LIBRA",5)) {
      //		$LIBRA = 1;
      LIBRA = 1;
      //		$LIBRA_CHANNEL = $1;
      LIBRA_CHANNEL = (arg+5);
      //		}
   }
   //
   //		if($ARGV[$k] =~ /^MINPROB(\S+)$/) {
   else if(!strncmp(arg,"MINPROB",7)) {
      //		$MIN_DATA_PROB = $1;
      MIN_DATA_PROB = atof(arg+7);
      //		}
   }
   
   else if(!strncmp(arg,"MININDEP",8)) {
      //		$MIN_DATA_PROB = $1;
      MIN_INDEP_RAT = atof(arg+8);
      //		}
   }

   else if(!strncmp(arg,"MUFACTOR",8)) {
      //		$MIN_DATA_PROB = $1;
      MU_FACTOR = atof(arg+8);
      //		}
   }
   
   //		$STY_MOD = 1 if($ARGV[$k] eq 'STY');
   else if(!strcmp(arg,"STY")) {
      STY_MOD = 1 ;
   }
   else if(!strcmp(arg,"XML_INPUT")) {
      ; // oldstyle arg //		XML_INPUT = 1 if($ARGV[$k] eq 'XML_INPUT');
   }
   //		$USE_INTERACT = 1 if($ARGV[$k] eq 'INTERACT');
   else if(!strcmp(arg,"INTERACT")) {
      USE_INTERACT = 1 ;
   }
   else if(!strcmp(arg,"EXCELPEPS")) { // if($ARGV[$k] eq 'EXCELPEPS') {
	   EXCEL_PEPTIDES = true;
   }
   else if(!strcmp(arg,"IPROPHET")) { // if($ARGV[$k] eq 'EXCELPEPS') {
	   IPROPHET = true;
   }
   else if(!strncmp(arg,"EXCEL",5)) { //  elsif($ARGV[$k] =~ /^EXCEL(\S+)$/) {
      EXCEL_MINPROB = atof(arg+5); 
      FIN_MIN_PROT_PROB = EXCEL_MINPROB; 
   } else {
      return false;
   }
   return true; // this was a switch, not a file
}

//##############################################################################


int main( int argc, char* argv[] ) {
	hooks_tpp(argc,argv); // handle installdir issues, XML_ONLY etc
   then = time(NULL); // for time info on exit
	// gather TPP version info												//&& Perl method of calling an external generated script has been stripped out
	TPPVersionInfo = getTPPVersionInfo();
	//	$CGI_HOME .= '/' if($CGI_HOME !~ /\/$/);							//&& doesn't seem necessary
	std::string VERSION = "ProteinProphet (C++) by Insilicos LLC and LabKey Software, after the original Perl by A. Keller (" + TPPVersionInfo + ")"; 

   std::cout << VERSION << std::endl;
	
   // regression test stuff
   char *testArg=NULL;
   std::string rawTestArg;
   for (int argNum=1;argNum<argc;argNum++) {
      if (!strncmp(argv[argNum],REGRESSION_TEST_CMDLINE_ARG,strlen(REGRESSION_TEST_CMDLINE_ARG))) {
         // learn or run a test
         testArg = argv[argNum];
         rawTestArg = testArg; // for use with programs we call
         // collapse down over this
         while (argNum < argc-1) {
            argv[argNum] = argv[argNum+1];
            argNum++;
         }
         argc--;
      }
   }
	
	// abort if fewer than two command line arguments
	if ( argc < 3 ) {
      usage();
	}
   
   memset(datanum,0,sizeof(datanum)); // paranoia
   BINARY_DIRECTORY = LOCAL_BIN;
   WEBSERVER_URL = getWebserverUrl() ? getWebserverUrl() : PEPXML_STD_XSL;
   SERVER_ROOT = getWebserverRoot() ? getWebserverRoot() : "";
#if defined __CYGWIN__ || defined WINDOWS_NATIVE
   XSL_MAKER = std::string(getPerlBinary())+std::string(" ")+getCGIFullBin()+"protxml2html.pl";
#else

   XSL_MAKER = std::string(getPerlBinary())+std::string(" ")+CGI_FULL_BIN+"protxml2html.pl";

#endif

   std::vector<std::string> FILES;
//##############################################################################
// start of unconverted code
/*
	use strict;
	use IO::Handle;
	use POSIX;

	use Time::Local;
	use Time::localtime qw(localtime);

	my @FILES = glob($ARGV[0]);

	# make full path
	for(my $f = 0; $f < @FILES; $f++) {
		if($FILES[$f] !~ /^\//) {
		$FILES[$f] = getcwd() . '/' . $FILES[$f];
		if ($^O eq 'MSWin32' ) {
			$FILES[$f] =~ s/\\/\//g;  # get those path seps pointing right!
		}
		}
	}
	my $source_files = join(' ', @FILES);
	my $source_files_alt = join('+', @FILES);
*/
   // process the arg list for switches, input, output file
   int k;
   std::string arglist;
   for (k = 1; k < argc; k++) {
      if (arglist.length()) {
         arglist+=" ";
      }
      arglist+=argv[k];
      if (process_arg(argv[k])) { // it was a switch, some global was set
         if (options.length()) {
            options += " ";
         }
         options += argv[k];
      } else { // assume it's a file - either one of many inputs, or the output
	if (OUTFILE.length()) {
	  FILES.push_back(OUTFILE); // that was actually an input file
	}
	OUTFILE = argv[k]; // last file listed is the output file
	if( !isAbsolutePath(OUTFILE.c_str())) {							// add current working directory to OUTFILE name if needed
	  char buffer[ 1024 ];					
	  if ( safepath_getcwd( buffer, sizeof( buffer ) ) != NULL ) {
	    std::string tempstr = buffer;
	    OUTFILE = tempstr + "/" + OUTFILE;
	  }
	}
	cleanUpPathSeps( OUTFILE );                  // get those path separators pointing upwards and to the right
      }
   } // end arglist processing
   
   if (!FILES.size()) {
     usage();
   }
   
   for (k=0;k<(int)FILES.size();k++) {
     if (k) {
         source_files += " ";
         source_files_alt += "+";
      }
	   source_files += FILES[k];			
	   source_files_alt += FILES[k];		
   }

  eTagListFilePurpose testType;
  char *testFileName=NULL;
  checkRegressionTestArgs(testArg,testType);
  if (testType!=NO_TEST) {
     testFileName = constructTagListFilename(arglist.c_str(), // input files, switches etc
        testArg, // program args
        "ProteinProphet", // program name
        testType); // user info output
  }


	// setWritePermissions( OUTFILE );					//&& function does nothing anyway, so ignore


/*
	my $MULTI_FILES = @FILES > 1;
	my @probs;
	my @triplets;
	my @cgifiles = ();
	foreach(@FILES) {
		if(/^\//) {
		push(@cgifiles, $_);
		}
		else {
		push(@cgifiles, getcwd() . '/' . $_);
		}
	}

	# can get smarter than this for xml format
//	my %substitution_aas = ('I' => 'L', '#' => '*', '@' => '*'); # which aa's are equivalent, and hence should be converted
*/
	substitution_aas[ 'I' ] = 'L';
	substitution_aas[ '#' ] = '*';
	substitution_aas[ '@' ] = '*';

	for (k = 0; k < NUM_NSP_BINS; k++) {
		pos_shared_prot_distrs[ k ] = 1.0 / NUM_NSP_BINS;
		neg_shared_prot_distrs[ k ] = 1.0 / NUM_NSP_BINS;
      NSP_BIN_EQUIVS[ k ] = 0;                              //&& need to check whether these are best initial values for NSP_BIN_EQUIVS
	}


	for (k = 0; k < NUM_FPKM_BINS; k++) {
		pos_fpkm_prot_distrs[ k ] = 1.0 / NUM_FPKM_BINS;
		neg_fpkm_prot_distrs[ k ] = 1.0 / NUM_FPKM_BINS;
      FPKM_BIN_EQUIVS[ k ] = 0;                              //&& need to check whether these are best initial values for FPKM_BIN_EQUIVS
	}
/*

	my %EXTRACTED_INDS = ();

	if(OUTFILE =~ /^(\S+\.)htm$/) {
		$XMLFILE = $1 . 'xml';
	}
	elsif($OUTFILE =~ /^(\S+\.)xml$/) {
		$XMLFILE = $1 . 'xml';
	}
	elsif($OUTFILE =~ /^(\S+\.)shtml$/) {
		$XMLFILE = $1 . 'xml';
	}
	else {
		$OUTFILE .= '.htm';
		$XMLFILE = $OUTFILE . 'xml';
	}
 */
   // we really, really want the right filename extension here...
   const char *ext = strrchr(OUTFILE.c_str(),'.');
   if (ext) {
      if (!hasValidProtXMLFilenameExt(OUTFILE.c_str())) {
         OUTFILE.erase(ext-OUTFILE.c_str());
		 OUTFILE += get_protxml_dot_ext();
      }
   }

 /*
	my %ASAP = ();
	my $ASAP_FILE = $FILES[0];
	if($ASAP_FILE !~ /^\//) {
		$ASAP_FILE = getcwd() . '/' . $ASAP_FILE;
	}
	if ($^O eq 'MSWin32' ) {
		$ASAP_FILE =~ s/\\/\//g;  # get those path seps pointing right!
	}

	# OPTIONS HERE
	my $options = join(' ', @ARGV[2 .. $#ARGV]);
*/

   //	$MIN_DATA_PROB = 0 if($ACCURACY_MODE);
   if (ACCURACY_MODE) {
      MIN_DATA_PROB = 0;
	  PAPER_FIGURES = 1; // preserve data files
   }
   //	$ASAP = 1 if($ASAP_REFRESH); # cannot have refresh without ASAP
   if (ASAP_REFRESH) { // # cannot have refresh without ASAP
      ASAP = 1;
   }
   //	# this one deprecates old ASAP options
   //	if($ASAP_PROPHET) {
   if(ASAP_PROPHET) {
      //		$ASAP = 0;
      //		$ASAP_REFRESH = 0;
      ASAP = ASAP_REFRESH = 0;
      //	}
   }

//	$USE_STD_MOD_NAMES = 0 if(! XML_INPUT);

/*
	# here set environment variable so that Xalan works ok in final system call to protxml2html.pl
	if(! ($LD_LIBRARY_PATH eq '')) {
		my $preset = 1;
		if(exists $ENV{'LD_LIBRARY_PATH'}) {
		my $match_ind = index($ENV{'LD_LIBRARY_PATH'}, $LD_LIBRARY_PATH);
		if($match_ind < 0) {
			$preset = 0;
		}
		else {
			# must make sure both ends are ok
			if($match_ind > 0) {
			my $next = substr($ENV{'LD_LIBRARY_PATH'}, $match_ind, 1);
			$preset = 0 if(! ($next eq ':'));
			}
			if(length($ENV{'LD_LIBRARY_PATH'}) > $match_ind + length($LD_LIBRARY_PATH)) {
			my $next = substr($ENV{'LD_LIBRARY_PATH'}, $match_ind + length($LD_LIBRARY_PATH), 1);
			$preset = 0 if(! ($next eq ':'));
			}
		}
		if(! $preset) {
			if($ENV{'LD_LIBRARY_PATH'} eq '') {
			$ENV{'LD_LIBRARY_PATH'} = $LD_LIBRARY_PATH;
			}
			else {
			$ENV{'LD_LIBRARY_PATH'} .= ':' . $LD_LIBRARY_PATH;
			}
		}
		}
		else {
		$ENV{'LD_LIBRARY_PATH'} = $LD_LIBRARY_PATH;
		}
	} # if not null LD_LIB_PATH
*/
//	# greeting
	if(MALDI) std::cout <<  " --- maldi mode ---" ;
	/*if(XML_INPUT)*/ std::cout <<  " (xml input)" ;
	if(! OCCAM) std::cout <<  " (without OCCAM's 'min prot list' razor)";
	if(ACCURACY_MODE) std::cout <<  " (accuracy mode)" ;
	if(! USE_NSP) std::cout <<  " (no nsp)" ;
	if(! USE_FPKM) std::cout <<  " (no FPKM)" ;
	if(ICAT) std::cout <<  " (icat mode)" ;
	if(GLYC) std::cout <<  " (glyc mode)" ;
	if(! USE_GROUPS) std::cout <<  " (no groups)" ;
	if(XPRESS) std::cout <<  " (XPRESS)" ;
	if(XPRESS_ALL) std::cout <<  " (XPRESS ALL)" ;
	if(ASAP_PROPHET) std::cout <<  " (ASAPRatio)" ;
	if(LIBRA) std::cout <<  " (LIBRA norm channel: " << LIBRA_CHANNEL << ")" ;
	if(ASAP_REFRESH) {
		std::cout <<  " (ASAP refresh";
		if(ASAP_EXTRACT) std::cout <<  "/extract" ;
		std::cout <<  ")";
	}
	else if(ASAP) {
		std::cout <<  " (ASAP)";
	}
        if (NORMPROTLEN) {
		std::cout <<  " (normalize NSP for Protein Length)";
	}
        if (PROTLEN) {
		std::cout <<  " (report Protein Length)";
	}
        if (CONFEM) {
		std::cout <<  " (computing Probability given confidence)";
	}
        if (PROTMW) {
		std::cout <<  " (report Protein Mol Weights)";
	}
        if (INSTANCES) {
		std::cout <<  " (using Expected Number of Ion Instances)";
	}
	if(DEGEN) {
		std::cout <<  " (using degen pep info)";
	}
	else {
		std::cout <<  " (w/o degen pep info)";
	}
	if( PRINT_PROT_COVERAGE == 0 ) std::cout <<  " (no coverage)" ;
	if(STY_MOD) std::cout <<  " (excluding STY mods)" ;
	if(USE_INTERACT) std::cout <<  " (using Interact files)" ;
	if(ACCEPT_ALL) std::cout <<  " (accept all peps)" ;
	if(DB_REFRESH) std::cout <<  " (database refresh)" ;
	if(EXCLUDE_ZEROS) std::cout <<  " (exclude zero prob entries)" ;
   if(EXCEL_PEPTIDES) std::cout<< " (create Excel file)";
	std::cout <<  "\n";

//	validateSubstitutionAAs(\%substitution_aas);
   assert(USE_STD_MOD_NAMES ); // otherwise we need to implement validateSubstitutionAAs
/*
	my %ASAP_INDS = ();
*/
   if (USE_STD_MOD_NAMES )
   {
      substitution_aas.clear();
      substitution_aas[ 'I' ] = 'L';
   }
   database.clear(); // init the database name
   for (int index = 0;index<(int)FILES.size();index++) {
     readDataFromXML( FILES[index] ); 
   }
   int ndata = singly_datanum;
   for (int d=MAX_PP_PREC_CHARGE-1;d--;) {
     ndata += datanum[d];
   }
   if (!ndata) {
     printf("WARNING: no data - output file will be empty\n");
     writeXMLOutput( FIN_MIN_PROT_PROB, FIN_MIN_PEP_PROB, OUTFILE );
     exit(1);
   }
#ifdef _DEBUG_PARANOID
   consistency_check();
#endif

   // writeHashContentsToStdOutput();                                                        //&& for test only
	// getBatchAnnotation($database) if(! XML_INPUT && BATCH_ANNOTATION && ANNOTATION);       //&& only called if ! XML_INPUT
 
   if(BATCH_ANNOTATION && ANNOTATION) {
     getBatchAnnotation(database);
  
   }
   
   getNumInstancesPeptide( 0.0, false );
   
   setPepMaxProb( false, false, false, true );
   
   if (INSTANCES)
     setExpectedNumInstances();
   
   setInitialWts();
   
   if (PROTLEN || NORMPROTLEN)
     getProteinLens();
   
   if (PROTMW)
     getProteinMWs();
   
   setExpectedNumSiblingPeps();
   
   if (USE_FPKM)
     setFPKMPeps();
   
   first_iters = iterate1( 50 );
   
   MIN_WT = DEGEN3_MINWT;
   MIN_PROB = DEGEN3_MINPROB;
   
   fourth_iters = multiiterate( 100 );
   // (my $third_iters, my $fourth_iters) = multiiterate(100);    //&& replaced by line immediately above; third_iters never subsequently referenced
   
   //setPepMaxProb ( true, ! ACCEPT_ALL, COMPUTE_TOTAL_SPECTRUM_COUNTS );
   
   findDegenGroups3( 0, 0.5, DEGEN_USE_NTT );
   //getNumInstancesPeptide( 0.0, false );
   
   
   computeDegenWts();
   
   //&& ($MIN_WT, $MIN_PROB) = (0, 0.2);
   
   MIN_WT = 0.5;
   MIN_PROB = 0.2;
   
   final_iters = final_iterate(50);
   
   MIN_WT = 0.5;					//&& why are these variables set so many times in this section of code?
   MIN_PROB = 0.2;
   
   if ( ! OCCAM ) {
     MIN_WT = 0.05;				// if not adjusting weights, include virtually everything
   }
   
   FINAL_PROB_MIN_WT = MIN_WT;
   FINAL_PROB_MIN_PROB = MIN_PROB;

 

   computeFinalProbs();
   
   if( USE_GROUPS )
     {
       findGroups();
     }
   
   double min_prot_prob = 0;
   num_prots = numProteins( min_prot_prob, 1 );
   //&& (my $num_prots1, my $num_prots) = numProteins($min_prot_prob, 1);  //&& replaced by line immediately above; $num_prots1 does not appear to be used elsewhere
   //&& my $SENSERR = 1;                                                   //&& does not appear to be used elsewhere

   bool INCLUDE_GROUPS = 1;

   std::string datafile;
   if (PLOT_PNG && getIsInteractiveMode()) { // TPP (web oriented) vs LabKey (headless) usage style
	   datafile = OUTFILE+"_senserr.txt"; // populate the filename to get an output file
	   if (!PAPER_FIGURES) {
		   // it's going to get deleted, so write to tmp dir
		   datafile += ".tmp.XXXXXX";
		   replace_path_with_webserver_tmp(datafile); // write this in tmpdir
		   safe_fclose(FILE_mkstemp(datafile)); // create then close a uniquely named file
	   }
   }
   writeErrorAndSens(datafile, INCLUDE_GROUPS ); // calculate error and sensitivity, write data file optionally
   if (datafile.length()) {
	   writeScript(OUTFILE, datafile, num_prots);
   }
	INIT_MIN_DATA_PROB = MIN_DATA_PROB;
	MIN_WT = 0.0;

   if ( PRINT_PROT_COVERAGE > 0 )
   {
      computeCoverage( 0.1, PRINT_PROT_COVERAGE );
      if (PROTLEN)
	computeMU(); 
   }

	if ( EXCLUDE_ZEROS ) {
		FIN_MIN_PROT_PROB = 0.2;
	}

	if ( XML ) {
      // count number of proteins that will be output
      int num_final_prots = 0;
      for ( proteinMap::const_iterator prot_iter = proteins.begin(); prot_iter != proteins.end(); ++prot_iter )
      {
         if ( prot_iter -> second -> hasProbability() )
         {
            num_final_prots++;
         }
      }
      //&& my $num_final_prots = scalar keys %final_prot_probs;         //&& replace by block immediately above

      getAllProteinConfs( FIN_MIN_PROT_PROB, FIN_MIN_PEP_PROB);
      
      if (CONFEM)
	ProteinProbEM();

      writeXMLOutput( FIN_MIN_PROT_PROB, FIN_MIN_PEP_PROB, OUTFILE );

      if (EXCEL_PEPTIDES) {
         std::string EXCELFILE(OUTFILE);
         int dot = EXCELFILE.find_last_of('.');
         if ((int)std::string::npos != dot) {
            EXCELFILE.erase(dot);
         }
         EXCELFILE += ".xls";
         writeExcelOutput(FIN_MIN_PROT_PROB, FIN_MIN_PEP_PROB, EXCELFILE );
      }

	  if (getIsInteractiveMode()) { // TPP (web oriented) vs LabKey (headless) usage style

      //
      // XSL file
      //
      std::string nfp_ch;
      format_int( num_final_prots, nfp_ch );
      std::string command;
      command = XSL_MAKER + " -file \"" + OUTFILE + "\" " + nfp_ch;
      if(ICAT) {
		   command += " ICAT";
		}
      if (GLYC) {
		   command += " GLYC";
		}
#ifdef __MINGW__
      if (getenv("WEBSERVER_TMP") == NULL) 
	checked_system(command.c_str());
#endif
	  
      }

      std::string parser = BINARY_DIRECTORY + "parser";
      std::string xpressparser = BINARY_DIRECTORY + "XPressProteinRatioParser";
      std::string asapparser = BINARY_DIRECTORY + "ASAPRatioProteinRatioParser";
      std::string asappvalueparser = BINARY_DIRECTORY + "ASAPRatioPvalueParser";
      std::string libraparser = BINARY_DIRECTORY + "LibraProteinRatioParser";

      if(LIBRA) {
		   std::cout << " importing LIBRA (norm channel: " << LIBRA_CHANNEL << " protein ratios...."; 
		   std::string command = libraparser + " \"" + OUTFILE + "\" " + LIBRA_CHANNEL + " " + rawTestArg;
 			checked_system( command );
	   } 

		if(XPRESS || XPRESS_ALL) {

         std::cout << ". . . importing XPRESS protein ratios . . . ";
         std::string command = xpressparser + " \"" + OUTFILE + "\" " + rawTestArg;
         checked_system( command );
		} // XPRESS

      if ( ASAP_PROPHET )
      {
            std::cout << ". . . importing ASAPRatio protein ratios . . . ";
            std::string command = asapparser + " \"" + OUTFILE + "\" " + rawTestArg;
   			checked_system( command );
            std::cout << "and pvalues . . . "; 
            command = asappvalueparser + " \"" + OUTFILE + "\" " + rawTestArg;
				checked_system( command );
            std::cout << std::endl;
			}
/*		else {
			my $fullpath = validateExecutableFile($parser);
			if(! $fullpath) {
			print "$parser not available for importing ASAP ratios\n";
			}
			else {
			print " importing ASAP protein ratios...."; 
			system("$fullpath $OUTFILE asap");

			if($WINDOWS_CYGWIN) {
				# compute the webserver relative png file ahead of time
				if($OUTFILE =~ /^(\S+)\.xml/) {
				my $local_png = $1 . '-pval.png';
				if((length $SERVER_ROOT) <= (length $local_png) && 
				   index((lc $local_png), (lc $SERVER_ROOT)) == 0) {
					$local_png = '/' . substr($local_png, (length $SERVER_ROOT));
				}
				else {
					die "problem: $local_png is not mounted under webserver root: $SERVER_ROOT\n";
					}
				print "and pvalues...."; 
				system("$fullpath $OUTFILE pvalue $local_png");
				print "\n";
				}
			}
			else {
				print "and pvalues...."; 
				system("$fullpath $OUTFILE pvalue");
				print "\n";
			}

		}

		} # asap

		print "", join('', @xsl_results);
*/
	}					// end: if ( XML ...

   // writeHashContentsToStdOutput();                                                           //&& for test only

   // deallocate memory used by Spectrum, Peptide, and Protein objects
#ifdef _MSC_VER  // why does GCC hate this? who knows, just let it leak
   spectra.delete_values();
	spectra.free_keys();

   singly_spectra.delete_values();
	singly_spectra.free_keys();

   peptides.delete_values();
	peptides.free_keys();

	proteins.delete_values();
	proteins.free_keys();
#endif
   if (testType!=NO_TEST) {
     //
     // regression test stuff
     //
	 const char *progname = argv[0]; // gcc 3.3.2 objects to direct use of argv for some reason
     TagListComparator(progname,testType,OUTFILE.c_str(),testFileName);
     delete[] testFileName;
  }

   return myexit(0);
} // end main

//########################################################################################################
//#                                                                                                      #
//#                                             SUBROUTINES                                              #
//#                                                                                                      #
//########################################################################################################

//##############################################################################
// function validateExecutableFile
//    input:   string - name of file to validate
//    output:  string - name of validated file
//
// NOTE: this version does not actually perform any test, so as to avoid problems
//       seen in Perl code with use of WHICH 
//##############################################################################
const std::string validateExecutableFile( const std::string& filename )
{
   return filename;     // just rely on path	##&&
}

/*
//##############################################################################
//
//##############################################################################
sub validateSubstitutionAAs {
(my $ptr) = @_;
my @formers = keys %{$ptr};
for(my $f = 0; $f < @formers; $f++) {
    for(my $g = 0; $g < @formers; $g++) {
	if($f != $g && $formers[$f] eq ${$ptr}{$formers[$g]}) {
	    die " problem with substitution aa's, whereby $formers[$g] -> ${$ptr}{$formers[$g]} -> ${$ptr}{$formers[$f]}\n";
	}
    }
}
}

*/
//##############################################################################
// function multiiterate
//    input:   int - maximum number of iterations to perform
//    output:  int - number of iterations actually performed
//##############################################################################
int multiiterate( int max_num_iters )
{
   int counter = 0;
   int counter2 = 0;
   int done = 0;
   while ( counter < max_num_iters && multiupdate() )
   {
      counter++;
   }
   // # now adjust the nsp distributions
   while ( counter2 < max_num_iters  )
   {

     counter2++;
     if  ( USE_NSP && updateNSPDistributions() ) {
       if(! SILENT) {std::cout <<  " updating nsp distributions.....\n" ;}
     }
     else {
       done++;
     }
     
     if ( USE_FPKM && updateFPKMDistributions() ) {
       done = 0;
       if(! SILENT ) {std::cout <<  " updating fpkm distributions.....\n" ;}
     }
     else if (done) {
       done++;
     }
     if (done)
       break;
     
   }
   return counter2;
}

//##############################################################################
// function multiupdate
//    input:      none
//    output:     bool - true if protein probabilities and weights were updated, false otherwise
//##############################################################################
bool multiupdate()
{
   bool output = false;
   if ( updateProteinProbs() )
   {
      if ( ! SILENT ) { std::cout <<  " updating protein probs....." << std::endl; }
      output = true;
   }
   if ( OCCAM )
   {
 
       output = output && updateProteinWeights( false );

   }
   if ( ! SILENT ) { std::cout <<  "------------------" << std::endl; }
   return output;
}

//##############################################################################
// function getDateTime
//    input:   none
//    output:  string - contains date and time in format returned by ctime()
//##############################################################################
const std::string getDateTime()
{
    //&& my $timestamp = time;
    //&& my $td = localtime($timestamp);
    //&& my $time = sprintf("%04d-%02d-%02dT%02d:%02d:%02d", $td->year + 1900, $td->mon+1, $td->mday, $td->hour, $td->min, $td->sec);
    //&& return $time;
   time_t current_time = time( NULL );
   struct tm *td = localtime(&current_time);
   char buf[200];
   snprintf(buf,sizeof(buf),"%04d-%02d-%02dT%02d:%02d:%02d", 
	   td->tm_year + 1900, td->tm_mon+1, td->tm_mday, td->tm_hour, 
	   td->tm_min, td->tm_sec);
   std::string time_str(buf);
   return time_str;
}

//##############################################################################
// function readDataFromXML
//    input:   string - name of XML file to read in
//    output:  none
//##############################################################################
void readDataFromXML( const std::string& xmlfile )
{
   // local variables

   // note curent totals, in case of multiple input files
   int previous_singly_datanum = singly_datanum;
   int previous_datanum[MAX_PP_PREC_CHARGE-1];
   for (int d=MAX_PP_PREC_CHARGE-1;d--;) {
      previous_datanum[d] = datanum[d];
   }
   ignored_datanum.clear(); // nothing ignored in this file yet
   bool bGotProphetResult = false; // any actual results in this file?

   // variables for storing fields extracted from <spectrum_query ... with RegEx
   std::string spectrum;                  
   int assumed_charge = 0;
   int index = 0;
   int spectrum_query_index = 0;          // for identifying peptide source

   // variables for storing fields extracted from <search_hit ... with RegEx 
   int hit_rank = 0;                      
   std::string peptide;
   std::string mpeptide;
   ppPeptide* pep = NULL;
   std::string protein;
   std::string annot;
   int ntt = UNINIT_VAL; // in perl, this was "-1", which as an index means "last in array"
   double pepmass = 0.0;

   // variables for storing fields extracted from <peptideprophet_result ... with RegEx
   double prob = UNINIT_VAL;                                // probability of peptide from PeptideProphet
   double parsed_probs[ 3 ] = { UNINIT_VAL, UNINIT_VAL, UNINIT_VAL };   // probabilities of peptide from PeptideProphet for nnt = 0, 1, 2 //&&(??)

   // variables for storing fields extracted from <alternative_protein ... with RegEx
   std::vector< std::string > alt_prots;
   std::vector< int > alt_ntts;
   std::vector< std::string > alt_annots;
   int alt_ntt = UNINIT_VAL; // in perl this was -1, which as an index means "last in array"

   // variables for storing fields extracted from <modification_info ... with RegEx
   double nterm = 0.0;                    // nterm and cterm are masses of terminal mods
   double cterm = 0.0;

   // variables for storing fields extracted from <mod_aminoacid_mass ... with RegEx
   ModsMap mods;          // hash: key = position of mod in peptide; value = mass of mod
   int position = 0;
   double mod_mass = 0.0;

   bool analyze = false;

   std::string NONEXISTENT = "UNMAPPED";

   const double error = 0.25;             // modified aa's within this error of one another's masses are considered the same

   // first get database
   std::string database_parser = BINARY_DIRECTORY + "DatabaseParser";
   std::string refresh_parser = BINARY_DIRECTORY + "RefreshParser";

   char buff[ 1024 ];
   std::string command = database_parser + " \"" + xmlfile + "\"";
   std::FILE* in = tpplib_popen( command.c_str(), "r" );          // create a pipe with popen to receive the output of DatabaseParser
   if ( in == NULL )
   {
      myexit( 1 );
   }
   database.clear();
   while ( fgets( buff, sizeof( buff ), in ) != NULL )
      {
      char *nl = strchr(buff,'\n');
      if (nl) {
         *nl = 0; // kill newline
      }
      if (buff[0] && (database.length()) && (database != buff)) {
         std::cout << "Error: input files reference different databases: " << database << " and " << buff << std::endl;
         std::cout << "Use RefreshParser to update all input files to common database" << std::endl;
	   myexit(1);
       }
      database = buff;
      if (database.find(',') != std::string::npos) {
         std::cout << "Error: multiple databases referenced by " << xmlfile << std::endl;
         std::cout << "Use RefreshParser to update all input files to common database" << std::endl;
	       myexit(1);
	   }
   }
   if (database.empty()) {
      std::cout << "WARNING: No database referenced by " << xmlfile << std::endl;
      // myexit(1);
   }
   pclose( in );                                           // close the pipe
   std::cout <<  ". . . reading in " << xmlfile << ". . ." << std::endl;
   if(DB_REFRESH) {
       std::cout <<  "\n";
       std::string command = refresh_parser + " \"" + xmlfile + "\" \"" + database + "\"";
       checked_system(command);
   }
   if (!database.empty() && (database.find( "IPI" ) != std::string::npos || database.find( "ipi" ) != std::string::npos))
   {
      IPI_DATABASE = true;
   }
   if (!database.empty() && (database.find( "rosophila" ) != std::string::npos))
   {
      DROSOPHILA_DATABASE = true;
   }
   // now ready to read input from pepXML file
/*
my $output = 0;
*/
   pwiz::util::random_access_compressed_ifstream XMLInfile( xmlfile.c_str() ); // can read gzipped xml
   char *stringbuf = new char[ LINE_WIDTH ];

   boost::RegEx rX_sample_enzyme( "<sample_enzyme\\s+name=\"(\\S+)\"" );       //&& Perl regex: /\<sample\_enzyme\s+name\=\"(\S+)\"/
   boost::RegEx rX_aminoacid_modification( "<aminoacid_modification\\s+aminoacid=\"(\\S)\".*mass=\"(\\S+)\".*variable=\"N\"" );   //&& Perl regex: /\<aminoacid\_modification\s+aminoacid\=\"(\S)\".*mass\=\"(\S+)\".*variable\=\"N\"/
   boost::RegEx rX_terminal_modification( "<terminal_modification\\s+terminus=\"(\\S)\".*mass=\"(\\S+)\".*variable=\"N\"" );   //&& Perl regex: /\<terminal\_modification\s+terminus\=\"(\S)\".*mass\=\"(\S+)\".*variable\=\"N\"/
   CheapRegEx rX_spectrum( "\\s+spectrum=\"(\\S+)\"" );
   CheapRegEx rX_assumed_charge( "\\s+assumed_charge=\"(\\S+)\"" );
   CheapRegEx rX_index( "\\s+index=\"(\\S+)\"" );
   CheapRegEx rX_hit_rank( "\\s+hit_rank=\"(\\S+)\"" );
   CheapRegEx rX_peptide( "\\s+peptide=\"(\\S+)\"" );
   CheapRegEx rX_protein( "\\s+protein=\"(\\S+)\"" );
   CheapRegEx rX_protein_descr( "\\s+protein_descr=\"(\\S+)\"" );  // original Perl regex was more complicated, for unknown reasons: /\s+protein\_descr\=\"(\S.*?\S)\s*\"/
   CheapRegEx rX_num_tol_term( "\\s+num_tol_term=\"(\\S+)\"" );
   CheapRegEx rX_calc_neutral_pep_mass( "\\s+calc_neutral_pep_mass=\"(\\S+)\"" );
   CheapRegEx rX_probability( "\\s+probability=\"(\\S+)\"" );
   CheapRegEx rX_all_ntt_prob( "\\s+all_ntt_prob=\"(\\S+)\"" );
   CheapRegEx rX_mod_nterm_mass( "\\s+mod_nterm_mass=\"(\\S+)\"" );
   CheapRegEx rX_mod_cterm_mass( "\\s+mod_cterm_mass=\"(\\S+)\"" );
   CheapRegEx rX_position( "\\s+position=\"(\\S+)\"" );
   CheapRegEx rX_mass( "\\s+mass=\"(\\S+)\"" );


   while( XMLInfile.getline( stringbuf, LINE_WIDTH ) ) {

      if( strstr(stringbuf,"<sample_enzyme") && rX_sample_enzyme.Search( stringbuf ) )
      {
         if ( ENZYMES.find( rX_sample_enzyme[ 1 ] ) != ENZYMES.end() )
         {
            ENZYMES[ rX_sample_enzyme[ 1 ] ]++;
         }
         else
         {
            ENZYMES.insert( std::make_pair( rX_sample_enzyme[ 1 ], 1 ) );
         }
      }

      if (strstr(stringbuf, "<msms_run_summary" ))
      {
         RUN_INDEX++;
      }

      if ( OMIT_CONST_STATICS && strstr(stringbuf, "<search_summary" ) )
      {
         constant_static_tots++;
      }

      if ( OMIT_CONST_STATICS && strstr(stringbuf,"<aminoacid_modification") && 
         rX_aminoacid_modification.Search( stringbuf ) )
      {
         std::string aa_mod = rX_aminoacid_modification[ 1 ]; 
         int mass = ( int )floor( atof( rX_aminoacid_modification[ 2 ].c_str() ) + 0.5 );                // round mass to nearest integer
         if ( constant_static_tots == 1 )
         {
            MassCounts new_mass_counts;
            new_mass_counts.insert( std::make_pair( mass, 1 ) );
            constant_static_mods[ aa_mod ] = new_mass_counts;
         }
         else
         {
            if ( constant_static_mods.find( aa_mod ) != constant_static_mods.end() && constant_static_mods[ aa_mod ].find( mass ) != constant_static_mods[ aa_mod ].end() )
            {
               constant_static_mods[ aa_mod ][ mass ]++;
            }
         }
      }

      if ( OMIT_CONST_STATICS && strstr(stringbuf,"<terminal_modification") &&
         rX_terminal_modification.Search( stringbuf ) )
      {
         std::string term_mod = rX_terminal_modification[ 1 ]; 
         int mass = ( int )floor( atof( rX_terminal_modification[ 2 ].c_str() ) + 0.5 );                // round mass to nearest integer
         if ( constant_static_tots == 1 )
         {
            MassCounts new_mass_counts;
            new_mass_counts.insert( std::make_pair( mass, 1 ) );
            constant_static_mods[ term_mod ] = new_mass_counts;
         }
         else
         {
            if ( constant_static_mods.find( term_mod ) != constant_static_mods.end() && constant_static_mods[ term_mod ].find( mass ) != constant_static_mods[ term_mod ].end() )
            {
               constant_static_mods[ term_mod ][ mass ]++;
            }
         }
      }

      // ADD THE RUN BASENAME HERE.......
      
      else if ( strstr(stringbuf,"<spectrum_query" ) )
      {
         // clear variables used for extraction of spectrum_query and saerch_hit data 
         spectrum.clear();
         assumed_charge = 0;
         index = 0;
         spectrum_query_index = 0;

         hit_rank = 0;   
         peptide.clear();
         protein.clear();
         annot.clear();
         ntt = UNINIT_VAL; // in perl this was -1, which as an index means "last in array"
         pepmass = 0.0;

         prob = UNINIT_VAL;
         parsed_probs[ 0 ] = UNINIT_VAL;
         parsed_probs[ 1 ] = UNINIT_VAL;
         parsed_probs[ 2 ] = UNINIT_VAL;

         alt_prots.clear();
         alt_ntts.clear();
         alt_annots.clear();
         alt_ntt = UNINIT_VAL; // in perl this was -1, which as an index means "last in array"

         nterm = 0.0;
         cterm = 0.0;

         mods.clear();
         position = 0;
         mod_mass = 0.0;

         analyze = true;                                 // set flag to process <search_hit ... if it comes after <search_query ...

         if ( rX_spectrum.Search( stringbuf ) )
         {
	   std::ostringstream conv;
	   conv << "_" << RUN_INDEX;
	   spectrum =  rX_spectrum.result() + conv.str() ;
         }
         if ( rX_assumed_charge.Search( stringbuf ) )
         {
            assumed_charge = atoi( rX_assumed_charge.result() );
         }
         if ( rX_index.Search( stringbuf ) )
         {
            index = atoi( rX_index.result() );
            spectrum_query_index = index;          // for identifying peptide source
         }

//      	if(/\s+spectrum\=\"(\S+)\"/) {               //&& don't know why they might modify spectrum name; worry about later
//	         $spectrum = RUN_INDEX . '_' . $1;
//	         if($spectrum =~ /^(\S+)\.\d$/) {
//             $spectrum = $1;
//        }
//   }
         charge = assumed_charge - 2;                 //&& it appears charge states 1, 2, 3 are converted to -1, 0, 1 and 0, 1 then used for indexes in arrays (??)
      }
      else if ( analyze )
      {
         if ( strstr(stringbuf,"<search_hit" ) )
         {
            if ( rX_hit_rank.Search( stringbuf ) )
            {
               hit_rank = atoi( rX_hit_rank.result() );             //&& should confirm that i_hit_rank actually = 1
            }
            if ( rX_peptide.Search( stringbuf ) )
            {
               peptide = rX_peptide.result();
            }
            if ( rX_protein.Search( stringbuf ) )
            {
               protein = rX_protein.result();
            }
            if ( rX_protein_descr.Search( stringbuf ) )
            {
	      annot = rX_protein_descr.result();
            }
	    else {
	      annot = "";
	    }
            if ( rX_num_tol_term.Search( stringbuf ) )
            {
               ntt = atoi( rX_num_tol_term.result() );                  //&& should confirm that ntt is in appropriate range (0 to 2?)
            }
            if( USE_STD_MOD_NAMES && rX_calc_neutral_pep_mass.Search( stringbuf ) )
            {
               pepmass = atof( rX_calc_neutral_pep_mass.result() );
            }
         }
	 if (IPROPHET && strstr(stringbuf,"<interprophet_result" ))
	   {
	     bGotProphetResult = true; // file is not totally bogus
	     if ( rX_probability.Search( stringbuf ) )
	       {
		 prob = atof( rX_probability.result() );
	       }
	     if ( rX_all_ntt_prob.Search( stringbuf ) )
	       {
		 double p1,p2,p3;
		 sscanf(rX_all_ntt_prob.result(),"(%lf,%lf,%lf)",&p1,&p2,&p3);
		 parsed_probs[ 0 ] = p1 * PROB_ADJUSTMENT;
		 parsed_probs[ 1 ] = p2 * PROB_ADJUSTMENT;
		 parsed_probs[ 2 ] = p3 * PROB_ADJUSTMENT;
	       }
	   }
     
	 if ( !IPROPHET && strstr(stringbuf,"<peptideprophet_result" ) )
	   {
	     bGotProphetResult = true; // file is not totally bogus
	     if ( rX_probability.Search( stringbuf ) )
	       {
		 prob = atof( rX_probability.result() );
	       }
	     if ( rX_all_ntt_prob.Search( stringbuf ) )
	       {
		 double p1,p2,p3;
		 sscanf(rX_all_ntt_prob.result(),"(%lf,%lf,%lf)",&p1,&p2,&p3);
		 parsed_probs[ 0 ] = p1 * PROB_ADJUSTMENT;
		 parsed_probs[ 1 ] = p2 * PROB_ADJUSTMENT;
		 parsed_probs[ 2 ] = p3 * PROB_ADJUSTMENT;
	       }
	   }
         
         if ( strstr(stringbuf,"<alternative_protein" ) )
         {
            if ( rX_protein.Search( stringbuf ) )
            {
               std::string alt_protein(rX_protein.result());
               alt_prots.push_back( alt_protein );
            }
            if ( rX_num_tol_term.Search( stringbuf ) )
            {
               alt_ntt = atoi( rX_num_tol_term.result() );        //&& should confirm that i_ntt is in appropriate range (0 to 2?)
               alt_ntts.push_back( alt_ntt );
            }
            if ( rX_protein_descr.Search( stringbuf ) )
            {
               std::string alt_annot(rX_protein_descr.result());
               alt_annots.push_back( alt_annot );
            }
	    else {
	      alt_annots.push_back(""); //optional
	    }
         }
         if ( USE_STD_MOD_NAMES && strstr(stringbuf,"<modification_info" ) )
         {
            if ( rX_mod_nterm_mass.Search( stringbuf ) )
            {
               nterm = atof( rX_mod_nterm_mass.result() );
            }
            if ( rX_mod_cterm_mass.Search( stringbuf ) )
            {
               cterm = atof( rX_mod_cterm_mass.result());
            }
         }
          
         if ( USE_STD_MOD_NAMES && strstr(stringbuf,"<mod_aminoacid_mass" ) )
         {
            if ( rX_position.Search( stringbuf ) && rX_mass.Search( stringbuf ))
            {
               position = atoi( rX_position.result() );
               mod_mass = atof( rX_mass.result() );
               mods.insert( std::make_pair( position, mod_mass ) );
	          }
         }

         // can add quantitation info here....

         ;
         if ( strstr(stringbuf,"</search_hit" ) )
         {
            // process spectrum_query record
            analyze = 0;
            if ( ( protein.find(NONEXISTENT) == string::npos || SHOW_UNMAPPED )  && prob > MIN_DATA_PROB )
            {
				if (charge >= (MAX_PP_PREC_CHARGE-1)) { 
					// chargestate beyond what we handle
					while((int)ignored_datanum.size()<=charge) {
						ignored_datanum.push_back(0);
					}
					ignored_datanum[charge]++;
				} else { // chargestate within what we handle
	      double probs[ NUM_NTT_STATES ]; // One for each NTT 
               for ( int i = 0; i < NUM_NTT_STATES; i++ )
               {
                  probs[ i ] = parsed_probs[ i ];
//               foreach(@parsed_probs)        // not sure what yet what Perl will do with probs array
//                  push(@probs, $_);
               }
               if( charge == -1 ) {
                  singly_datanum++;
               }
               else
               {
                  datanum[ charge ]++;
               }

               //&& !!!!!!!!!!!!!!!!!!! WHAT IF PROTEIN ALREADY EXISTS?!?!?!? !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
               //&& !!!!!!!!!!!!!!!!!!! PERL CODE APPARENTLY JUST OVERWRITES EXISTING VALUES !!!!!!!!!!!!!!!!
               ppProtein *protp = getProteinByName( protein ); // this will create a new protein record if needed
               protp -> setAnnotation( annot );

	       size_t start = annot.find("FPKM=");
	       size_t end=-1;
	       if (start != string::npos) {
		 start+=5;
		 end = annot.find_first_of(" \t\f\v\n\r", start);
		 double FPKM = atof(annot.substr(start, end-start).c_str());
		 protp->setFPKM(FPKM);
	       }
	       else {
		 protp->setFPKM(0);
	       }
               //&& $annotation{$protein} = $annot;   //&& Perl code replaced by above

               // get the standard peptide name
               if ( !IPROPHET && USE_STD_MOD_NAMES ) {
                  makeStdModifiedPeptide( peptide, nterm, cterm, mods, error);
               }
	       else
		 {
		 mpeptide = peptide;
		 makeStdModifiedPeptide( mpeptide, nterm, cterm, mods, error);
	       }
	       
               if ( !IPROPHET && UNIQUE_2_3 )               // add charge state of precursor ion to beginning of peptide name
		 {
		   std::stringstream ss;
                  ss << charge + 2;
                  peptide = ss.str() + '_' + peptide;
               }
	       else {
		 std::stringstream ss;
		 ss << charge + 2;
		 mpeptide =   ss.str() + '_' + mpeptide;
	       }
               // substitute equivalent amino acids into peptide name	
	       if ( !IPROPHET) {
		 peptide = equivalentPeptide( substitution_aas, equivalent_peps, peptide );		
	       }
	       else {
		 peptide = equivIProphPeptide( substitution_aas, equivalent_peps, peptide, mpeptide, true );
		 
	       }
	       

               // Add new peptide.  Note that pepmass and spectrum_query_index will have values of 0 unless set otherwise above.
               // TODO: investigate, this does seem bad - although names are decorated for different masses etc
               //&& !!!!!!!!!!!!!!!!!!! WHAT IF PEPTIDE ALREADY EXISTS?!?!?!? !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
               //&& !!!!!!!!!!!!!!!!!!! PERL CODE APPARENTLY JUST OVERWRITES EXISTING VALUES !!!!!!!!!!!!!!!!
               pep = getPeptideByName( peptide );      // will create a new one if needed
               

	       std::map< std::string, PeptideMassMap >::const_iterator eqpep_iter;
	       if (mpeptide.length()>2) {
		 if ( ( eqpep_iter = equiv_pepmasses.find( peptide ) ) != equiv_pepmasses.end() )
		   {                                                                               
		     equiv_pepmasses[ peptide ][mpeptide.substr(2)] = pepmass ;
		   }
		 else                                                                              
		   {
		     PeptideMassMap new_pep;
		     new_pep.insert( std::make_pair(mpeptide.substr(2), pepmass) );
		     equiv_pepmasses.insert( std::make_pair( peptide, new_pep) );                  
		   }
	       }
	       
	       
	       pep -> setPeptideMass( pepmass );
               pep -> setSpectrumQueryIndex( spectrum_query_index );
               pep -> setHitRank( hit_rank );

               //&& $peptide_masses{$peptide} = $pepmass if($USE_STD_MOD_NAMES);          //&& replaced by above
               //&& if (track_peptide_origin) {                                           //&& replaced by above
               //&&    $peptide_spectrum_query_index{$peptide} = $spectrum_query_index;   //&& replaced by above
               //&&    $peptide_hit_rank{$peptide} = $hit_rank;                           //&& replaced by above

               ppParentProtein &parentProt = pep ->getParentProtein(protp); // will add if needed
			   if (!bWarnedNoNTT && !isInit(ntt)) {
				 std::cout << "warning: no NTT value provided, defaulting to 2" << std::endl;
				 bWarnedNoNTT = true;
			   }
               parentProt.setNTT(ntt);       
               parentProt.setPepWt(1.0/(alt_prots.size()+1));      
               parentProt.setGroupPepWt(1.0/(alt_prots.size()+1));      

	       // per ADK 3/12/07
               /*    //&& following block replaced by above two lines
                  if(exists $pep_prob_ind{$peptide}) {
                     ${$pep_prob_ind{$peptide}}{$protein} = 0;
                     ${$pep_wts{$peptide}}{$protein} = 1;
                     ${$pep_prob_ind{$peptide}}{$protein} = $ntt;    //&& this appears to override assignment two lines above
                  } else {
                     my %next3 = ($protein => 0);
                     $pep_prob_ind{$peptide} = \%next3;
                     my %next4 = ($protein => 1);
                     $pep_wts{$peptide} = \%next4;
                     my %next5 = ($protein => $ntt);
                     $pep_prob_ind{$peptide} = \%next5;              //&& this appears to override assignment four lines above
                  }
               */

               protp -> addPeptide( pep );                                // insert peptide into PeptidesInProt set for this protein (has no effect if peptide already in set)

               //&& $unique{$peptide} = 1 if(! exists $unique{$peptide}); # it is unique  //&& this is now set in constructor for Peptide object 
	    
               // # take care of degenerate cases
               if ( alt_prots.size() > 0)
               {
                  pep -> setUnique( false );

                  //		my @degentries = split(' ', $5);                   //&& no idea what is going on here 
                  //		die "problem with $_\n" if(@degentries%2 != 0);

                  for( unsigned int k = 0; k < alt_prots.size(); k++ )
                  {
                     const char *next_protein = alt_prots[ k ].c_str();
                     ppProtein *altp=getProteinByName( next_protein ); // will create a new protein if needed
		     if (k < alt_annots.size()) {
		       altp -> setAnnotation( alt_annots[ k ] );   //&& same issue as above with probably overwriting existing values
		       std::string annot = alt_annots[ k ];
		       size_t start = annot.find("FPKM=");
		       size_t end=-1;
		       if (start != string::npos) {
			 start+=5;
			 end = annot.find_first_of(" \t\f\v\n\r", start);
			 double FPKM = atof(annot.substr(start, end-start).c_str());
			 altp->setFPKM(FPKM);
		       }
		       else {
			 altp->setFPKM(0);
		       }
		     }
                     ppParentProtein &parentProt = pep->getParentProtein(altp);
		     if (k < alt_ntts.size()) 
		       parentProt.setNTT(alt_ntts[ k ]);                                // inserts new key-value pair for protein-ntt if protein not already in ProteinNTT hash for this peptide
//                     parentProt.setPepWt(pow( ( 1.0 / (alt_prots.size()+1) ), WT_POWER )); // restore this if WT_POWER <> 1
                     parentProt.setPepWt( 1.0 / (alt_prots.size()+1)  ); // inserts new key-value pair for protein-pep_wts if protein not already in ProteinPepWt hash for this peptide
                     parentProt.setGroupPepWt( 1.0 / (alt_prots.size()+1)  ); // inserts new key-value pair for protein-pep_wts if protein not already in ProteinPepWt hash for this peptide
                                                                                                // NOTE: formula is altered from Perl code due to suspected cut-and-paste error; compare use of similar formulas elsewhere
                     /*    //&& following block replaced by above two lines
                        if(exists $pep_prob_ind{$peptide}) {
                           ${$pep_prob_ind{$peptide}}{$next_protein} = $alt_ntts[$k]; 
                           ${$pep_wts{$peptide}}{$next_protein} = 2 / scalar @alt_prots;
                           ${$pep_wts{$peptide}}{$next_protein} = ${$pep_wts{$peptide}}{$protein}**$WT_POWER;
                        } else {
                           my %next3 = ($next_protein => $alt_ntts[$k]); 
                           $pep_prob_ind{$peptide} = \%next3;
                           my %next4 = ($next_protein => (2 / scalar @alt_prots)**$WT_POWER);
                           $pep_wts{$peptide} = \%next4;
                        }
                     */
                     altp -> addPeptide( pep );                                  // insert peptide into PeptidesInProt set for this protein (has no effect if peptide already in set)
                  }           // # next deg entry
               }              // # if degen
               if ( charge == 0 || charge == 1 || charge == 2 || charge == 3 || charge == 4|| charge == 5 )
               {
                  ppSpectrum *spec = getSpectrum(spectrum);
                  spec -> setAssumedCharge( assumed_charge );
                  spec -> setIndex( index );
                  PARANOID_ASSERT(peptideExists(peptide));
                  spec -> setSpecPeps( pep, charge );
                  for ( int i = 0; i < NUM_NTT_STATES; i++ )
                  {
                     spec -> setSpecProbs( probs[ i ], charge, i );
                  }
               }
               else if ( charge == -1 )                                    // # singlys
               {
                  ppSpectrum *spec = getSinglySpectrum(spectrum);
                  spec -> setAssumedCharge( assumed_charge );
                  spec -> setIndex( index );
                  spec -> setSinglySpecPeps( pep );
                  for ( int i = NUM_NTT_STATES; i--; )
                  {
                     spec -> setSinglySpecProbs( probs[ i ], i );
                  }
               }
               // consistency_check();
            }     // end:  if ( ! ( protein == NONEXISTENT) ...
			} // end if charge is within range we handle
         }     // end:  rX.SetExpression( "</spectrum_query" ...
      }     // end:  else if ( analyze )
   }     // end:  while ( XMLInfile.getline( ...

   if (!bGotProphetResult) {
	   const char *progname = IPROPHET?"InterProphet":"PeptideProphet";
	   printf("did not find any %s results in input data!  Did you forget to run %s?\n",progname,progname);
   }

   std::cout << ". . . read in " << (singly_datanum-previous_singly_datanum) << " 1+, " 
	         << (datanum[0]-previous_datanum[0]) << " 2+, " 
			 << (datanum[1]-previous_datanum[1]) << " 3+";
   for (int i=4;i<=MAX_PP_PREC_CHARGE;i++) {
	   std::cout << ", " << (datanum[i-2]-previous_datanum[i-2]) << " " << i << "+";
   }
   std::cout << " spectra with min prob " << MIN_DATA_PROB;
   if (ignored_datanum.size()) {
	   std::cout << " (skipped ";
	   int n=0;
	   for (int i=0;i<(int)ignored_datanum.size();i++) {
		   if (ignored_datanum[i]) {
			   if (n++) {
				   std::cout << ", ";
			   }
			   std::cout << ignored_datanum[i] << " "<<(i+2)<<"+";
		   }
	   }
	   std::cout << " spectra with min prob " << MIN_DATA_PROB << " but unhandled higher charge state)";
   }
   std::cout << std::endl;
   delete[] stringbuf;
   XMLInfile.close();
}


void ProteinProbEM() {
  KDModel* model = new KDModel("Confidence");
  double total = 0;
  double last_tot=0;
  double maxdiff = 0.1;
  cerr << "Running Confidence EM" << endl;
  //  Array<double> allprobs;
  for ( proteinMap::const_iterator prot_iter = proteins.begin(); prot_iter != proteins.end(); ++prot_iter )
    {
      if ( prot_iter -> second -> hasProbability() && prot_iter -> second -> hasConfidence() )
	{
	  //	  allprobs.insert( prot_iter -> second -> getProbability() );
	  total += prot_iter -> second -> getProbability();
	  model->insert( prot_iter -> second -> getProbability(), prot_iter -> second -> getConfidence());
	}
    }

  model->makeReady(false, 10);
  cerr << "Iterations: " << endl;
  while (fabs(total-last_tot) > maxdiff) {
    cerr << "."; 
    last_tot = total;
    total = 0;
    //allprobs.clear();

    // Update probs
    for ( proteinMap::const_iterator prot_iter = proteins.begin(); prot_iter != proteins.end(); ++prot_iter )
      {
	if ( prot_iter -> second -> hasProbability() && prot_iter -> second -> hasConfidence() )
	  {
	    double posprob, negprob;
	    double prob = prot_iter -> second -> getProbability();
	    double conf = prot_iter -> second -> getConfidence();
	    
	    posprob = model->getPosProb(conf) * prob;
	    negprob = model->getNegProb(conf) * (1 - prob);
	    
	    double num = posprob;
	    double denom = posprob + negprob;
	    if (denom <= 0) {
	      prob = 0;
	    }
	    else {
	      prob = exp(posprob) / (exp(posprob) + exp(negprob));
	    }


	    prot_iter -> second -> setProbability(prob);
	    total += prob;
	  }
      }

    // Update model
    model->clear();

    for ( proteinMap::const_iterator prot_iter = proteins.begin(); prot_iter != proteins.end(); ++prot_iter )
      {
	if ( prot_iter -> second -> hasProbability() && prot_iter -> second -> hasConfidence() )
	  {
	    //	  allprobs.insert( prot_iter -> second -> getProbability() );
	    model->insert( prot_iter -> second -> getProbability(), prot_iter -> second -> getConfidence());
	  }
      }
    model->makeReady(false, 10);
  }

  model->report(cerr);
}

//##############################################################################
// function setInitialWts
//    input:   none
//    output:  none
//
// # in proportion to total numbers of peptides corresponding to each of its corresponding proteins
//##############################################################################
void setInitialWts()
{
//   my $alt = 1; # do it by probability instead of cardinality      //&& appears not to be used
   double UNIQUE_FACTOR = 5.0;         // unique peptides are weighted more heavily, by this factor

	peptideMap::const_iterator pep_iter = peptides.begin();
   while ( pep_iter != peptides.end() )                              // process all peptides stored in peptides hash
   {
      double tot_peps = 0.0;
      ppParentProteinList& parent_proteins_list = pep_iter -> second -> getParentProteinListNonConst();
      ppParentProteinList::iterator parent_proteins_iter = parent_proteins_list.begin();
      while ( parent_proteins_iter != parent_proteins_list.end() )
      {
         if ( UNIQUE_FACTOR != 0.0 )
         {
            tot_peps += weightedPeptideProbs( parent_proteins_iter -> first, UNIQUE_FACTOR );
         }
         ++parent_proteins_iter;
      }
      if( tot_peps > 0.0 )
      {
         parent_proteins_iter = parent_proteins_list.begin();
         while ( parent_proteins_iter != parent_proteins_list.end() )
         {
            const ppProtein *prot = parent_proteins_iter -> first;
            if ( OCCAM )
            {
               if ( UNIQUE_FACTOR != 0.0 )
               {
                  parent_proteins_iter -> second->setPepWt(weightedPeptideProbs( prot, UNIQUE_FACTOR ) / tot_peps);
		  parent_proteins_iter -> second->setGroupPepWt(weightedPeptideProbs( prot, UNIQUE_FACTOR ) / tot_peps);
               }
            }
            else
            {
	      parent_proteins_iter -> second->setPepWt(1);
	      parent_proteins_iter -> second->setGroupPepWt(1);
            }
            parent_proteins_iter -> second->setOrigPepWt(parent_proteins_iter -> second->getPepWt()); 
            ++parent_proteins_iter;
         }
      }
		++pep_iter;
   }
}

//##############################################################################
// function computeProteinProb
//    input:   ppProtein * - the protein
//    output:  double - computed probability for protein       //&& should define this better
//##############################################################################
double computeProteinProb( const ppProtein *prot )
{
   double prob = 1.0;
   ppPeptide* pep = NULL;

   const orderedPeptideList& pip_ref = prot -> getPeptidesInProt();     // get set whose keys are all peptides that appear in sequence of passed protein
   for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )                                         // iterate through all peptides in set
   {
      // # go through all other prots that this pep corresponds to, and multiply weight by ( 1 - w' ) for each
      pep = *pip_iter;
      if( ! STY_MOD || ! isSTYMod( pep -> getName() ) )
      {
         ppParentProtein &protref = pep->getParentProtein(prot);
         double pep_wt = protref.getPepWt();
         double max_prob = protref.getMaxProb();
         if ( ( pep_wt >= MIN_WT ) && ( max_prob >= MIN_PROB ) )
         {
            prob *= ( 1 - max_prob * pep_wt);
         }
       }
   }
   return ( 1.0 - prob );
}
//##############################################################################
// function computeGroupProteinProb
//    input:   ppProtein * - the protein
//    output:  double - computed probability for protein       //&& should define this better
//##############################################################################
double computeGroupProteinProb( const ppProtein *prot, const  std::map< std::string, double > & gr_pep_wts ) 
{
   double prob = 1.0;
   ppPeptide* pep = NULL;

   const orderedPeptideList& pip_ref = prot -> getPeptidesInProt();     // get set whose keys are all peptides that appear in sequence of passed protein
   for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )                                         // iterate through all peptides in set
   {
      // # go through all other prots that this pep corresponds to, and multiply weight by ( 1 - w' ) for each
      pep = *pip_iter;
      if( ! STY_MOD || ! isSTYMod( pep -> getName() ) )
      {
         ppParentProtein &protref = pep->getParentProtein(prot);
         double pep_wt = protref.getPepWt();
         double max_prob = protref.getMaxProb();
	 const char* pep_str = pep->getName();

         if ( ( gr_pep_wts.find( pep_str )->second >= MIN_WT ) && ( max_prob >= MIN_PROB ) )
         {
            prob *= ( 1 - max_prob * pep_wt);
         }
       }
   }
   return ( 1.0 - prob );
}

//######################

//##############################################################################
// function updateProteinProbs
//    input:   none
//    output:  bool - true if protein probabilities were changed, false otherwise
//##############################################################################
bool updateProteinProbs()
{
   bool changed = false;
   double MAX_DIFF = 0.05;
   
	proteinMap::const_iterator prot_iter = proteins.begin();
   while ( prot_iter != proteins.end() )
   {
      ppProtein *prot = prot_iter -> second;
      double nextprob = computeProteinProb( prot );
      // $changed ||= (! exists $protein_probs{$_} || abs($nextprob - $protein_probs{$_}) > $MAX_DIFF);  //&& replaced by line immediately above
      if ((!prot-> hasProbability()) || ( fabs( nextprob - ( prot-> getProbability() ) ) > MAX_DIFF ))
      {
         prot -> setProbability( nextprob );
         changed = true;
      }
		++prot_iter;
   }
   return changed;
}

//##############################################################################
// function numProteins
//    input:   double - minimum protein or group probability
//    input:   bool - flag for whether to include groups
//    output:  double - sum of protein (and possibly group) probabilities
//##############################################################################
double numProteins( double min_prot_prob, bool include_groups )
{
   double tot = 0.0;
   double group_tot = 0.0;
   for ( proteinMap::const_iterator prot_iter = proteins.begin(); prot_iter != proteins.end(); ++prot_iter )
   {
      if (prot_iter -> second -> hasProbability()) {
         double prob = prot_iter -> second -> getProbability();
         if ( ( ! include_groups || !(prot_iter -> second -> hasGroupMembership())) && prob >= min_prot_prob )
         {
            tot += prob;
         }
      }
   }
   if ( include_groups )
   {
      for ( unsigned int i = 0; i < group_probs.size(); i++ )
      {
         if ( group_probs[ i ] >= min_prot_prob )
         {
            group_tot += group_probs[ i ];
         }
      }
   }
   return tot + group_tot;
}

//##############################################################################
// function sens_err
//    input:   double - minimum protein probability
//    input:   bool - flag for whether to include groups
//    input:   double& - double reference in which to place total of protein correct probabilities 
//    input:   double& - double reference in which to place total of protein incorrect probabilities
//                NOTE: function modifies external values of above two inputs via reference
//    output:  none
//##############################################################################
void sens_err( double min_prot_prob, bool include_groups, double& tot_corr, double& tot_incorr )
{
   tot_corr = 0.0;
   tot_incorr = 0.0;
   for ( proteinMap::const_iterator prot_iter = proteins.begin(); prot_iter != proteins.end(); ++prot_iter )
   {
      if (prot_iter -> second -> hasProbability()) {
         double prob = prot_iter -> second -> getProbability();
         if ( ( ! include_groups || (!prot_iter -> second -> hasGroupMembership())) && prob >= min_prot_prob )
         {
            tot_corr += prob;
            tot_incorr += 1 - prob;
         }
      }
   }
   if( include_groups )
   {
      for( unsigned int k = 0; k < grp_indices.size(); k++ )
      {
         double prob = group_probs[ grp_indices[ k ] ];
         PARANOID_ASSERT(isInit(prob));
         if ( prob >= min_prot_prob )
         {
            tot_corr += prob;
            tot_incorr += 1 - prob;
         }
         else
         {
            break;
         }
      }
   }
}

//##############################################################################
// function writeErrorAndSens
//    input:   bool - flag for whether to include groups
//    output:  none
//
//*** NOTE: This implementation does not take a file name as argument, nor write
//    results to an external file, because there was no apparent subsequent use
//    of the external file in Perl code.
//*** NOTE: Array holding thresholds for sensitivity and error rate calculations
//    is now at file scope, rather than passed as argument.
//##############################################################################
void writeErrorAndSens( const std::string &datafile, bool include_groups )
{
   FILE *OUTFPTR = NULL;
   if (datafile.length()) {
      unlink(datafile);
      OUTFPTR = fopen( datafile.c_str(), "w" );
   }
   double tot_corr = 0.0;
   double tot_incorr = 0.0;
   sens_err( 0.0, include_groups, tot_corr, tot_incorr );
   double tot = 0.0;
   double total = tot_corr;
   sensAndErr.clear();
   for ( unsigned int i = 0; i < (unsigned int)NUM_SENS_ERR_THRESH; i++ )
   {
      sens_err( sens_err_thresh[ i ], include_groups, tot_corr, tot_incorr );
      tot = tot_corr + tot_incorr;
      SensitivityAndError sae;
      sae.threshold = sens_err_thresh[ i ];
      sae.sensitivity = ( total > 0.0 ? tot_corr / total : 1.00 );
      sae.error = ( tot > 0.0 ? tot_incorr / tot : 0.0 );
      sensAndErr.push_back( sae );
      if (OUTFPTR) {
         if((tot > 0) && (total > 0)) {
            fprintf( OUTFPTR ,"%0.3f\t%0.1f\t%0.1f\t%0.3f\t%0.3f\n", sens_err_thresh[ i ], tot_corr, tot_incorr, tot_corr/total, tot_incorr/tot );
         }
         if ((tot == 0) || (total == 0)) {
            fprintf( OUTFPTR, "%0.3f\t%0.1f\t%0.1f\t%0.3f\t%0.3f\n", sens_err_thresh[ i ], 0.0, 0.0, 0.0, 0.0 );
         }
      }
   }
   if (OUTFPTR) {
     fclose(OUTFPTR);
   }
}


//##############################################################################
void writeScript(const std::string &outfile, const std::string &datafile, double num) {
   std::string imagefile(outfile);
   std::string::size_type dot = imagefile.find_last_of('.');
   if (dot != std::string::npos) {
      imagefile.erase(dot);
   }
   imagefile += ".png";
   std::string scriptfile = imagefile + ".script.tmp.XXXXXX";
   replace_path_with_webserver_tmp(scriptfile); // write this in tmpdir
   safe_fclose(FILE_mkstemp(scriptfile)); // create then close a uniquely named file
   unlink(scriptfile); // in case it exists
   std::ofstream OUTF( scriptfile.c_str() );
   
   OUTF << "set terminal png;\n";
   OUTF << "set output \""<< imagefile <<"\";\n";
   OUTF << "set border;\n";
   OUTF << "set title \"Estimated Sensitivity (fraction of " << std::setprecision( 2 ) << num << " total) and Error Rates\";\n";
   OUTF << "set xlabel \"Min Protein Prob\";\n";
   OUTF << "set ylabel \"Sensitivity or Error\";\n";
   OUTF << "set xtics (\"0\" 0, \"0.2\" 0.2, \"0.4\" 0.4, \"0.6\" 0.6, \"0.8\" 0.8, \"1\" 1.0);\n";
   OUTF << "set grid;\n";
   OUTF << "set size 0.75,0.8;\n";
   OUTF << "plot \"" << datafile << "\" using 1:4 title 'sensitivity' with lines, \\\n";
   OUTF << " \"" << datafile << "\" using 1:5 title 'error' with lines\n";
   OUTF.close();
   unlink(imagefile); // in case it exists
   std::string command(GNUPLOT_BINARY);
   command += " \"";
   command += scriptfile;
   command += "\"";
   int result = verified_system(command.c_str()); // invoke gnuplot
   verified_unlink(scriptfile); // clean up
   if (!PAPER_FIGURES) { // preserve for paper figures
      verified_unlink(datafile); // clean up otherwise
   }
   if (result) {
      std::cout <<  "ProteinProphet: png file "<< imagefile << " not written, error code "<< result <<"\n";
   }
}

/*
//##############################################################################
//
//##############################################################################
sub setWritePermissions {
(my $file) = @_;
return; 
my $directory = './'; # default
if($file =~ /^(\S*\/)\S+$/) {
    $directory = $1;
}
system("chmod ug+w $directory");
}

*/


//##############################################################################
// function getAllProteinConfs
//		input:	double - minimum protein probability
//		input:	double - minimum peptide probability
//		output:
//##############################################################################
void getAllProteinConfs( double min_prot_prob, double min_pep_prob) {
  std::string prot_name;
  
  
  


   int index = 1;
   unsigned int grp_counter = 0;

   std::vector< ppProtein* > prot_ptrs;
   proteinMap::const_iterator prot_iter = proteins.begin();
   while ( prot_iter != proteins.end() )
   {
      if (prot_iter -> second -> hasProbability()) {
         prot_ptrs.push_back( prot_iter -> second );
      }
      ++prot_iter;
   }
   std::sort( prot_ptrs.begin(), prot_ptrs.end(), compareProteinProbabilitiesDesc );    // sort proteins in order of descending probability

   for ( unsigned int i = 0; i < prot_ptrs.size(); i++ )
   {
      ppProtein* prot = prot_ptrs[ i ];
      double prob = roundProtProbability(prot -> getProbability());
      prot_name = prot -> getName();
      while( grp_counter < grp_indices.size() )
      {
         PARANOID_ASSERT(isInit(group_probs[ grp_indices[ grp_counter ] ]));
         double group_prob = roundProtProbability(group_probs[ grp_indices[ grp_counter ] ]);   // round probability to nearest 0.01 before making comparison to protein probability
         if ( group_prob > prob && group_prob >= min_prot_prob )
         {
	   computeGroupConfidence(grp_counter , min_pep_prob, index ); 
            grp_counter++;
            index++;
         }
         else
         {
            break;
         }
      }
      if ( prob >= min_prot_prob && !prot -> hasGroupMembership() )
      {
	std::string index_str;
	format_int( index, index_str );
	computeProteinConfidence( prot, index_str, min_pep_prob, "a"); 
	index++;                                                                //&& increment *after* call to writeProteinXML?? 
      }
   }
	// # any stragglers?
   while( grp_counter < grp_indices.size() )
     {
       computeGroupConfidence( grp_counter , min_pep_prob, index ); 
       grp_counter++;
       index++;
     }
   
}

//##############################################################################
// function writeXMLOutput
//		input:	double - minimum protein probability
//		input:	double - minimum peptide probability
//		input:	string - name of XML file to write out
//		output:
//##############################################################################
void writeXMLOutput( double min_prot_prob, double min_pep_prob, const std::string& xmlfile ) {
   std::string prot_name;

   ogzstream XMLOutfile( xmlfile.c_str() ); // will write as .gz if file is so named
	XMLOutfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;

   if (!SERVER_ROOT.empty()) {
       std::string local_xslfile;
       char *xslfile = strdup(xmlfile.c_str()); 
       char *dot = strrchr(xslfile,'.');
       if (dot) {
          *dot = 0;
	    }
       if (!strncasecmp(xslfile,SERVER_ROOT.c_str(),SERVER_ROOT.length())) {
          local_xslfile = WEBSERVER_URL + (xslfile+SERVER_ROOT.length());
       } else {
          local_xslfile = xslfile;
       }
       local_xslfile += ".xsl";
       free(xslfile);
       XMLOutfile << "<?xml-stylesheet type=\"text/xsl\" href=\"" << local_xslfile << "\"?>" << std::endl;
   }

	XMLOutfile << "<protein_summary xmlns=\"http://regis-web.systemsbiology.net/protXML\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://sashimi.sourceforge.net/schema_revision/protXML/" << PROTXML_SCHEMA << "\" summary_xml=\"" << OUTFILE << "\">" << std::endl;
	XMLOutfile << "<protein_summary_header reference_database=\"";

//	if($WINDOWS_CYGWIN) {								//&& worry about cygwin later
//		# here use convential windows name of database
//		my $local_database = `cygpath -u '$database'`;
//		chomp($local_database);
//		XMLOutfile << $local_database;
//		$local_database = `cygpath -w '$database'`;
//		if($local_database =~ /^(\S+)\s?/) {
//		$local_database = $1;
//		}
//		# check for error
//		 XMLOutfile << "" win-cyg_reference_database="', $local_database if(! ($local_database eq '') && $local_database =~ /^\S\:/); 
//	} # if iis & cygwin
//	else {

		XMLOutfile << database;
//	}

	XMLOutfile << "\" residue_substitution_list=\"";

	std::map< char, char >::const_iterator subaa_iter = substitution_aas.begin();
	unsigned int counter = 0;
	while ( subaa_iter != substitution_aas.end() ) { 
		XMLOutfile << subaa_iter -> first << " -> " << subaa_iter -> second;
		if ( counter < substitution_aas.size() - 1 ) {
			XMLOutfile << ", ";
		}
		++subaa_iter;
		counter++;
	}

	if ( IPI_DATABASE ) {
		if ( database.find( "ipi.HUMAN." ) != std::string::npos ) {
			XMLOutfile << "\" organism=\"Homo_sapiens";
		} else if ( database.find( "ipi.MOUSE." ) != std::string::npos ) {
			XMLOutfile << "\" organism=\"Mus_musculus";
		} else if ( database.find( "ipi.RAT." ) != std::string::npos ) {
			XMLOutfile << "\" organism=\"Rattus_norvegicus";
		} else if ( database.find( "hiv_hcv") != std::string::npos ) {
			XMLOutfile << "\" organism=\"Homo_sapiens";
		}
	} else if ( DROSOPHILA_DATABASE ) {
		XMLOutfile << "\" organism=\"Drosophila";
	}

	XMLOutfile << "\" source_files=\"" << source_files; 
	XMLOutfile << "\" source_files_alt=\"" << source_files_alt; 

//	if($WINDOWS_CYGWIN) {								//&& worry about cygwin later
//		my @windows_files = split(' ', $source_files);
//		for(my $k = 0; $k < @windows_files; $k++) {
//		$windows_files[$k] = `cygpath -w '$windows_files[$k]'`;
//		if($windows_files[$k] =~ /^(\S+)\s?/) {
//			$windows_files[$k] = $1;
//		}
//		} # next input file
//		XMLOutfile << "" win-cyg_source_files="', join(' ', @windows_files); 
//	} # if iis & cygwin

//	if($ASAP_FILE =~ /interact\-(\S+)\-data\.htm$/) {	//&& worry about ASAP later
//		XMLOutfile << "" source_file_xtn="', $1; 
//	}

   XMLOutfile << std::setiosflags( std::ios::fixed ); 
	XMLOutfile << "\" min_peptide_probability=\"" << std::setprecision( 2 ) << FINAL_PROB_MIN_PROB;
	XMLOutfile << "\" min_peptide_weight=\"" << std::setprecision( 2 ) << FINAL_PROB_MIN_WT;
	XMLOutfile << "\" num_predicted_correct_prots=\"" << std::setprecision( 1 ) << num_prots;
	XMLOutfile << "\" num_input_1_spectra=\"" << singly_datanum;
	XMLOutfile << "\" num_input_2_spectra=\"" << datanum[ 0 ];
	XMLOutfile << "\" num_input_3_spectra=\"" << datanum[ 1 ];
	XMLOutfile << "\" num_input_4_spectra=\"" << datanum[ 2 ];
	XMLOutfile << "\" num_input_5_spectra=\"" << datanum[ 3 ];
	XMLOutfile << "\" initial_min_peptide_prob=\"" << std::setprecision( 2 ) << INIT_MIN_DATA_PROB;
	if ( COMPUTE_TOTAL_SPECTRUM_COUNTS ) {
		XMLOutfile << "\" total_no_spectrum_ids=\"" << std::setprecision( 1 ) << total_spectrum_counts;
	}
	XMLOutfile << "\" sample_enzyme=\"";
	std::map< std::string, int >::const_iterator enz_iter = ENZYMES.begin();
	counter = 0;
	while ( enz_iter != ENZYMES.end() ) { 
		XMLOutfile << enz_iter -> first;
		if ( counter < ENZYMES.size() - 1 ) {
			XMLOutfile << ", ";
		}
		++enz_iter;
		counter++;
	}
	XMLOutfile << "\">" << std::endl;

   XMLOutfile << "<program_details analysis=\"proteinprophet\" time=\"" << getDateTime() << "\" version=\" Insilicos_LabKey_C++ (" << TPPVersionInfo << ")\">" << std::endl;
	XMLOutfile << "<proteinprophet_details ";
	XMLOutfile << " occam_flag=\"" << Bool2Alpha( OCCAM );
	XMLOutfile << "\" groups_flag=\"" << Bool2Alpha( USE_GROUPS );
	XMLOutfile << "\" degen_flag=\"" << Bool2Alpha( DEGEN );
	XMLOutfile << "\" nsp_flag=\"" << Bool2Alpha( USE_NSP );
	XMLOutfile << "\" fpkm_flag=\"" << Bool2Alpha( USE_FPKM );
	XMLOutfile << "\" initial_peptide_wt_iters=\"" << first_iters;
	XMLOutfile << "\" nsp_distribution_iters=\"" << fourth_iters;
	XMLOutfile << "\" final_peptide_wt_iters=\"" << final_iters;
	if ( options.length() ) {
		XMLOutfile << "\" run_options=\"" << options;
	}
	XMLOutfile << "\">" << std::endl;

	// NSP information
	spaceXML( 2, XMLOutfile );
	XMLOutfile << "<nsp_information neighboring_bin_smoothing=\"" << Bool2Alpha( SMOOTH >= 0.0 ) << "\">" << std::endl;
	double start = 0.0;
	int k;
	for( k = 0; k < NUM_NSP_BINS; k++)
	  {
	    if( k > 0 )
	      {
		start = shared_prot_prob_threshes[ k - 1 ];
		}
		spaceXML( 3, XMLOutfile );
		XMLOutfile << "<nsp_distribution bin_no=\"" << k;
		
		if (!static_nsp) {
		  (k == 0 && shared_prot_prob_threshes[ k ] == 0) ? XMLOutfile << "\" nsp_lower_bound_incl=\"" : XMLOutfile << "\" nsp_lower_bound_excl=\"";
		  XMLOutfile << std::setprecision( 2 ) << start;
		  (k == NUM_NSP_BINS-1) ? XMLOutfile << "\" nsp_upper_bound_excl=\"" : XMLOutfile << "\" nsp_upper_bound_incl=\"" ;
		}
		else {
		  XMLOutfile << "\" nsp_lower_bound_incl=\"";
		  XMLOutfile << std::setprecision( 2 ) << start;
		  XMLOutfile << "\" nsp_upper_bound_excl=\"";
		}
		
		if ( k < NUM_NSP_BINS - 1)
		  {
		    XMLOutfile << std::setprecision( 2 ) << shared_prot_prob_threshes[ k ];
		  }
		else
		  {
		    XMLOutfile << "inf";
		  }
		XMLOutfile << "\" pos_freq=\"" << std::setprecision( 3 ) << floor( pos_shared_prot_distrs[ k ] * 1000.0 + 0.5 ) / 1000.0;     // round to nearest 0.001
 		XMLOutfile << "\" neg_freq=\"" << std::setprecision( 3 ) << floor( neg_shared_prot_distrs[ k ] * 1000.0 + 0.5 ) / 1000.0;     // round to nearest 0.001
		XMLOutfile << "\" pos_to_neg_ratio=\"";
		if ( neg_shared_prot_distrs[ k ] > 0.0 )
      {
			XMLOutfile << std::setprecision( 2 ) << floor( ( pos_shared_prot_distrs[ k ] / neg_shared_prot_distrs[ k ] ) * 100.0 + 0.5 ) / 100.0;  // round to nearest 0.01
		}
      else
      {
			XMLOutfile << std::setprecision( 2 ) << 9999.99;
		}

		if ( NSP_BIN_EQUIVS[ k ] != k )
      {
			XMLOutfile << "\" alt_pos_to_neg_ratio=\"";
         XMLOutfile << std::setprecision( 2 ) << floor( ( pos_shared_prot_distrs[ NSP_BIN_EQUIVS[ k ] ] / neg_shared_prot_distrs[ NSP_BIN_EQUIVS[ k ] ] ) * 100.0 + 0.5 ) / 100.0;  // round to nearest 0.01
		}
		XMLOutfile << "\"/>" << std::endl;
	}	// next nsp bin

	spaceXML( 2, XMLOutfile );
	XMLOutfile << "</nsp_information>" << endl;

	if (USE_FPKM) {
	  // FPKM information
	  spaceXML( 2, XMLOutfile );
	  XMLOutfile << "<fpkm_information neighboring_bin_smoothing=\"" << Bool2Alpha( SMOOTH >= 0.0 ) << "\">" << std::endl;
	  start = 0.0;
	  
	  for( k = 0; k < NUM_FPKM_BINS; k++)
	    {
	      if( k > 0 )
		{
		  start = fpkm_prot_prob_threshes[ k - 1 ];
		}
	      spaceXML( 3, XMLOutfile );
	      XMLOutfile << "<fpkm_distribution bin_no=\"" << k;
	      
	      if (!static_fpkm) {
		(k == 0 && fpkm_prot_prob_threshes[ k ] == 0) ? XMLOutfile << "\" fpkm_lower_bound_incl=\"" : XMLOutfile << "\" fpkm_lower_bound_excl=\"";
		XMLOutfile << std::setprecision( 2 ) << start;
		(k == NUM_FPKM_BINS-1) ? XMLOutfile << "\" fpkm_upper_bound_excl=\"" : XMLOutfile << "\" fpkm_upper_bound_incl=\"" ;
	      }
	      else {
		XMLOutfile << "\" fpkm_lower_bound_incl=\"";
		XMLOutfile << std::setprecision( 2 ) << start;
		XMLOutfile << "\" fpkm_upper_bound_excl=\"";
	      }
	      
	      if ( k < NUM_FPKM_BINS - 1)
		{
		  XMLOutfile << std::setprecision( 2 ) << fpkm_prot_prob_threshes[ k ];
		}
	      else
		{
		  XMLOutfile << "inf";
		}
	      XMLOutfile << "\" pos_freq=\"" << std::setprecision( 3 ) << floor( pos_fpkm_prot_distrs[ k ] * 1000.0 + 0.5 ) / 1000.0;     // round to nearest 0.001
	      XMLOutfile << "\" neg_freq=\"" << std::setprecision( 3 ) << floor( neg_fpkm_prot_distrs[ k ] * 1000.0 + 0.5 ) / 1000.0;     // round to nearest 0.001
	      XMLOutfile << "\" pos_to_neg_ratio=\"";
	      if ( neg_fpkm_prot_distrs[ k ] > 0.0 )
		{
		  XMLOutfile << std::setprecision( 2 ) << floor( ( pos_fpkm_prot_distrs[ k ] / neg_fpkm_prot_distrs[ k ] ) * 100.0 + 0.5 ) / 100.0;  // round to nearest 0.01
		}
	      else
		{
		  XMLOutfile << std::setprecision( 2 ) << 9999.99;
		}
	      
	      if ( FPKM_BIN_EQUIVS[ k ] != k )
		{
		  XMLOutfile << "\" alt_pos_to_neg_ratio=\"";
		  XMLOutfile << std::setprecision( 2 ) << floor( ( pos_fpkm_prot_distrs[ FPKM_BIN_EQUIVS[ k ] ] / neg_fpkm_prot_distrs[ FPKM_BIN_EQUIVS[ k ] ] ) * 100.0 + 0.5 ) / 100.0;  // round to nearest 0.01
		}
	      XMLOutfile << "\"/>" << std::endl;
	    }	// next fpkm bin
	  
	  spaceXML( 2, XMLOutfile );
	  XMLOutfile << "</fpkm_information>" << endl;
	}

	if (INSTANCES) {
	  // NI  information
	  spaceXML( 2, XMLOutfile );
	  XMLOutfile << "<ni_information>" << std::endl;
	  start = 0.0;
	  
	  for( int k = 0; INSTANCES && k < NUM_NI_BINS; k++)
	    {
		if( k > 0 )
		  {
		    start = ni_prob_threshes[ k - 1 ];
		  }
		spaceXML( 3, XMLOutfile );
		XMLOutfile << "<ni_distribution bin_no=\"" << k;
		
		(k == 0 && ni_prob_threshes[ k ] == 0) ? XMLOutfile << "\" ni_lower_bound_incl=\"" : XMLOutfile << "\" ni_lower_bound_excl=\"";
		
		XMLOutfile << std::setprecision( 2 ) << start;
		
		(k == NUM_NI_BINS-1) ? XMLOutfile << "\" ni_upper_bound_excl=\"" : XMLOutfile << "\" ni_upper_bound_incl=\"" ;
		
		if ( k < NUM_NI_BINS - 1)
		  {
		    XMLOutfile << std::setprecision( 2 ) << ni_prob_threshes[ k ];
		  }
		else
		  {
		    XMLOutfile << "inf";
		  }
		XMLOutfile << "\" pos_freq=\"" << std::setprecision( 3 ) << floor( pos_ni_distrs[ k ] * 1000.0 + 0.5 ) / 1000.0;     // round to nearest 0.001
 		XMLOutfile << "\" neg_freq=\"" << std::setprecision( 3 ) << floor( neg_ni_distrs[ k ] * 1000.0 + 0.5 ) / 1000.0;     // round to nearest 0.001
		XMLOutfile << "\" pos_to_neg_ratio=\"";
		if ( neg_ni_distrs[ k ] > 0.0 )
		  {
		    XMLOutfile << std::setprecision( 2 ) << floor( ( pos_ni_distrs[ k ] / neg_ni_distrs[ k ] ) * 100.0 + 0.5 ) / 100.0;  // round to nearest 0.01
		  }
		else
		  {
		    XMLOutfile << std::setprecision( 2 ) << 9999.99;
		  }
		
		if ( NI_BIN_EQUIVS[ k ] != k )
		  {
		    XMLOutfile << "\" alt_pos_to_neg_ratio=\"";
		    XMLOutfile << std::setprecision( 2 ) << floor( ( pos_ni_distrs[ NI_BIN_EQUIVS[ k ] ] / neg_ni_distrs[ NI_BIN_EQUIVS[ k ] ] ) * 100.0 + 0.5 ) / 100.0;  // round to nearest 0.01
		  }
		XMLOutfile << "\"/>" << std::endl;
	    }	// next ni bin
	  
	  spaceXML( 2, XMLOutfile );
	  XMLOutfile << "</ni_information>" << endl;
	}
	// # sens and error info
	for ( unsigned int s = 0; s < sensAndErr.size(); s++ )
   {
      double sens = sensAndErr[ s ].sensitivity;
      double err = sensAndErr[ s ].error;
		spaceXML( 2, XMLOutfile );
      XMLOutfile << "<protein_summary_data_filter min_probability=\"" << std::setprecision( 2 ) << sensAndErr[ s ].threshold;
      XMLOutfile << "\" sensitivity=\"" << std::setprecision( 3 ) << sens;
      XMLOutfile << "\" false_positive_error_rate=\"" << std::setprecision( 3 ) << err;
      XMLOutfile << "\" predicted_num_correct=\"" << std::setprecision( 0 ) << sens * num_prots;
      XMLOutfile << "\" predicted_num_incorrect=\"" << std::setprecision( 0 ) << ( err < 1.0 ? sens * num_prots * err / ( 1.0 - err ) : num_prots );
      XMLOutfile << "\" />" << std::endl;
	}

	XMLOutfile << "</proteinprophet_details>" << std::endl << "</program_details>" << std::endl;
	XMLOutfile << "</protein_summary_header>" << std::endl;
	XMLOutfile << "<dataset_derivation generation_no=\"0\">" << std::endl << "</dataset_derivation>" << std::endl;

	const int MAX_PEPLENGTH = 45;
	int max_peplength = maxPeptideLength();
   if ( max_peplength > MAX_PEPLENGTH )
   {
      max_peplength = MAX_PEPLENGTH;
   }

   int index = 1;
   unsigned int grp_counter = 0;

   std::vector< ppProtein* > prot_ptrs;
   proteinMap::const_iterator prot_iter = proteins.begin();
   while ( prot_iter != proteins.end() )
   {
      if (prot_iter -> second -> hasProbability()) {
         prot_ptrs.push_back( prot_iter -> second );
      }
      ++prot_iter;
   }
   std::sort( prot_ptrs.begin(), prot_ptrs.end(), compareProteinProbabilitiesDesc );    // sort proteins in order of descending probability

   for ( unsigned int i = 0; i < prot_ptrs.size(); i++ )
   {
      ppProtein* prot = prot_ptrs[ i ];
      double prob = roundProtProbability(prot -> getProbability());
      prot_name = prot -> getName();
      while( grp_counter < grp_indices.size() )
      {
         PARANOID_ASSERT(isInit(group_probs[ grp_indices[ grp_counter ] ]));
         double group_prob = roundProtProbability(group_probs[ grp_indices[ grp_counter ] ]);   // round probability to nearest 0.01 before making comparison to protein probability
         if ( group_prob > prob && group_prob >= min_prot_prob )
         {
            writeGroupXML( grp_counter , min_pep_prob, index, XMLOutfile ); 
            grp_counter++;
            index++;
         }
         else
         {
            break;
         }
      }
      if ( prob >= min_prot_prob && !prot -> hasGroupMembership() )
      {
         XMLOutfile << "<protein_group group_number=\"" << index;
         XMLOutfile << "\" probability=\"" << std::setprecision( 4 ) << prob << "\">" << std::endl;
         //&& writeProteinXML($_, $index++, $min_pep_prob, 'a');                 //&& replaced by four lines immediately following
         std::string index_str;
         format_int( index, index_str );
         writeProteinXML( prot, index_str, min_pep_prob, "a", XMLOutfile ); 
         index++;                                                                //&& increment *after* call to writeProteinXML?? 
         XMLOutfile << "</protein_group>" << std::endl;
      }
   }
	// # any stragglers?
   while( grp_counter < grp_indices.size() )
     {
       writeGroupXML( grp_counter , min_pep_prob, index, XMLOutfile ); 
       grp_counter++;
       index++;
     }
   
   
   XMLOutfile << "</protein_summary>" << std::endl;
   XMLOutfile.close();
}

//##############################################################################
// function spaceXML
//		input:	int - number of blocks of three spaces to output
//    input:   ofstream - file handle for writing output
//		output:	none
//##############################################################################
void spaceXML( int num, ogzstream& outfile ) {
	int mult = 3;
	for ( int k = 0; k < num * mult; k++ ) {
		outfile << ' ';
	}
}


void computeGroupConfidence( int grp_num, double min_pep_prob, int index)
{
   int group_ind = grp_indices[ grp_num ];
 
   char id[8] = {'a',0,0,0,0,0,0,0};
   int iInc = 0;
   for ( unsigned int g = 0; g < protein_groups[ group_ind ].size(); g++ )
   {
      std::string index_str;
      format_int( index, index_str );
      index_str += "-";
      std::string index_str2;
      format_int( ( g + 1 ), index_str2 );
      index_str += index_str2;
      std::string id_str(id);
      computeProteinConfidence( protein_groups[ group_ind ][ g ], index_str, min_pep_prob, id_str);

      // Base-26 ID counting.
      while (iInc >= 0 && id[iInc] == 'z')
          id[iInc--] = 'a';
      if (iInc < 0)
      {
          while (id[++iInc])
              id[iInc] = 'a';
          id[iInc] = 'a';
      }
      else
      {
          id[iInc]++;
          iInc = strlen(id) - 1;
      }
   }
}

//##############################################################################
// function writeGroupXML
//    input:   int - group number
//    input:   double - minimum peptide probability
//    input:   int - protein group index
//    input:   ofstream - file handle for writing output
//    output:  none
//##############################################################################
void writeGroupXML( int grp_num, double min_pep_prob, int index, ogzstream& outfile )
{
   int group_ind = grp_indices[ grp_num ];
   outfile << "<protein_group group_number=\"" << index << "\" pseudo_name=\"" << grp_num+1 ;
   outfile << "\" probability=\"" << std::setprecision( 4 ) << group_probs[ group_ind ] << "\">" << std::endl;
   char id[8] = {'a',0,0,0,0,0,0,0};
   int iInc = 0;
   for ( unsigned int g = 0; g < protein_groups[ group_ind ].size(); g++ )
   {
      std::string index_str;
      format_int( index, index_str );
      index_str += "-";
      std::string index_str2;
      format_int( ( g + 1 ), index_str2 );
      index_str += index_str2;
      std::string id_str(id);
      writeProteinXML( protein_groups[ group_ind ][ g ], index_str, min_pep_prob, id_str, outfile );

      // Base-26 ID counting.
      while (iInc >= 0 && id[iInc] == 'z')
          id[iInc--] = 'a';
      if (iInc < 0)
      {
          while (id[++iInc])
              id[iInc] = 'a';
          id[iInc] = 'a';
      }
      else
      {
          id[iInc]++;
          iInc = strlen(id) - 1;
      }
   }
   outfile << "</protein_group>" << std::endl;
}

//##############################################################################
// function getUniqueStrippedPeps
//    input:   vector< ppPeptide* >& - peptides to be stripped of everything but uppercase alpha characters
//    output:  string - concatentation of stripped peptides ('+' used as concatenation symbol)
//##############################################################################
void getUniqueStrippedPeps( const std::vector< ppPeptide* >& peps, std::string &output )
{
   output.clear();
   std::set< std::string > uniques;
   for ( unsigned int i = 0; i < peps.size(); i++ )
   {
      const char *pep = peps[ i ] -> getName();
      if ( equivalent_peps.find( pep ) != equivalent_peps.end() )
      {
         StringSet& act_peps_ref = equivalent_peps[ pep ];
         for ( StringSet::const_iterator act_peps_iter = act_peps_ref.begin(); act_peps_iter != act_peps_ref.end(); ++act_peps_iter )
         {
            uniques.insert( strip( *act_peps_iter ) );
         }
      }
      else
      {
         uniques.insert( strip( pep ) );
      }
   }
   std::set< std::string >::const_iterator uniques_iter = uniques.begin();
   while ( uniques_iter != uniques.end() )
   {
      output += *uniques_iter;
      ++uniques_iter;
      if ( uniques_iter != uniques.end() )
      {
         output += "+";
      }
   }
}

//##############################################################################
// function getTotalNumPeps
//    input:   string - protein name
//    input:   vector< ppPeptide* >& - peptides to be checked for counting toward total for protein
//    output:  int - total number of instances of all peptides found in sequence of protein 
//##############################################################################
int getTotalNumPeps( const ppProtein *prot, const std::vector< ppPeptide* >& peps )
{
   int tot = 0;
   for ( unsigned int i = peps.size(); i--; )
   {
      ppPeptide* pep = peps[ i ];
      ppParentProtein &protref = pep->getParentProtein(prot);
      if (GROUPWTS) {
	if ( ( protref.getMaxProb() >= FINAL_PROB_MIN_PROB ) && ( protref.getGroupPepWt() >= FINAL_PROB_MIN_WT ) )
	  {
	    tot += protref.getNumInsts();
	  }
      }
      else {
	if ( ( protref.getMaxProb() >= FINAL_PROB_MIN_PROB ) && ( protref.getPepWt() >= FINAL_PROB_MIN_WT ) )
	  {
	    tot += protref.getNumInsts();
	  }
      }
   }
   return tot;
}

int getTotalNumDistinctPeps( const ppProtein *prot, const std::vector< ppPeptide* >& peps )
{
   int tot = 0;
   std::map< std::string, int > processed_peps;

   for ( unsigned int i = peps.size(); i--; )
   {
      ppPeptide* pep = peps[ i ];
      std::string pep_str = equivIProphPeptide( substitution_aas, equivalent_peps, pep->getName(), pep->getName(), false );

      std::map< std::string, int >::const_iterator iter;
      
      if ((iter = processed_peps.find(pep_str)) == processed_peps.end()) {

	processed_peps.insert( std::make_pair( pep_str, 1) );

	ppParentProtein &protref = pep->getParentProtein(prot);
	if (GROUPWTS) {
	  if ( ( protref.getMaxProb() >= FINAL_PROB_MIN_PROB ) && ( protref.getGroupPepWt() >= FINAL_PROB_MIN_WT ) )
	    {
	      tot += protref.getNumInsts();
	    }
	}
	else {
	  if ( ( protref.getMaxProb() >= FINAL_PROB_MIN_PROB ) && ( protref.getPepWt() >= FINAL_PROB_MIN_WT ) )
	    {
	      tot += protref.getNumInsts();
	    }
	}

	
      }
      
   }
   return tot;
}



int getTotalUniqPeps( const ppProtein *prot )
{
   int tot = 0;
   const orderedPeptideList& pip_ref = prot -> getPeptidesInProt();
   for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )
   {
      ppPeptide* pep = *pip_iter;
      ppParentProtein &protref = pep->getParentProtein(prot);

      if (GROUPWTS) {

	if ( ( protref.getMaxProb() >= FINAL_PROB_MIN_PROB ) && ( protref.getGroupPepWt() >= FINAL_PROB_MIN_WT ) )
	  {
	    tot++;
	  } 

      }
      else {
	if ( ( protref.getMaxProb() >= FINAL_PROB_MIN_PROB ) && ( protref.getPepWt() >= FINAL_PROB_MIN_WT ) )
	  {
	    tot++;
	  } 
      }
   }
   return tot;
}

int getTotalNumPeps( const ppProtein *prot )
{
   int tot = 0;
   const orderedPeptideList& pip_ref = prot -> getPeptidesInProt();
   for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )
   {
      ppPeptide* pep = *pip_iter;
      ppParentProtein &protref = pep->getParentProtein(prot);

      if (GROUPWTS) {

	if ( ( protref.getMaxProb() >= FINAL_PROB_MIN_PROB ) && ( protref.getGroupPepWt() >= FINAL_PROB_MIN_WT ) )
	  {
	    tot += protref.getNumInsts();
	  }

      }
      else {
	if ( ( protref.getMaxProb() >= FINAL_PROB_MIN_PROB ) && ( protref.getPepWt() >= FINAL_PROB_MIN_WT ) )
	  {
	    tot += protref.getNumInsts();
	  }
      }
   }
   return tot;
}

//##############################################################################
// function writeProteinXML
//    input:   string - contains concatenation of names of all proteins in a degenerate group   //&& nomenclature?
//    input:   string - contains group index and index for entry                                //&& ???
//    input:   double - minimum peptide probability
//    input:   string - protein ID                                                              //&& ???
//    input:   ofstream - file handle for writing output
//    output:  none
//##############################################################################
class PepSorter { // helper class for sorting by probability desc and name asc
public:
   PepSorter(ppPeptide *inpep,double inprob) :
      pep(inpep),probRank(rankProtProbability(inprob)) {};
      ppPeptide *pep;
      int probRank;
};

bool ltpepsorter(const PepSorter &s1, const PepSorter &s2) {
   return (s1.probRank> s2.probRank)?1:
     ((s1.probRank==s2.probRank)?(strcmp(s1.pep->getName(),s2.pep->getName())<0):0);
}

void computeProteinConfidence ( ppProtein * entry_prot, const std::string& index, double min_pep_prob, const std::string& prot_id) {
  const degenList &entries = entry_prot->getDegenList();
   const orderedPeptideList& pip_ref = entry_prot -> getPeptidesInProt();
   if ( pip_ref.empty() )
     {
       return;
     }
   
   // do selection sort to order peptides according their max probs for this protein (highest prob first)
   std::vector< PepSorter > sortpeps;
   for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )
     {
       sortpeps.push_back( PepSorter(*pip_iter,(*pip_iter)->getParentProtein(entry_prot).getMaxProb() ));
     }
   std::sort(sortpeps.begin(),sortpeps.end(),ltpepsorter);
   std::vector<ppPeptide *> peps;
   for (unsigned int p=0;p<sortpeps.size();p++) {
     peps.push_back(sortpeps[p].pep);
   }  
   if (PROTLEN) {
    
    unsigned long minLen = 0;
    GammaDistribution* gamma = new GammaDistribution(0);
    
    for (std::vector<ppProtein *>::size_type e=0; e<entries.size(); e++) {
      if (e==0 || entries[e]->getProteinLen() < minLen) {
	minLen = entries[e]->getProteinLen();
      }
    }

     if (minLen == 0) {
       minLen = mean_prot_len;
     }

     double lambda = countTheoretPeps(minLen) * MU;
          
     if (LOGPROBS) {
       lambda = minLen * MU;
     }
     
     double sum=0;
     double Prand=-1;
     double Expect=0;
     
     
     //int j=0;
     double totprob=0;
     
     for (std::vector<ppProtein *>::size_type j = 0; j < peps.size(); j++) {
       //if ( peps[ j ] -> getParentProtein( entry_prot ).getMaxProb() > 0.5)
       if (peps[ j ] -> getParentProtein( entry_prot ).getOrigMaxProb() >= MIN_DATA_PROB) {
	 if (LOGPROBS) {
	   totprob += log10(1-peps[ j ] -> getParentProtein( entry_prot ).getOrigMaxProb())*-1.;
	 }
	 else {
	   totprob += peps[ j ] -> getParentProtein( entry_prot ).getOrigMaxProb();
	 }
       }
       //       totprob += peps[ j ] -> getParentProtein( entry_prot ).getPepWt()* peps[ j ] -> getParentProtein( entry_prot ).getMaxProb();
     }     
     
     double wid = 0.05;
     double j = totprob;
     
     while (sum - Prand > 1.e-20) {
       Prand = sum;
       double a = pow(lambda, j);
       double b = exp(gamma->gammln(j+1));
       double c = exp((-1.)*lambda);
       sum += (wid * a *c / b);
       j+=wid;
     }
     Prand = sum;
     //     Prand = 1 - sum;
     Expect = Prand * TOT_DB_SEQ;
     
     
     delete gamma;
     
     entry_prot->setConfidence(1 / (1 + Expect));
   

   }

}

void writeProteinXML( ppProtein * entry_prot, const std::string& index, double min_pep_prob, const std::string& prot_id, ogzstream& outfile )
{
/*
   my $grp_index = '';
   if($index =~ /^(\d+)(\-\d+)$/) {
       $grp_index = $1;
       $index = $2;
   }
   else {
       $grp_index = 0 - $index if($index < 0);
       $index = '' if($index < 0);
   }
*/
   const orderedPeptideList& pip_ref = entry_prot -> getPeptidesInProt();
   if ( pip_ref.empty() )
   {
      return;
   }

   // do selection sort to order peptides according their max probs for this protein (highest prob first)
   std::vector< PepSorter > sortpeps;
   for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )
   {
      sortpeps.push_back( PepSorter(*pip_iter,(*pip_iter)->getParentProtein(entry_prot).getMaxProb() ));
   }
   std::sort(sortpeps.begin(),sortpeps.end(),ltpepsorter);
   std::vector<ppPeptide *> peps;
   for (unsigned int p=0;p<sortpeps.size();p++) {
      peps.push_back(sortpeps[p].pep);
   }

   const degenList &entries = entry_prot->getDegenList();
   spaceXML( 2, outfile );	
   outfile << "<protein protein_name=\"" << entries[ 0 ]->getName() << "\" n_indistinguishable_proteins=\"" << entries.size() << "\" probability=\"";
 
   
   outfile << std::setprecision( 4 ) << entry_prot -> getProbability() << "\"";
   if ( PRINT_PROT_COVERAGE > 0 )
   {
      double cov = getCoverageForEntry( entry_prot, PRINT_PROT_COVERAGE );
      if ( isInit(cov) )
      {
         outfile << " percent_coverage=\"" << std::setprecision( 1 ) << cov << "\"";
      }
   }

 

   std::string uniquestrippedpeps;
   getUniqueStrippedPeps( peps, uniquestrippedpeps );
   outfile << " unique_stripped_peptides=\"" << uniquestrippedpeps << "\"";
   outfile << " group_sibling_id=\"" << prot_id << "\"";
   outfile << " total_number_peptides=\"" << getTotalNumPeps( entry_prot, peps ) << "\"";
   outfile << " total_number_distinct_peptides=\"" << getTotalNumDistinctPeps( entry_prot, peps ) << "\"";
   if ( entry_prot -> getSubsumingProtein() )
   {
      outfile << " subsuming_protein_entry=\"" << entry_prot -> getSubsumingProtein()->getName() << "\"";
   }
   if ( COMPUTE_TOTAL_SPECTRUM_COUNTS && total_spectrum_counts > 0.0 && entry_prot -> getProbability() > 0.0)
   {
       double tot_cnts = 0.0;
       for ( unsigned int i = 0; i < peps.size(); i++ )
       {
          double counts = peps[ i ] -> getSpectrumCounts();
          if ( counts > 0.0 )
          {
             tot_cnts += counts * peps[ i ] -> getParentProtein( entry_prot ).getPepWt();
          }
       }
       if ( total_spectrum_counts > 10000.0 )
       {
          outfile << std::setprecision( 3 );       // # add extra decimal place
       }
       else
       {
          outfile << std::setprecision( 2 );
       }
       outfile << " pct_spectrum_ids=\"" << tot_cnts * 100.00 / total_spectrum_counts << "\"";
   }

   
   if (PROTLEN) {

     outfile << " confidence=\"" << entry_prot->getConfidence() << "\"";


   }

   bool NEW_ASAP = true;   // # object for ASAP data
   if ( NEW_ASAP )
   {
      outfile << ">" << std::endl;
   }

   if (PROTMW) {
     
     spaceXML( 3, outfile );
     outfile << "<parameter name=\"mol_weight\" value=\"" <<  entries[ 0 ]->getProteinMW() << "\"/>" << endl;
   }

   if (PROTLEN || NORMPROTLEN) {
     
     spaceXML( 3, outfile );
     outfile << "<parameter name=\"prot_length\" value=\"" <<  entries[ 0 ]->getProteinLen() << "\"/>" << endl;
   }

/*                                                       //&& TODO this block can't run because getASAPRatio() calls a non-existent executable
   if($ASAP) {
      my $result = getASAPRatio($ASAP_IND, $entry, \@peps);
      my $pro_index = '';
      if($result =~ /proIndx\=(\d+)/) {
         $pro_index = $1;
      }

      # now strip off html tags
      while($result =~ /(.*)\<.*?\>(.*)/) {
         $result = $1 . $2;
      }
      if($NEW_ASAP && $result =~ /ASAP\:\s+(\S+)\s+\+\-\s+(\S+)\s+\(\S+\)\s+(\d+)\s?$/) {
         spaceXML( 3, outfile );	
         outfile << " " <ASAPRatio ratio_mean=\"%0.2f\" ratio_standard_dev=\"%0.2f\" ratio_number_peptides=\"%d\"", $1, $2, $3;
         outfile << " ' index="' . $pro_index . '"';
         outfile << " '/>';
         spaceXML( 3, outfile );	
      }
      elsif(! $NEW_ASAP) {
         outfile << " ' quantitation_ratio="', $result, '"';
      }
   }
   if(! $NEW_ASAP) {
      outfile << " '>';
   }
*/
   // # annotation
   std::string prot_descr;
   std::string ipi;
   std::string refseq;
   std::string swissprot;
   std::string ensembl;
   std::string trembl;
   if ( ANNOTATION )
   {
      // getAnnotation($entry, $database, -1);           //&& ignore for now since input comes from XML ( XML_INPUT == true )
      if ( IPI_DATABASE )
      {
         spaceXML( 3, outfile );
         std::string next_annot = entries[ 0 ] -> getAnnotation();
         if ( next_annot.empty() )
         {
            next_annot = entries[ 0 ]->getName();
         }
         parseIPI( next_annot, prot_descr, ipi, refseq, swissprot, ensembl, trembl );
         outfile << "<annotation protein_description=\"" << prot_descr << "\"";
         if ( ipi.length() )
         {
            outfile << " ipi_name=\"" << ipi << "\"";
         }
         if ( refseq.length() )
         {
            outfile << " refseq_name=\"" << refseq << "\"";
         }
         if ( swissprot.length() )
         {
            outfile << " swissprot_name=\"" << swissprot << "\"";
         }
         if ( ensembl.length() )
         {
            outfile << " ensembl_name=\"" << ensembl << "\"";
         }
         if ( trembl.length() )
         {
            outfile << " trembl_name=\"" << trembl << "\"";
         }
         outfile << "/>";
      }
      else if ( DROSOPHILA_DATABASE )
      {
	      spaceXML( 3, outfile );
         std::string next_annot = entries[ 0 ] -> getAnnotation();
	      outfile << " <annotation protein_description=\"" << next_annot << "\"";
         if(rX_flybase.Search(next_annot)) {
            outfile << "  flybase=\"" << rX_flybase[1] << "\"";
         }
	      outfile << "/>";
      }
      else 
      {
         spaceXML( 3, outfile );
         std::string next_annot = entries[ 0 ] -> getAnnotation();
         if ( next_annot.empty() ) {
            next_annot = entries[ 0 ]->getName();
         }
         outfile << "<annotation protein_description=\"" << cleanUpProteinDescription(next_annot) << "\"";
         outfile << "/>";
      }
   }
   outfile << std::endl;

   // # indistinguishables
   for ( unsigned int k = 1; k < entries.size(); k++ )
   {
      spaceXML( 3, outfile );
      outfile << "<indistinguishable_protein protein_name=\"" << entries[ k ]->getName() << "\">";
      if (PROTMW) {
	outfile << endl;
	spaceXML( 3, outfile );
	outfile << "<parameter name=\"mol_weight\" value=\"" <<  entries[ k ]->getProteinMW() << "\"/>";
      }
      if ( ANNOTATION )
      {                                               // # && exists $annotation{$entries[$k]})    //&& comment from Perl code
         if ( IPI_DATABASE )
         {
            outfile << std::endl;
            spaceXML( 4, outfile );
            std::string next_annot;
            ppProtein *prot=entries[ k ] ;
            if ( prot -> getAnnotation().length() )
            {
               next_annot = prot -> getAnnotation();
            }
            else
            {
               next_annot = prot -> getName();
            }
            parseIPI( next_annot, prot_descr, ipi, refseq, swissprot, ensembl, trembl );
            outfile << "<annotation protein_description=\"" << prot_descr << "\"";
            if ( ipi.length() )
            {
               outfile << " ipi_name=\"" << ipi << "\"";
            }
            if ( refseq.length() )
            {
               outfile << " refseq_name=\"" << refseq << "\"";
            }
            if ( swissprot.length() )
            {
               outfile << " swissprot_name=\"" << swissprot << "\"";
            }
            if ( ensembl.length() )
            {
               outfile << " ensembl_name=\"" << ensembl << "\"";
            }
            if ( trembl.length() )
            {
               outfile << " trembl_name=\"" << trembl << "\"";
            }
            outfile << "/>" <<  std::endl;
/*         }                                                    //&& worry about this section later
         else if(exists $annotation{$entries[$k]} && (length $annotation{$entries[0]}) > 0)
         {
            outfile << " "\n";
            spaceXML( 4, outfile );
            my @description = split('\n', $annotation{$entries[$k]});
            outfile << " "<annotation protein_description=\"", join(' ', @description), "\"/>";
*/
         }

/* James was here... */

         else
         {
            outfile << std::endl;
            spaceXML( 4, outfile );
            std::string next_annot;
            ppProtein *prot=entries[ k ] ;
            if ( prot -> getAnnotation().length() )
            {
               next_annot = prot -> getAnnotation();
            }
            else
            {
               next_annot = prot -> getName();
            }

            outfile << "<annotation protein_description=\"" << cleanUpProteinDescription(next_annot) << "\"";

            outfile << "/>" << std::endl;
            

	 }

/* James has left... */

         spaceXML( 3, outfile );
      }
      outfile << "</indistinguishable_protein>" << std::endl;
   }
   std::map< std::string, std::string > pep_grp_desigs;            //&& should this be made a data member of Peptide?
   findCommonPeps( peps, pep_grp_desigs );

   for ( unsigned int j = 0; j < peps.size(); j++ )
   {
      ppPeptide* pep = peps[ j ];
      std::string pep_str(pep -> getName());
      ppParentProtein &parentProt = pep->getParentProtein(entry_prot);
      double pep_prob;
      if ( !parentProt.hasMaxProb() || 
         (pep_prob = parentProt.getMaxProb()) < min_pep_prob )        // # skip if not high enough prob
      {
         continue;
      }
      //      double pep_wt = parentProt.getPepWt(); 
      double pep_wt = GROUPWTS? parentProt.getGroupPepWt() : parentProt.getPepWt();
      
      spaceXML( 3, outfile );
      outfile << "<peptide ";
      std::string color =  pep_prob >= FINAL_PROB_MIN_PROB && pep_wt >= FINAL_PROB_MIN_WT ? "Y" : "N";
      std::string std_pep;
      std::string pep_seq;
      double nterm = 0.0;
      double cterm = 0.0;
      ModsMap mods;
      double pepmass = 0.0;
      // # must deal with the equivalent peptide names ...
      std::vector< std::string > actualpeps;
      if ( equivalent_peps.find( pep_str ) != equivalent_peps.end() )
      {
         StringSet& act_peps_ref = equivalent_peps[ pep_str ];       // convert STL set of strings to vector of strings for local use
         for ( StringSet::const_iterator act_peps_iter = act_peps_ref.begin(); act_peps_iter != act_peps_ref.end(); ++act_peps_iter )
         {
            actualpeps.push_back( *act_peps_iter );
         }
      }

      if (IPROPHET && ! actualpeps.empty()) {
	pep_seq = strip(actualpeps[ 0 ].substr( 2, actualpeps[ 0 ].size() - 2 ));
      }
      else if ( ! actualpeps.empty() )
      {
	pep_seq = actualpeps[ 0 ].substr( 2, actualpeps[ 0 ].size() - 2 );
      }
      else
      {
	if (IPROPHET) {
	  pep_seq = pep_str;
	}
	else {
	  pep_seq = pep_str.substr( 2, pep_str.size() - 2 );
	}
      }
	

      if ( USE_STD_MOD_NAMES )
      {
         std_pep = pep_seq;
   	   // # must recreate original stripped peptide from stdp eptide name, also get back modification info
   	   // # write out modification below
         interpretStdModifiedPeptide( std_pep, pep_seq, nterm, cterm, mods );
         pepmass = pep -> getPeptideMass();
      }
      outfile << "peptide_sequence=\"" << pep_seq << "\" ";

      if ( ! USE_STD_MOD_NAMES && pep_seq.find( '#' ) != std::string::npos )
      {
         for ( unsigned int i = 0; i < pep_seq.size(); i++ )
         {
            if ( pep_seq[ i ] == '#' )
            {
               pep_seq[ i ] = '~';
            }
         }
   	   outfile << "pound_subst_peptide_sequence=\"" << pep_seq << "\" ";
       }
      if (!IPROPHET) {
	outfile << "charge=\"" << pep_str.substr( 0, 1 ) << "\" ";
      }
      else {
	outfile << "charge=\"0\" ";
      }
	outfile << "initial_probability=\"" << std::setprecision( 4 ) << parentProt.getOrigMaxProb() << "\" ";
       if (INSTANCES)
	 outfile << "ni_adjusted_probability=\"" << std::setprecision( 4 ) << parentProt.getInstsMaxProb() << "\" ";
       outfile << "nsp_adjusted_probability=\"" << std::setprecision( 4 ) << parentProt.getMaxProb() << "\" ";
       outfile << "fpkm_adjusted_probability=\"" << std::setprecision( 4 ) << parentProt.getMaxProb() << "\" ";
       if ( pep_grp_desigs.find( pep_str ) != pep_grp_desigs.end() )
       {
          outfile << "peptide_group_designator=\"" << pep_grp_desigs[ pep_str ] << "\" ";
       }
       outfile << "weight=\"" << std::setprecision( 2 ) << parentProt.getPepWt() << "\" ";
       outfile << "group_weight=\"" << std::setprecision( 2 ) << parentProt.getGroupPepWt() << "\" ";
       outfile << "is_nondegenerate_evidence=\"" << Bool2Alpha( getNumberDegenProteins( pep ) == 1 ) << "\" ";
       outfile << "n_enzymatic_termini=\"" << maxNTT( entry_prot, pep ) << "\" ";
       outfile << "n_sibling_peptides=\"" << std::setprecision( 2 ) << parentProt.getEstNSP() << "\" ";
       outfile << "n_sibling_peptides_bin=\"" << parentProt.getNSPBin() << "\" ";
       if (USE_FPKM) {
	 outfile << "max_fpkm=\"" << std::setprecision( 2 ) << parentProt.getFPKM() << "\" ";
         outfile << "fpkm_bin=\"" << parentProt.getFPKMBin() << "\" ";
       }

       if (INSTANCES )
	 outfile << "exp_sibling_ion_instances=\"" << std::setprecision( 2 ) << parentProt.getExpNI() << "\" "
		 << "exp_sibling_ion_bin=\"" << std::setprecision( 2 ) << parentProt.getNIBin() << "\" ";
       outfile << "n_instances=\"" << parentProt.getNumInsts() << "\" ";
       outfile << "exp_tot_instances=\"" << parentProt.getInstsTotProb() << "\" ";
       outfile << "is_contributing_evidence=\"" << color << "\"";
       if ( USE_STD_MOD_NAMES && !IPROPHET)
       {
         if( pep -> getPeptideMass() > 0.0 )
         {
            outfile << " calc_neutral_pep_mass=\"" << std::setprecision( 4 ) << pep -> getPeptideMass() << "\"";
         }
         else
         {
            outfile << " calc_neutral_pep_mass=\"0\"";
            //&& print "error: no peptide mass available for $peps[$pep]\n";
         }
      }	    
      if ( track_peptide_origin )
      {
         outfile << " spectrum_query_index=\"" << pep -> getSpectrumQueryIndex() << "\"";
         outfile << " hit_rank=\"" << pep ->getHitRank() << "\"";
	   }
      outfile << ">" << std::endl;

      // # mod info
      bool modified = nterm > 0.0 || cterm > 0.0 || mods.size() > 0;
      if( USE_STD_MOD_NAMES && modified )
      {
	      outfile << "<modification_info";
         if ( nterm > 0.0 )
         {
            outfile << " mod_nterm_mass=\"" << nterm << "\"";
         }
         if ( cterm > 0.0 )
         {
            outfile << " mod_cterm_mass=\"" << cterm << "\"";
         }
         outfile << " modified_peptide=\"" << streamlineStdModifiedPeptide( std_pep ) << "\"";
         outfile << ">" << std::endl;
         ModsMap::const_iterator mods_iter = mods.begin();
         while ( mods_iter != mods.end() )
         {
            outfile << "<mod_aminoacid_mass position=\"" << mods_iter -> first << "\" mass=\"" << std::setprecision( 6 ) << mods_iter -> second << "\"/>" << std::endl;
            ++mods_iter;
         }
/*	      foreach(sort {$a <=> $b} keys %{$mods})                  //&& the C++ hash appears to take care of maintaining ascending sort order automatically
         {
	          outfile << " "<mod_aminoacid_mass position=\"%d\" mass=\"%f\"/>\n", $_, ${$mods}{$_};
	      }
*/
         outfile << "</modification_info>" << std::endl;
      }

      // # parent protein info....
      const ppParentProteinList& pmp_ref = pep -> getParentProteinList();
      std::vector<const ppProtein *>prots; // for alpha sort
      for ( ppParentProteinList::const_iterator pmp_iter = pmp_ref.begin(); pmp_iter != pmp_ref.end(); ++pmp_iter )
      {
         ppProtein * prot = pmp_iter -> first;
	      // # only record additional proteins (not also in an indistinguishable group)
         if (  pmp_iter->second->hasMaxProb() && (prot!=entry_prot) && ! ( entry_prot -> isDegen() ) )
         {
            prots.push_back(prot->getDegenList()[0]);
         }
      }
      std::sort(prots.begin(),prots.end(),compareProteinNamesAsc);
      for (unsigned int nm=0;nm<prots.size();nm++) {
         spaceXML( 4, outfile );
         outfile << "<peptide_parent_protein ";
         outfile << "protein_name=\"" << prots[nm]->getName() << "\"/>" << std::endl;
      }

//       # must deal with the equivalent peptide names....
      const char *pepname = pep -> getName();
      if ( equivalent_peps.find( pepname ) != equivalent_peps.end() )
      {
         StringSet& act_peps_ref = equivalent_peps[ pepname ];
	 StringSet::const_iterator act_peps_iter; 
         for ( act_peps_iter = act_peps_ref.begin(); act_peps_iter != act_peps_ref.end(); ++act_peps_iter )
         {
//       if(exists $equivalent_peps{$peps[$pep]}) {
//	   my @actualpeps = keys %{$equivalent_peps{$peps[$pep]}};     //&& these are already extracted above
//	   for(my $k = 1; $k < @actualpeps; $k++) {
	   std::string stripped_alt=*act_peps_iter;
	   	
	   //  if (strstr(pepname,  "TGLGR")!=NULL ) {
	   //  cerr << "DDS: check!" << endl;
	   // }

	   if (!IPROPHET)
	     stripped_alt = ( strip( (*act_peps_iter).substr(2) ) );
	   
	   if (pep_seq!=stripped_alt) {
               spaceXML( 4, outfile );
               if(USE_STD_MOD_NAMES) {
                  // # write out modification below
		 outfile << " <indistinguishable_peptide peptide_sequence=\"" ;
		 if (!IPROPHET) {
		   outfile << stripped_alt  ;
		 }
		 else {
		   outfile << pep_seq ;
		 }

		 
		 outfile << "\" charge=\"" << (*act_peps_iter).substr(0,1) << "\"";

		 if (IPROPHET) {
		   outfile << " calc_neutral_pep_mass=\"" << equiv_pepmasses[ pepname ][act_peps_iter->substr(2)] <<"\"";
		   
		 }
		 
		 outfile << ">\n";
		 
		 if ((IPROPHET && strchr(act_peps_iter->substr(2).c_str(), '[') != NULL) || modified) {
		   spaceXML( 4, outfile );
		   outfile << " <modification_info modified_peptide=\"" << act_peps_iter->substr(2)<<"\"/>\n" ;
		 }
		 spaceXML( 4, outfile );
		 outfile << " </indistinguishable_peptide>\n";
               }
               else {
		 outfile << " <indistinguishable_peptide peptide_sequence=\"" << act_peps_iter->substr(2) << "\"/>\n";
               }
            }
         }
      }
      spaceXML( 3, outfile );
      outfile << "</peptide>" << std::endl;
   } // # next pep
   spaceXML( 2, outfile );
   outfile << "</protein>" << std::endl;
}
 
//##############################################################################
// function parseIPI
//    input:   string - unparsed annotation for a protein
//    input:   string& - string reference in which to place protein description
//    input:   string& - string reference in which to place IPI code of protein
//    input:   string& - string reference in which to place refseq code of protein
//    input:   string& - string reference in which to place swissprot code of protein
//    input:   string& - string reference in which to place ensembl code of protein
//    input:   string& - string reference in which to place trembl code of protein
//                         NOTE: function modifies external values of above seven inputs via reference
//    output:  none
//##############################################################################
boost::RegEx rX_ensemble_locations( "Ensembl_locations\\S+\\s+(\\S.*\\S)" );                      //&& Perl regex: $annot =~ /Ensembl\_locations\S+\s+(\S.*\S)/
boost::RegEx rX_IPI_tax_id_long( "IPI.*Tax_Id=\\d\\d*\\s+(\\S.*\\S)\\s\\s,\\s\\s\\s\\|" );     //&& Perl regex: $annot =~ /IPI.*?Tax\_Id\=\d\d*\s+(\S.*\S)\s?\s?\,?\s?\s?\s?\|/
boost::RegEx rX_IPI_tax_id_short( "IPI.*Tax_Id=\\d\\d*\\s+(\\S.*\\S)" );                        //&& Perl regex: $annot =~ /IPI.*?Tax\_Id\=\d\d*\s+(\S.*\S)/

boost::RegEx rX_refseq( "REFSEQ_[A-Z][A-Z]:([A-Za-z0-9_]+)[\\|\\s;]" );      //&& Perl regex: $annot =~ /REFSEQ\_[A-Z][A-Z]\:(\S+?)[\|,\s,\;]/
boost::RegEx rX_refseq_1( "REFSEQ:([A-Za-z0-9_]+)[\\|\\s;]" );
boost::RegEx rX_swissprot( "SWISS-PROT:([A-Za-z0-9_-]+)[\\|\\s;]" );            //&& Perl regex: $annot =~ /SWISS\-PROT\:(\S+?)[\|,\s,\;]/
boost::RegEx rX_trembl( "TREMBL:([A-Za-z0-9_]+)[\\|\\s;]" );                 //&& Perl regex: $annot =~ /TREMBL\:(\S+?)[\|,\s,\;]/
boost::RegEx rX_ensembl( "ENSEMBL:([A-Za-z0-9_]+)[\\|\\s;]" );                //&& Perl regex: $annot =~ /ENSEMBL\:(\S+?)[\|,\s,\;]/

boost::RegEx rX_IPI_0( "IPI:(IPI\\S+)\\.\\d[\\|\\s;]" );     //&& Perl regex: $annot =~ /IPI\:(IPI\S+)\.\d[\|,\s,\;]/
boost::RegEx rX_IPI_1( "(IPI\\S+)\\.\\d[\\|\\s;]" );         //&& Perl regex: $annot =~ /(IPI\S+)\.\d[\|,\s,\;]/
boost::RegEx rX_IPI_2( "IPI:(IPI\\S+)[\\|\\s;]" );           //&& Perl regex: $annot =~ /IPI\:(IPI\S+)[\|,\s,\;]/
boost::RegEx rX_IPI_3( "(IPI\\S+)[\\|\\s;]" );               //&& Perl regex: $annot =~ /(IPI\S+)[\|,\s,\;]/
boost::RegEx rX_IPI_4( "(IPI\\d+)$" );                       //&& Perl regex: $annot =~ /(IPI\d+)$/

void parseIPI(    const std::string& annot,
                  std::string& protein_description,
                  std::string& ipi,
                  std::string& refseq,
                  std::string& swissprot,
                  std::string& ensembl,
                  std::string& trembl
                  )
{
   protein_description.clear();
   ipi.clear();
   refseq.clear();
   swissprot.clear();
   ensembl.clear();
   trembl.clear();

   if ( rX_ensemble_locations.Search( annot ) )                                                        // # mouse IPI
   {
      protein_description = rX_ensemble_locations[ 1 ];
   }
   else if ( rX_IPI_tax_id_long.Search( annot ) )
   {
      protein_description = rX_IPI_tax_id_long[ 1 ];
   }
   else if ( rX_IPI_tax_id_short.Search( annot ) )
   {
      protein_description = rX_IPI_tax_id_short[ 1 ];
   }
   else
   {
      protein_description = annot;
   }

   if ( rX_refseq.Search( annot ) )
   {
      refseq = rX_refseq[ 1 ];
   }
   else if ( rX_refseq_1.Search( annot ) )
   {
      refseq = rX_refseq_1[ 1 ];
   }
   if ( rX_swissprot.Search( annot ) )
   {
      swissprot = rX_swissprot[ 1 ];
   }
   if ( rX_trembl.Search( annot ) )
   {
      trembl = rX_trembl[ 1 ];
   }
   if ( rX_ensembl.Search( annot ) )
   {
      ensembl = rX_ensembl[ 1 ];
   }

   
   if ( rX_IPI_0.Search( annot ) )
   {
      ipi = rX_IPI_0[ 1 ];
   }
   else if ( rX_IPI_1.Search( annot ) )
   {
      ipi = rX_IPI_1[ 1 ];
   }
   else if ( rX_IPI_2.Search( annot ) )
   {
      ipi = rX_IPI_2[ 1 ];
   }
   else if ( rX_IPI_3.Search( annot ) )
   {
      ipi = rX_IPI_3[ 1 ];
   }
   else if ( rX_IPI_4.Search( annot ) )
   {
      ipi = rX_IPI_4[ 1 ];
      if ( protein_description == ipi )
      {
         protein_description.clear();                          // # set to blank
      }
   }
}

void getProteinLens() {
   if ( ! fileExists( database ) )
   {
      std::cout << "WARNING: Cannot find database \"" << database << "\" for protein lengths. PROTLEN and NORMPROTLEN options are off." << std::endl;
      return;
      //      myexit( 1 );
   }
   pwiz::util::random_access_compressed_ifstream DB( database.c_str() ); // can read gzipped files
   unsigned int line_len = 524288;
   char* charbuf = new char[ line_len ];                                          // when this was [ 1024 ] processing of database terminated early
   unsigned long prot_len = 0;
   unsigned long prot_count = 0;
   unsigned long totprot_count = 0;
   unsigned long totprot_len = 0;
   ppProtein *prot = NULL;
   while( DB.getline( charbuf, line_len ) ) {
     if ('>'==*charbuf) {
       totprot_count++;
       if (prot != NULL) {
	 prot->setProteinLen(prot_len);
	 mean_prot_len += prot_len;
	 prot_count++;
       }
       if (LOGPROBS) {
	 totprot_len += prot_len;
       }
       else {
	 totprot_len += countTheoretPeps(prot_len);
       }
       totprot_len += prot_len;
       prot_len = 0;
       prot = NULL;
       char *space = strpbrk(charbuf," \t");
       if (space) {
	 *space = 0;
       } else {
	 space = strrchr(charbuf,'|');
	 if (space) {
	   *space = 0;
	 }
	 else {
	   space = strchr(charbuf,'\n');
	   if (space) {
	     *space = 0;
	   }
	 }
       }
       prot = proteinExists(charbuf+1); // are we likely to need this?
     }
     else {
       //Count totprot_len for all the proteins
       prot_len += strlen(charbuf);       
       //totprot_len += strlen(charbuf);
     }
   }
   if (prot != NULL) {
     prot->setProteinLen(prot_len);
     mean_prot_len += prot_len;
     prot_count++;
   }
   
   if (LOGPROBS) {
     totprot_len += prot_len;
   }
   else {
     totprot_len += countTheoretPeps(prot_len);
   }

   TOT_DB_LEN = totprot_len;
   TOT_DB_SEQ = totprot_count;
   if (prot_count) {
	mean_prot_len /= prot_count;
   } 
   delete [] charbuf;
   DB.close();

}

unsigned long countTheoretPeps(unsigned long prot_len) {
  if (!ALLPEPS) return prot_len;

  return (prot_len * prot_len - prot_len) / 2;

}

void getProteinMWs() {
   if ( ! fileExists( database ) )
   {
      std::cout << "WARNING: Cannot find database \"" << database << "\" for protein MW. PROTMW option is off." << std::endl;
      return;
      //      myexit( 1 );
   }
   pwiz::util::random_access_compressed_ifstream DB( database.c_str() ); // can read gzipped files
   unsigned int line_len = 524288;
   char* charbuf = new char[ line_len ];                                          // when this was [ 1024 ] processing of database terminated early
   double prot_mw = 0;
   ppProtein *prot = NULL;
   while( DB.getline( charbuf, line_len ) ) {
     if ('>'==*charbuf) {
       if (prot != NULL) {
	 prot->setProteinMW(prot_mw);
       }
       prot_mw = 0;
       prot = NULL;
       char *space = strchr(charbuf,' ');
       if (space) {
	 *space = 0;
       } else {
	 space = strchr(charbuf,'\n');
	 if (space) {
	   *space = 0;
	 }
       }
       prot = proteinExists(charbuf+1); // are we likely to need this?
     }
     else {
       prot_mw += ResidueMass::getProteinMass(charbuf, 1);
     }
   }
   if (prot != NULL) {
     prot->setProteinMW(prot_mw);
   }
   delete [] charbuf;
   DB.close();

}

//##############################################################################
// function writeCoverageInfo
//    input:   string - path name to use for output file
//    input:   double - minimum protein probability
//    input:   int - maximum number of proteins
//    output:  none
//##############################################################################
void writeCoverageInfo( const std::string& outfile, double min_prot_prob, int max_num_prots)
{
   // # order all db entries
   int num = 0;
   fast_map< int > db_no;
   ProteinProteinMap prot_degen_groups;
   std::vector< CoveredProt > cov_prots;     //&& using a vector instead of a set probably results in some redundant entries - should this be allowed, and does it matter?  BTW Perl code pushed everything into an array, such that redundant entries would be preserved

   if ( ! fileExists( database ) )
   {
      std::cout << "WARNING: Cannot find database \"" << database << "\" to write coverage information." << std::endl;
      return;
   }
   pwiz::util::random_access_compressed_ifstream DB( database.c_str() ); // can read gzipped files
   unsigned int line_len = 524288;
   char* charbuf = new char[ line_len ];                                          // when this was [ 1024 ] processing of database terminated early
   ppProtein *prot = NULL;
   while( DB.getline( charbuf, line_len ) ) {
     if ('>'==*charbuf) {
       prot = NULL;
       char *space = strchr(charbuf,' ');
       if (space) {
	 *space = 0;
       } else {
	 space = strchr(charbuf,'\n');
	 if (space) {
	   *space = 0;
	 }
       }
       prot = proteinExists(charbuf+1); // are we likely to need this?
       if (prot && db_no.find(charbuf+1)==db_no.end()) {
	 db_no.insert( std::make_pair(prot->getName(), num ) );
       }
       num++;
     }
   }
   delete [] charbuf;
   DB.close();

   // # compile list of all prots for cov
   std::vector< ppProtein* > prot_ptrs;
   proteinMap::const_iterator prot_iter = proteins.begin();
   while ( prot_iter != proteins.end() )
   {
      if (prot_iter -> second -> hasProbability() && 
         ( prot_iter -> second -> getProbability() >= min_prot_prob ))
      {
         prot_ptrs.push_back( prot_iter -> second );
      } // # above min prob 
      ++prot_iter;
   }
   std::sort( prot_ptrs.begin(), prot_ptrs.end(), compareProteinProbabilitiesDesc );    // sort proteins in order of descending probability
   for ( unsigned int i = 0; i < prot_ptrs.size(); i++ )
   {
      const std::vector< ppProtein *> &parsed = prot_ptrs[ i ] -> getDegenList();
      for ( int k = 0; k < (int)parsed.size() && k < max_num_prots; k++ )
      {
         fast_map<int>::iterator db_index = db_no.find( parsed[ k ]->getName() );
         if ( db_index  != db_no.end() )
         {
            CoveredProt cv;
            cv.protein = parsed[k];
            cv.db_index = db_index->second;
            cov_prots.push_back( cv );
            prot_degen_groups.insert( std::make_pair( parsed[ k ], prot_ptrs[ i ] ) );
         }
      }
   }
   // # sort by db_no
   std::sort( cov_prots.begin(), cov_prots.end(), compareCoveredProteinDBIndices );    // sort covered proteins in order of ascending IPI database index
   //&& @cov_prots = sort {$db_no{$a} <=> $db_no{$b}} @cov_prots;                      //&& replaced by line immediately above

   // # now write to file
   std::ofstream outFile( outfile.c_str() );
   for ( unsigned int j = 0; j < cov_prots.size(); j++ )
   {
      outFile << ">" << cov_prots[ j ].protein->getName() << std::endl;
      std::set< std::string > next;
      const orderedPeptideList& pip_ref =  prot_degen_groups[ cov_prots[ j ].protein ] -> getPeptidesInProt();     // get set whose keys are all peptides that appear in sequence of passed protein
      for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )                                                          // iterate through all peptides in set
      {
         const char *pep = ( *pip_iter ) -> getName();
         if ( equivalent_peps.find( pep ) != equivalent_peps.end() )
         {
            StringSet& act_peps_ref = equivalent_peps[ pep ];
            for ( StringSet::const_iterator act_peps_iter = act_peps_ref.begin(); act_peps_iter != act_peps_ref.end(); ++act_peps_iter )
            {
               next.insert( strip( *act_peps_iter ) );
            }
         }
         else
         {
            next.insert( strip( pep ) );
         }
      }
      for ( std::set< std::string >::const_iterator next_iter = next.begin(); next_iter != next.end(); ++next_iter)
      {
         outFile << *next_iter << std::endl;
      }
   }
   outFile.close();
}


//##############################################################################
// function computeCoverage
//    input:   double - minimum protein probability
//    input:   int - maximum number of proteins
//    output:  none
//##############################################################################
void computeCoverage( double min_prot_prob, int max_num_prots )
{
   std::string covinfofile = OUTFILE + ".covinfo";
   std::string covresultsfile = OUTFILE + ".cov";
   std::string coverage_exec = BINARY_DIRECTORY + "batchcoverage";
   if (!database.empty() && !fileExists( database ) )
   {  // look in the wwwroot
      std::string strtry = resolve_root(database.c_str());
      if ( fileExists(strtry)) {
         database = strtry;
      }
   }

   if (database.empty() || !fileExists( database ) )
   {
     std::cout << "WARNING: Cannot open DB \"" << database << "\". No coverage information is available." << std::endl;
     return;
     //      myexit( 1 );
   }
   writeCoverageInfo( covinfofile, min_prot_prob, max_num_prots );
   std::string command = coverage_exec + " \"" + database + "\" \"" + covinfofile + "\" \"" + covresultsfile + "\"";
   checked_system( command );
   if ( ! fileExists( covresultsfile ) )
   {
      std::cout << "* * * cannot find results file: " << covresultsfile << ", no coverage reporting possible * * *" << std::endl;
      myexit( 1 );
   }
   readCoverageResults( covresultsfile );
   verified_unlink(covinfofile);
   verified_unlink(covresultsfile);
}



//##############################################################################
// function readCoverageResults
//    input:   string - path name for file to read
//    output:  none
//##############################################################################
void readCoverageResults( std::string& infile )
{
   if ( ! fileExists( infile ) )
   {
      std::cout << "* * * cannot find results file: " << infile << ", no coverage reporting possible * * *" << std::endl;
      myexit( 1 );
   }
   // %coverage = ();     //&& not necessary to clear or initialize; taken care of by Protein class constructor
   pwiz::util::random_access_compressed_ifstream inFile( infile.c_str() ); // can read gzipped files
   char charbuf[ 256 ];
   boost::RegEx rX( "^(\\S+)\\s+(\\S+)" );
   while( inFile.getline( charbuf, sizeof( charbuf ) ) )
   {
      if ( rX.Search( charbuf ) )
      {
         getProteinByName( rX[ 1 ] ) -> setCoverage( atof( rX[ 2 ].c_str() ) );
      }
   }
   inFile.close();
}

//##############################################################################
// function getCoverageForEntry
//    input:   string - protein name (may be concatenated)
//    input:   int - if concatenated, maximum number of individual proteins to check for coverage 
//    output:  double - coverage for protein
//##############################################################################
double getCoverageForEntry( const ppProtein * entry, int max_num_prots )
{
   if (entry->hasCoverage()) {
      return entry->getCoverage();
   }
   const degenList & prots = entry->getDegenList();
   double max = UNINIT_VAL;
   for( int k = 0; k < (int)prots.size() && k < max_num_prots; k++ ) {
      if (prots[ k ]  -> hasCoverage()) {
         double cov = prots[ k ]  -> getCoverage();
         if (!isInit(max) || (cov > max )) {
            max = cov;
         }
      }
   }
   if (isInit(max)) {
      ((ppProtein *)entry) -> setCoverage( max ); // casting away const
   }
   return max;
}

double computeMU( ) {

  //MU is freq of false peptide matches
  //# false matches <= total matches
  //conservative MU = total matches / # proteins
  double exp_false_matches = 0;
  unsigned int grp_counter=0;
  proteinMap::const_iterator prot_iter = proteins.begin();
  proteinMap prots_done;

  while ( prot_iter != proteins.end() )
    {
      ppProtein* prot = prot_iter->second;
      
      if (prots_done.find(prot_iter->first) != prots_done.end()) {
	prot_iter++;     
	continue;
      }

      const degenList &entries = prot->getDegenList();
      
      for (int k=0;k<(int)entries.size();k++) {
	prots_done.insert( std::make_pair( entries[ k ]->getName(),  entries[ k ] ) );
      }

      const orderedPeptideList& pip_ref = prot -> getPeptidesInProt();     // get set whose keys are all peptides that appear in sequence of passed protein
      for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )                                         // iterate through all peptides in set
	{
	  ppPeptide* pep = *pip_iter;
	  ppParentProtein &parentProt = pep->getParentProtein(prot);
	  //exp_false_matches +=  parentProt.getPepWt() * (1 - parentProt.getOrigMaxProb());
	  if (parentProt.getOrigMaxProb() >= MIN_DATA_PROB) {
	    if (LOGPROBS) {
	      exp_false_matches +=  parentProt.getPepWt() * log10(parentProt.getOrigMaxProb()) * -1.;
	    }
	    else {
	      exp_false_matches +=  parentProt.getPepWt() * (1-parentProt.getOrigMaxProb());
	    }
	  }
	}

      prot_iter++;      
    }
   MU = exp_false_matches / (double) TOT_DB_LEN;
   MU *= MU_FACTOR;
   cerr << "INFO: mu=" << MU << ", db_size=" << TOT_DB_LEN << endl; 
  return MU;
}

//##############################################################################
// function updateProteinWeights
//    input:   bool - flag for whether to include groups
//    output:  bool - true if protein weights were updated, false otherwise
//##############################################################################
bool updateProteinWeights( bool include_groups )
{
  //if (GROUPWTS)
  //  include_groups = true;
   bool change = false;
   const double max_prob = 1.1;     // #0.85; #1.1; #0.9; #1.1;   //&& former values set in Perl code
   double PRIOR = 0.005;      // #2;                        //&& former value set in Perl code
   if ( ACCURACY_MODE )
   {
      PRIOR = 0.0;
   }
   const double MAX_DIFF = 0.005;   // #0.02;                     //&& former value set in Perl code

  	peptideMap::const_iterator pep_iter = peptides.begin();
   while ( pep_iter != peptides.end() )                              // process all peptides stored in peptides hash
   { 
      ppPeptide *pep = pep_iter -> second;
      double tot = 0.0;
      ppParentProteinList& parent_proteins_list = pep-> getParentProteinListNonConst();
      PRIOR = USE_WT_PRIORS ? 0.1 / parent_proteins_list.size() : PRIOR;
      ppParentProteinList::iterator parent_proteins_iter = parent_proteins_list.begin();
      while ( parent_proteins_iter != parent_proteins_list.end() )          // process all proteins in the ProteinPepWt hash of the peptide
      {
         const ppProtein *prot = parent_proteins_iter -> first;
         bool skip = include_groups && prot-> isMember() && ! prot-> isDegen();	    
         if( ! skip )
         {
            double next_prob = prot-> getProbability();
            if ( next_prob >= max_prob )
            {
               next_prob = 1.0;
            }
            tot += next_prob + PRIOR;
         }
         ++parent_proteins_iter;
      }     // # next protein
      parent_proteins_iter = parent_proteins_list.begin();
      while ( parent_proteins_iter != parent_proteins_list.end() )          // process all proteins in the ProteinPepWt hash of the peptide
      {
         const ppProtein *prot = parent_proteins_iter -> first;
         bool skip = include_groups && prot-> isMember() && ! prot-> isDegen();	    
         if( ! skip )
         {
            double next_prob = prot-> getProbability();
            double next_wt;
            if ( next_prob >= max_prob )
            {
               next_prob = 1.0;
            }
            if ( tot > 0.0 )
            {
               next_wt = ( next_prob + PRIOR ) / tot;
            }
            else if ( ! parent_proteins_list.empty() )                   //&& seems like this condition should always be true if execution reached this point
            {
               next_wt = 1.0 / parent_proteins_list.size();
            }
            else
            {
               next_wt = 0.0;
            }
//            next_wt = pow( next_wt, WT_POWER );  // restore this if WT_POWER <> 1
            if (fabs( next_wt - parent_proteins_iter -> second->getPepWt()) > MAX_DIFF ) {
               parent_proteins_iter -> second->setPepWt(next_wt);
	       parent_proteins_iter -> second->setGroupPepWt(next_wt);


               change = true;
            }
         }
         ++parent_proteins_iter;
      }     // # next protein
      ++pep_iter;
   }        // # next peptide
   return change;
}

//##############################################################################
// function iterate1
//    input:   int - maximum number of iterations to perform
//    output:  int - number of iterations actually performed         //&& need to check - count seems off by one
//##############################################################################
int iterate1( int max_iters )
{
   int counter = 1;
   updateProteinProbs();
   while( OCCAM && ( counter < max_iters ) && updateProteinWeights( false ) )
   {
      counter++;
      updateProteinWeights( false );      //&&  updateProteinWeights is executed TWICE for each increment of counter
   }
   return counter;
}

//##############################################################################
// function final_iterate
//    input:   int - maximum number of iterations to perform
//    output:  int - number of iterations actually performed
//##############################################################################
int final_iterate( int max_iters )
{
   int counter = 0;
   if ( ! OCCAM )
   {
      return counter;
   }
   computeFinalProbs();    // # with wts of degenerate groups
   while ( counter < max_iters && updateProteinWeights( true ) )
   {
      counter++;
      computeFinalProbs();
      if( !SILENT) {std::cout <<  " final update of weights and protein probabilities.....\n" ;}
   }
   return counter;
}

//##############################################################################
// function setPepMaxProb
//    input:   bool - flag for whether to use nsp
//    input:   bool - flag for whether to use joint probabilities
//    input:   bool - flag for whether to compute spectrum counts
//    output:  none
//##############################################################################
void setPepMaxProb( bool use_nsp, bool use_fpkm, bool use_joint_probs, bool compute_spectrum_cnts )
{
   // for all peptides, clear ProteinMaxProb hash; also clear ProteinOrigMaxProb hash, if use_nsp flag is false
  	peptideMap::const_iterator pep_iter = peptides.begin();
   while ( pep_iter != peptides.end() )                                 // process all peptides stored in peptides hash
   { 
      pep_iter -> second -> clearProteinMaxProb();
      if ( ! use_nsp )
      {
         pep_iter -> second -> clearProteinOrigMaxProb();
      }
      ++pep_iter;
   }        // # next peptide

   // local variables
   std::vector< const ppProtein *> prots1;
   std::vector< const ppProtein *> prots2;
   std::vector< const ppProtein *> prots3;
   std::vector< const ppProtein *> prots4;
   std::vector< const ppProtein *> prots5;
   std::vector< const ppProtein *> prots6;
   std::vector< const ppProtein *> prots7;
   std::vector< double > probs1;
   std::vector< double > probs2;
   std::vector< double > probs3;
   std::vector< double > probs4;
   std::vector< double > probs5;
   std::vector< double > probs6;
   std::vector< double > probs7;
   double prob = 0.0;
   double max2 = 0.0;     // # hold the maximum prob for all prots
   double max3 = 0.0;
   double max4 = 0.0;
   double max5 = 0.0;
   double max6 = 0.0;
   double max7 = 0.0;

   spectrumMap::const_iterator spec_iter = spectra.begin();
   while ( spec_iter != spectra.end() )
   {
	   max2 = 0.0;
	   max3 = 0.0;
	   max4 = 0.0;
	   max5 = 0.0;
	   max6 = 0.0;
	   max7 = 0.0;

      prob = 0.0;
      ppPeptide *pep2 = spec_iter -> second -> getSpecPeps( 0 );

      if ( pep2 )
      {
         prots2.clear();
         probs2.clear();
         prob = 0.0;
         max2 = 0.0;
         const ppParentProteinList& parent_proteins_list = pep2->getParentProteinList();
         ppParentProteinList::const_iterator parent_proteins_iter = parent_proteins_list.begin();
         while ( parent_proteins_iter != parent_proteins_list.end() )
         {
            const ppProtein *prot = parent_proteins_iter -> first;
            prots2.push_back( prot );
            ppParentProtein &parentProt = pep2->getParentProtein(prot);
            prob = ( spec_iter -> second -> getSpecProbs( 0, parentProt.getNTT() ) );
            if ( use_nsp )
            {
               prob = getNSPAdjustedProb( prob, parentProt.getNSPBin() );
            }
	    if ( use_fpkm )
            {
               prob = getFPKMAdjustedProb( prob, parentProt.getFPKMBin() );
            }
            probs2.push_back( prob );
            if ( use_joint_probs && prob > max2 )
            {
               max2 = prob;
            }
            ++parent_proteins_iter;
         }     // # next protein
      }
      ppPeptide *pep3 = spec_iter -> second -> getSpecPeps( 1 );

 

      if ( pep3 )
      {
         prots3.clear();
         probs3.clear();
         prob = 0.0;
         max3 = 0.0;
         const ppParentProteinList& parent_proteins_list = pep3->getParentProteinList();
         ppParentProteinList::const_iterator parent_proteins_iter = parent_proteins_list.begin();
         while ( parent_proteins_iter != parent_proteins_list.end() )
         {
            const ppProtein *prot = parent_proteins_iter -> first;
            prots3.push_back( prot );
            prob = ( spec_iter -> second -> getSpecProbs( 1, parent_proteins_iter->second->getNTT() ) );
            if ( use_nsp )
            {
               prob = getNSPAdjustedProb( prob, parent_proteins_iter->second->getNSPBin() );
            }
            probs3.push_back( prob );
            if ( use_joint_probs && prob > max3 )
            {
               max3 = prob;
            }
            ++parent_proteins_iter;
         }     // # next protein
      }
      ppPeptide *pep4 = spec_iter -> second -> getSpecPeps( 2 );
      if ( pep4 )
      {
         prots4.clear();
         probs4.clear();
         prob = 0.0;
         max4 = 0.0;
         const ppParentProteinList& parent_proteins_list = pep4->getParentProteinList();
         ppParentProteinList::const_iterator parent_proteins_iter = parent_proteins_list.begin();
         while ( parent_proteins_iter != parent_proteins_list.end() )
         {
            const ppProtein *prot = parent_proteins_iter -> first;
            prots4.push_back( prot );
            prob = ( spec_iter -> second -> getSpecProbs( 2, parent_proteins_iter->second->getNTT() ) );
            if ( use_nsp )
            {
               prob = getNSPAdjustedProb( prob, parent_proteins_iter->second->getNSPBin() );
            }
            probs4.push_back( prob );
            if ( use_joint_probs && prob > max4 )
            {
               max4 = prob;
            }
            ++parent_proteins_iter;
         }     // # next protein
      }
      ppPeptide *pep5 = spec_iter -> second -> getSpecPeps( 3 );
      if ( pep5 )
      {
         prots5.clear();
         probs5.clear();
         prob = 0.0;
         max5 = 0.0;
         const ppParentProteinList& parent_proteins_list = pep5->getParentProteinList();
         ppParentProteinList::const_iterator parent_proteins_iter = parent_proteins_list.begin();
         while ( parent_proteins_iter != parent_proteins_list.end() )
         {
            const ppProtein *prot = parent_proteins_iter -> first;
            prots5.push_back( prot );
            prob = ( spec_iter -> second -> getSpecProbs( 3, parent_proteins_iter->second->getNTT() ) );
            if ( use_nsp )
            {
               prob = getNSPAdjustedProb( prob, parent_proteins_iter->second->getNSPBin() );
            }
            probs5.push_back( prob );
            if ( use_joint_probs && prob > max5 )
            {
               max5 = prob;
            }
            ++parent_proteins_iter;
         }     // # next protein
      }

      ppPeptide *pep6 = spec_iter -> second -> getSpecPeps( 4 );
      if ( pep6 )
      {
         prots6.clear();
         probs6.clear();
         prob = 0.0;
         max6 = 0.0;
         const ppParentProteinList& parent_proteins_list = pep6->getParentProteinList();
         ppParentProteinList::const_iterator parent_proteins_iter = parent_proteins_list.begin();
         while ( parent_proteins_iter != parent_proteins_list.end() )
         {
            const ppProtein *prot = parent_proteins_iter -> first;
            prots6.push_back( prot );
            prob = ( spec_iter -> second -> getSpecProbs( 4, parent_proteins_iter->second->getNTT() ) );
            if ( use_nsp )
            {
               prob = getNSPAdjustedProb( prob, parent_proteins_iter->second->getNSPBin() );
            }
            probs6.push_back( prob );
            if ( use_joint_probs && prob > max6 )
            {
               max6 = prob;
            }
            ++parent_proteins_iter;
         }     // # next protein
      }
	
      ppPeptide *pep7 = spec_iter -> second -> getSpecPeps( 5 );
      if ( pep7 )
	{
	  prots7.clear();
	  probs7.clear();
	  prob = 0.0;
	  max7 = 0.0;
	  const ppParentProteinList& parent_proteins_list = pep7->getParentProteinList();
	  ppParentProteinList::const_iterator parent_proteins_iter = parent_proteins_list.begin();
	  while ( parent_proteins_iter != parent_proteins_list.end() )
	    {
	      const ppProtein *prot = parent_proteins_iter -> first;
	      prots7.push_back( prot );
	      prob = ( spec_iter -> second -> getSpecProbs( 5, parent_proteins_iter->second->getNTT() ) );
	      if ( use_nsp )
		{
		  prob = getNSPAdjustedProb( prob, parent_proteins_iter->second->getNSPBin() );
		}
	      probs7.push_back( prob );
	      if ( use_joint_probs && prob > max7 )
		{
		  max7 = prob;
		}
	      ++parent_proteins_iter;
	    }     // # next protein
	}
   

      // # now assign pep_max_probs using adjusted for joint 2+/3+/4+/5+ if specified
      if ( pep2 )
      {
         for ( unsigned int k = 0; k < prots2.size(); k++ )
         {
            const ppProtein *prot = prots2[ k ];
            prob = probs2[ k ];
            if( use_joint_probs && prob > 0.0 )                            // make joint 2+/3+/4+/5+ adjustments
            {
               double factor = prob + max3 + max4 + max5 + max6 + max7 >  1.0 ? ( prob + max3 + max4 + max5 + max6 + max7 -
								  prob * max3 - prob * max4 - prob * max5 - prob * max6 - prob * max7) / ( prob + max3 + max4 + max5 + max6 + max7) : 1.0;
               prob *= factor;
            }
            ppParentProtein& parentProt = pep2->getParentProtein(prot);
            if ( (!parentProt.hasMaxProb() ) || 
               ( prob > parentProt.getMaxProb() ) )
            {                                                              // if protein not in ProteinMaxProb hash, add along with its probability; or update existing protein's probability if appropriate
               parentProt.setMaxProb(prob);
               if ( ! use_nsp )
               {
                  parentProt.setOrigMaxProb(prob);                  // do same for ProteinOrigMaxProb hash
               }
            }
            if ( compute_spectrum_cnts && k == 0 )                         //&& only doing this for first protein in hash - not sure why
            {
               pep2 -> incrementSpectrumCounts( probs2[ k ] );
               total_spectrum_counts += probs2[ k ];
      	   }
         }     // # next protein
      }        //  end: if ( ! ( pep2.empty() ...
      if ( pep3 )
      {

         for ( unsigned int j = 0; j < prots3.size(); j++ )
         {
            const ppProtein *prot = prots3[ j ];
            prob = probs3[ j ];
            if( use_joint_probs && prob > 0.0 )                            // make joint 2+/3+/4+/5+ adjustments
            {
	      double factor = prob + max2 + max4 + max5 + max6 + max7 >  1.0 ? ( prob + max2 + max4 + max5 + max6 + max7 -
										 prob * max2 - prob * max4 - prob * max5 - prob * max6 - prob * max7) / ( prob + max2 + max4 + max5 + max6 + max7) : 1.0;            prob *= factor;
            }
            ppParentProtein& parentProt = pep3->getParentProtein(prot);
            if ( ( !parentProt.hasMaxProb() ) || 
                 ( prob > parentProt.getMaxProb() ) )
            {                                                              // if protein not in ProteinMaxProb hash, add along with its probability; or update existing protein's probability if appropriate
               parentProt.setMaxProb( prob );
               if ( ! use_nsp )
               {
                  parentProt.setOrigMaxProb(prob);                  // do same for ProteinOrigMaxProb hash
               }
            }
            if ( compute_spectrum_cnts && j == 0 )                              //&& only doing this for first protein in hash - not sure why
            {
               pep3->incrementSpectrumCounts( probs3[ j ] );
               total_spectrum_counts += probs3[ j ];
      	   }  // # if compute total spec counts
         }     // # next protein
      }        //  end: if ( ! ( pep3.empty() ...
      if ( pep4 )
      {

         for ( unsigned int j = 0; j < prots4.size(); j++ )
         {
            const ppProtein *prot = prots4[ j ];
            prob = probs4[ j ];
            if( use_joint_probs && prob > 0.0 )                            // make joint 2+/3+/4+/5+ adjustments
            {
	      double factor = prob + max2 + max3 + max5 + max6 + max7 >  1.0 ? ( prob + max2 + max3 + max5 + max6 + max7 -
										 prob * max2 - prob * max3 - prob * max5 - prob * max6 - prob * max7) / ( prob + max2 + max3 + max5 + max6 + max7) : 1.0;
	      
               prob *= factor;
            }
            ppParentProtein& parentProt = pep4->getParentProtein(prot);
            if ( ( !parentProt.hasMaxProb() ) || 
                 ( prob > parentProt.getMaxProb() ) )
            {                                                              // if protein not in ProteinMaxProb hash, add along with its probability; or update existing protein's probability if appropriate
               parentProt.setMaxProb( prob );
               if ( ! use_nsp )
               {
                  parentProt.setOrigMaxProb(prob);                  // do same for ProteinOrigMaxProb hash
               }
            }
            if ( compute_spectrum_cnts && j == 0 )                              //&& only doing this for first protein in hash - not sure why
            {
               pep4->incrementSpectrumCounts( probs4[ j ] );
               total_spectrum_counts += probs4[ j ];
      	   }  // # if compute total spec counts
         }     // # next protein
      }        //  end: if ( ! ( pep4.empty() ...
      if ( pep5 )
      {

         for ( unsigned int j = 0; j < prots5.size(); j++ )
         {
            const ppProtein *prot = prots5[ j ];
            prob = probs5[ j ];
            if( use_joint_probs && prob > 0.0 )                            // make joint 2+/3+/4+/5+ adjustments
            {
	        double factor = prob + max2 + max3 + max4 + max6 + max7 >  1.0 ? ( prob + max2 + max3 + max4 + max6 + max7 -
										 prob * max2 - prob * max3 - prob * max4 - prob * max6 - prob * max7) / ( prob + max2 + max3 + max4 + max6 + max7) : 1.0;               prob *= factor;
            }
            ppParentProtein& parentProt = pep5->getParentProtein(prot);
            if ( ( !parentProt.hasMaxProb() ) || 
                 ( prob > parentProt.getMaxProb() ) )
            {                                                              // if protein not in ProteinMaxProb hash, add along with its probability; or update existing protein's probability if appropriate
               parentProt.setMaxProb( prob );
               if ( ! use_nsp )
               {
                  parentProt.setOrigMaxProb(prob);                  // do same for ProteinOrigMaxProb hash
               }
            }
            if ( compute_spectrum_cnts && j == 0 )                              //&& only doing this for first protein in hash - not sure why
            {
               pep5->incrementSpectrumCounts( probs5[ j ] );
               total_spectrum_counts += probs5[ j ];
      	   }  // # if compute total spec counts
         }     // # next protein
      }        //  end: if ( ! ( pep5.empty() ...

      if ( pep6 )
      {

         for ( unsigned int j = 0; j < prots6.size(); j++ )
         {
            const ppProtein *prot = prots6[ j ];
            prob = probs6[ j ];
            if( use_joint_probs && prob > 0.0 )                            // make joint 2+/3+/5+/5+ adjustments
            {
	      double factor = prob + max2 + max3 + max4 + max5 + max7 >  1.0 ? ( prob + max2 + max3 + max4 + max5 + max7 -
										 prob * max2 - prob * max3 - prob * max4 - prob * max5 - prob * max7) / ( prob + max2 + max3 + max4 + max5 + max7) : 1.0;              
	      prob *= factor;
            }
            ppParentProtein& parentProt = pep6->getParentProtein(prot);
            if ( ( !parentProt.hasMaxProb() ) || 
                 ( prob > parentProt.getMaxProb() ) )
            {                                                              // if protein not in ProteinMaxProb hash, add along with its probability; or update existing protein's probability if appropriate
               parentProt.setMaxProb( prob );
               if ( ! use_nsp )
               {
                  parentProt.setOrigMaxProb(prob);                  // do same for ProteinOrigMaxProb hash
               }
            }
            if ( compute_spectrum_cnts && j == 0 )                              //&& only doing this for first protein in hash - not sure why
            {
               pep6->incrementSpectrumCounts( probs6[ j ] );
               total_spectrum_counts += probs6[ j ];
      	   }  // # if compute total spec counts
         }     // # next protein
      }        //  end: if ( ! ( pep5.empty() ...
      if ( pep7 )
	{
	  
	  for ( unsigned int j = 0; j < prots7.size(); j++ )
	    {
	      const ppProtein *prot = prots7[ j ];
	      prob = probs7[ j ];
	      if( use_joint_probs && prob > 0.0 )                            // make joint 2+/3+/5+/5+ adjustments
		{
		  double factor = prob + max2 + max3 + max4 + max5 + max6 >  1.0 ? ( prob + max2 + max3 + max4 + max5 + max6 -
										     prob * max2 - prob * max3 - prob * max4 - prob * max5 - prob * max6) / ( prob + max2 + max3 + max4 + max5 + max6) : 1.0;              
		  prob *= factor;
		}
	      ppParentProtein& parentProt = pep7->getParentProtein(prot);
	      if ( ( !parentProt.hasMaxProb() ) || 
		   ( prob > parentProt.getMaxProb() ) )
		{                                                              // if protein not in ProteinMaxProb hash, add along with its probability; or update existing protein's probability if appropriate
		  parentProt.setMaxProb( prob );
		  if ( ! use_nsp )
		    {
		      parentProt.setOrigMaxProb(prob);                  // do same for ProteinOrigMaxProb hash
		    }
		}
	      if ( compute_spectrum_cnts && j == 0 )                              //&& only doing this for first protein in hash - not sure why
		{
		  pep7->incrementSpectrumCounts( probs7[ j ] );
		  total_spectrum_counts += probs7[ j ];
		}  // # if compute total spec counts
	    }     // # next protein
	}        //  end: if ( ! ( pep5.empty() ...
      
      ++spec_iter;
   }
   // # now do the singly charged...
   spec_iter = singly_spectra.begin();
   while ( spec_iter != singly_spectra.end() )
   {
      ppPeptide *pep1 = spec_iter -> second -> getSinglySpecPeps();

      if ( pep1 )
      {
         prots1.clear();         //&& this array probably not needed
         probs1.clear();         //&& this array probably not needed
         prob = 0.0;
         ppParentProteinList& parent_proteins_list = pep1->getParentProteinListNonConst();

         ppParentProteinList::iterator parent_proteins_iter = parent_proteins_list.begin();
         while ( parent_proteins_iter != parent_proteins_list.end() )
         {
            const ppProtein *prot = parent_proteins_iter -> first;
            prots1.push_back( prot );     //&& values in this array do not appear to be used subsequently (in this translation of the code)
            prob = ( spec_iter -> second -> getSinglySpecProbs( parent_proteins_iter ->second->getNTT() ) );
            if ( use_nsp )
            {
               prob = getNSPAdjustedProb( prob, parent_proteins_iter ->second->getNSPBin() );
            }
            probs1.push_back( prob );     //&& values in this array do not appear to be used subsequently (in this translation of the code)
            if ( !parent_proteins_iter ->second->hasMaxProb() || prob > parent_proteins_iter ->second->getMaxProb() )
            {                                                 
               parent_proteins_iter ->second->setMaxProb(prob);
               if ( ! use_nsp )
               {
                  parent_proteins_iter ->second->setOrigMaxProb(prob);                  // do same for ProteinOrigMaxProb hash
               }
            }
            if ( compute_spectrum_cnts && parent_proteins_iter == parent_proteins_list.begin() )       //&& only doing this for first protein in hash - not sure why
            {
               pep1 -> incrementSpectrumCounts( prob );
               total_spectrum_counts += prob;
      	   }  // # if compute total spec counts
            ++parent_proteins_iter;
         }     // # next protein
      }
      ++spec_iter;
   }           // # next singly spectrum
}

//##############################################################################
// function setExpectedNumSiblingPeps
//    input:   none
//    output:  none
//##############################################################################
void setExpectedNumSiblingPeps()
{
  std::vector< double > all_shared_probs;

  for ( peptideMap::const_iterator pep_iter = peptides.begin();pep_iter != peptides.end(); ++pep_iter )                              // process all peptides stored in peptides hash
   { 
      ppPeptide* pep = pep_iter -> second;
      ppParentProteinList& ppw_ref = pep -> getParentProteinListNonConst();
      for ( ppParentProteinList::iterator ppw_iter = ppw_ref.begin(); ppw_iter != ppw_ref.end(); ++ppw_iter )          // process all proteins in the ProteinPepWt hash of the peptide
      {
         const ppProtein *prot = ppw_iter -> first;
         double prob;
	 ppw_iter ->second->setEstNSP(prob=getSharedProtProb( pep, getProteinByName( prot->getName() ))); 
	 all_shared_probs.push_back(prob);
   
      }  // # next protein
      
   }        // # next peptide
  
  //set thresholds dynamically
  std::sort( all_shared_probs.begin(), all_shared_probs.end());     
  for ( int k = 0; k < NUM_SHARED_PROT_PROB_THRESHES; k++ )
    {
      int sh_prob_idx = (k+1)*all_shared_probs.size()/NUM_NSP_BINS - 1;
      if (sh_prob_idx < 0)
          sh_prob_idx = 0;
      shared_prot_prob_threshes[ k ] = all_shared_probs[sh_prob_idx];
      
    }
  
  int num_threshes = NUM_SHARED_PROT_PROB_THRESHES;
  int num_bins = NUM_NSP_BINS;
  
  //collapse bins with same threshold
  for ( int k = 1; k <  num_threshes; k++ )
    {
      if (shared_prot_prob_threshes[ k-1 ] == shared_prot_prob_threshes[k]) {
	memmove(&(shared_prot_prob_threshes[ k-1 ]), &(shared_prot_prob_threshes[k]), sizeof(double)*(num_threshes-k));
	num_threshes--;
	num_bins--;
	k--;
      }
      //if (num_threshes <= 0) {
      //	USE_NSP = false;
      //	return;
      //}
    }

  //use hard coded NSP bins when too few dynamic bins created
  if (num_threshes < statNUM_SHARED_PROT_PROB_THRESHES ) {
    for ( int k = 0; k < statNUM_SHARED_PROT_PROB_THRESHES; k++ )
      {
	shared_prot_prob_threshes[ k ] = static_shared_prot_prob_threshes[ k ];
      }
    NUM_SHARED_PROT_PROB_THRESHES = statNUM_SHARED_PROT_PROB_THRESHES;
    NUM_NSP_BINS = statNUM_NSP_BINS;
    static_nsp = True;
  }
  else {
    NUM_SHARED_PROT_PROB_THRESHES = num_threshes;
    NUM_NSP_BINS = num_bins;
  }

  for ( peptideMap::const_iterator pep_iter = peptides.begin();pep_iter != peptides.end(); ++pep_iter )                              // process all peptides stored in peptides hash
   { 
      ppPeptide* pep = pep_iter -> second;
      ppParentProteinList& ppw_ref = pep -> getParentProteinListNonConst();
      for ( ppParentProteinList::iterator ppw_iter = ppw_ref.begin(); ppw_iter != ppw_ref.end(); ++ppw_iter )          // process all proteins in the ProteinPepWt hash of the peptide
      {
         const ppProtein *prot = ppw_iter -> first;
	 ppw_iter ->second->setNSPBin(getSharedProtIndex( ppw_iter->second->getEstNSP()));
      }     // # next protein
      
   }        // # next peptide
}

void setFPKMPeps()
{
  std::vector< double > all_fpkm;

  for ( peptideMap::const_iterator pep_iter = peptides.begin();pep_iter != peptides.end(); ++pep_iter )                              // process all peptides stored in peptides hash
   { 
      ppPeptide* pep = pep_iter -> second;
      ppParentProteinList& ppw_ref = pep -> getParentProteinListNonConst();
      for ( ppParentProteinList::iterator ppw_iter = ppw_ref.begin(); ppw_iter != ppw_ref.end(); ++ppw_iter )          // process all proteins in the ProteinPepWt hash of the peptide
      {
         const ppProtein *prot = ppw_iter -> first;
         double fpkm;
	 //ppw_iter ->second->setEstFPKM(prob=getFpkmProtProb( pep, getProteinByName( prot->getName() ))); 
	 fpkm = getFPKM( pep, getProteinByName( prot->getName() ));
	 all_fpkm.push_back(fpkm);
   
      }  // # next protein
      
   }        // # next peptide
  
  //set thresholds dynamically
  std::sort( all_fpkm.begin(), all_fpkm.end());    
  int created_bins = 0;
  double last_fpkm = -1;
  int sh_prob_idx = 0;
  for ( int k = 0; k < NUM_FPKM_PROT_PROB_THRESHES; k++ )
    {
      sh_prob_idx += (k+1)*(all_fpkm.size()-sh_prob_idx)/(NUM_FPKM_BINS-created_bins) - 1;
      
      if ( all_fpkm[sh_prob_idx] == last_fpkm) {
	while (all_fpkm[sh_prob_idx] == last_fpkm ) {
	  sh_prob_idx ++;

	}
	sh_prob_idx += (k+1)*(all_fpkm.size()-sh_prob_idx)/(NUM_FPKM_BINS-created_bins) - 1;

      }

      
      
      if (sh_prob_idx < 0)
          sh_prob_idx = 0;

      if (last_fpkm == all_fpkm[sh_prob_idx]) {
	k--;
      }
      else if (last_fpkm > all_fpkm[sh_prob_idx]) {
	for (int kk = created_bins; kk < NUM_FPKM_PROT_PROB_THRESHES; kk++ ) {
	  fpkm_prot_prob_threshes[ kk ] = -777;
	}
	break;
      }
      else {
	created_bins++;
	fpkm_prot_prob_threshes[ k ] = all_fpkm[sh_prob_idx];
      }

      last_fpkm = all_fpkm[sh_prob_idx];
    }
  
 

  NUM_FPKM_PROT_PROB_THRESHES = created_bins-1;
  NUM_FPKM_BINS = created_bins;

  int num_threshes = NUM_FPKM_PROT_PROB_THRESHES;
  int num_bins = NUM_FPKM_BINS;

  if (num_threshes <= 2) {
  	USE_NSP = false;
	return;
  }



  //collapse bins with same threshold
  if (false) {  //DISABLED

    for ( int k = 1; k <  num_threshes; k++ )
      {
	if (fpkm_prot_prob_threshes[ k-1 ] == fpkm_prot_prob_threshes[k]) {
	  memmove(&(fpkm_prot_prob_threshes[ k-1 ]), &(fpkm_prot_prob_threshes[k]), sizeof(double)*(num_threshes-k));
	  num_threshes--;
	  num_bins--;
	  k--;
	}
	//if (num_threshes <= 0) {
	//	USE_FPKM = false;
	//	return;
	//}
      }
    
    //use hard coded FPKM bins when too few dynamic bins created
    if (num_threshes < statNUM_FPKM_PROT_PROB_THRESHES ) {
      for ( int k = 0; k < statNUM_FPKM_PROT_PROB_THRESHES; k++ )
	{
	  fpkm_prot_prob_threshes[ k ] = static_fpkm_prot_prob_threshes[ k ];
	}
      NUM_FPKM_PROT_PROB_THRESHES = statNUM_FPKM_PROT_PROB_THRESHES;
      NUM_FPKM_BINS = statNUM_FPKM_BINS;
      static_fpkm = True;
    }
    else {
      NUM_FPKM_PROT_PROB_THRESHES = num_threshes;
      NUM_FPKM_BINS = num_bins;
    }
  }

  

  for ( peptideMap::const_iterator pep_iter = peptides.begin();pep_iter != peptides.end(); ++pep_iter )                              // process all peptides stored in peptides hash
   { 
      ppPeptide* pep = pep_iter -> second;
      ppParentProteinList& ppw_ref = pep -> getParentProteinListNonConst();
      for ( ppParentProteinList::iterator ppw_iter = ppw_ref.begin(); ppw_iter != ppw_ref.end(); ++ppw_iter )          // process all proteins in the ProteinPepWt hash of the peptide
      {
         const ppProtein *prot = ppw_iter -> first;
	 ppw_iter ->second->setFPKMBin(getFPKMProtIndex( ppw_iter->second->getFPKM()));
      }     // # next protein

   }        // # next peptide
}
//##############################################################################
// function setExpectedNumIntances
//    input:   none
//    output:  none
//##############################################################################
void setExpectedNumInstances()
{
  std::vector< double > all_ni_probs;
  
  for ( peptideMap::const_iterator pep_iter = peptides.begin();pep_iter != peptides.end(); ++pep_iter )                              // process all peptides stored in peptides hash
    { 
      ppPeptide* pep = pep_iter -> second;
      ppParentProteinList& ppw_ref = pep -> getParentProteinListNonConst();
      for ( ppParentProteinList::iterator ppw_iter = ppw_ref.begin(); ppw_iter != ppw_ref.end(); ++ppw_iter )          // process all proteins in the ProteinPepWt hash of the peptide
	{
	  const ppProtein *prot = ppw_iter -> first;
	  double prob;
	  ppw_iter ->second->setExpNI(prob=getExpNumInstances( pep, prot)); 
	  all_ni_probs.push_back(prob);
	  
	}  // # next protein
      
    }        // # next peptide
  
  //set thresholds dynamically
  std::sort( all_ni_probs.begin(), all_ni_probs.end());     
  for ( int k = 0; k < NUM_NI_PROB_THRESHES; k++ )
    {
      int sh_prob_idx = (k+1)*all_ni_probs.size()/NUM_NI_BINS - 1;
      ni_prob_threshes[ k ] = all_ni_probs[sh_prob_idx];
      
    }
  //collapse bins with same threshold
  for ( int k = 1; k < NUM_NI_PROB_THRESHES; k++ )
    {
      if (ni_prob_threshes[ k-1 ] == ni_prob_threshes[k]) {
	memmove(&(ni_prob_threshes[ k-1 ]), &(ni_prob_threshes[k]), sizeof(double)*(NUM_NI_PROB_THRESHES-k));
	NUM_NI_PROB_THRESHES--;
	NUM_NI_BINS--;
	k--;
      }
      if (NUM_NI_PROB_THRESHES <= 0) {
	INSTANCES = false;
	cout << "WARNING: Not able to use Number of Ion Instances model. " << endl;
	return;
      }
    }

  double pos_tot=0;
  double neg_tot=0;
  for ( int k = 0; k < NUM_NI_BINS; k++ )
     {
       pos_ni_distrs[k] = 0;
       neg_ni_distrs[k] = 0;
     }

  for ( peptideMap::const_iterator pep_iter = peptides.begin();pep_iter != peptides.end(); ++pep_iter )                              // process all peptides stored in peptides hash
    { 
      ppPeptide* pep = pep_iter -> second;
      ppParentProteinList& ppw_ref = pep -> getParentProteinListNonConst();
      for ( ppParentProteinList::iterator ppw_iter = ppw_ref.begin(); ppw_iter != ppw_ref.end(); ++ppw_iter )          // process all proteins in the ProteinPepWt hash of the peptide
	{
	  const ppProtein *prot = ppw_iter -> first;
	  ppw_iter ->second->setNIBin(getNumInstancesIndex( ppw_iter->second->getExpNI()));
	}     // # next protein
      
    }        // # next peptide
  
  //calculate NI stats
  for ( peptideMap::const_iterator pep_iter = peptides.begin();pep_iter != peptides.end(); ++pep_iter )                              // process all peptides stored in peptides hash
    { 
      ppPeptide* pep = pep_iter -> second;
      ppParentProteinList& ppw_ref = pep -> getParentProteinListNonConst();
      for ( ppParentProteinList::iterator ppw_iter = ppw_ref.begin(); ppw_iter != ppw_ref.end(); ++ppw_iter )          // process all proteins in the ProteinPepWt hash of the peptide
	{
	  const ppProtein *prot = ppw_iter -> first;
	  int bin;

	  double prob; 
	  //	  if (INSTANCES) {
	  //	    prob = ppw_iter ->second->getInstsMaxProb();
	  //	  }
	  //	  else {
	  prob = ppw_iter ->second->getOrigMaxProb();
	  //	  }

	  if (prob < 0) {
	    cout << "WARNING: Uninitialized peptide in the final list: " << pep->getName() << endl;
	  }
	  else {
	    bin = ppw_iter ->second->getNIBin();
	    pos_ni_distrs[bin]+=prob;
	    pos_tot+=prob;
	    neg_ni_distrs[bin]+=1-prob;
	    neg_tot+=1-prob;
	  }
	  
	}     // # next protein
      
    }        // # next peptide
  for ( int k = 0; k < NUM_NI_BINS; k++ )
     {
       pos_ni_distrs[k] /= pos_tot;
       neg_ni_distrs[k] /= neg_tot;
       
     }

  int maxind = 0;
  double maxrat = pos_ni_distrs[0] /  neg_ni_distrs[0];
  NI_BIN_EQUIVS[0] = 0;
  for ( int k = 1; k < NUM_NI_BINS; k++ )
     {
       if (pos_ni_distrs[k] / neg_ni_distrs[k] < maxrat) {
	 NI_BIN_EQUIVS[k] = maxind;
       }
       else { 
	 NI_BIN_EQUIVS[k] = k;
	 maxrat = pos_ni_distrs[k] / neg_ni_distrs[k];
	 maxind = k;
       }
     }

  for ( peptideMap::const_iterator pep_iter = peptides.begin();pep_iter != peptides.end(); ++pep_iter )                              // process all peptides stored in peptides hash
    { 
      ppPeptide* pep = pep_iter -> second;
      ppParentProteinList& ppw_ref = pep -> getParentProteinListNonConst();
      for ( ppParentProteinList::iterator ppw_iter = ppw_ref.begin(); ppw_iter != ppw_ref.end(); ++ppw_iter )          // process all proteins in the ProteinPepWt hash of the peptide
	{
	  const ppProtein *prot = ppw_iter -> first;
	  int bin;
	  double prob = ppw_iter ->second->getOrigMaxProb();
	  bin = ppw_iter ->second->getNIBin();
	  double negprob =  1 - prob;

	  //DDS: LIMIT max reward at 3X
	  if (pos_ni_distrs[NI_BIN_EQUIVS[bin]] / neg_ni_distrs[NI_BIN_EQUIVS[bin]] > 3.0) {
	    prob *= 3;
	  }
	  else {
	    prob *= pos_ni_distrs[NI_BIN_EQUIVS[bin]];	  
	    negprob *= neg_ni_distrs[NI_BIN_EQUIVS[bin]];
	  }	  prob = prob / (prob + negprob);
	  ppw_iter->second->setInstsMaxProb(prob);
	}     // # next protein
      
    }        // 

}

//##############################################################################
// function getFPKM
//    input:   ppPeptide* - peptide to process
//    input:   ppProtein* - protein to use
//    output:  double - total of probabilities for all sibling peptides of passed peptide that occur in sequence of passed protein (potentially weighted by a couple of factors)
//##############################################################################
double getFPKM( ppPeptide* pep, const ppProtein * prot )
{
   double numer_tot = 0.0;

   const degenList& members = prot -> getDegenList();       // get set whose keys are all peptides that appear in sequence of passed protein
   double maxFPKM = 0;
   for ( unsigned int k = 0; k < members.size(); k++ )
     {
       ppProtein *memberk = members[ k ];
       PARANOID_ASSERT(pep->hasParentProtein(memberk));
       //ppParentProtein &parentProt = pep->getParentProtein(memberk);
       
       if (!k || memberk->getFPKM() > maxFPKM) {
	 maxFPKM = memberk->getFPKM();
       }

     }

   for ( unsigned int k = 0; k < members.size(); k++ )
     {
       ppProtein *memberk = members[ k ];
       //PARANOID_ASSERT(pep->hasParentProtein(memberk));
       ppParentProtein &parentProt = pep->getParentProtein(memberk);
       parentProt.setFPKM(maxFPKM);
       parentProt.setFPKMBin(getFPKMProtIndex(maxFPKM));

     }
   return maxFPKM;
}

//##############################################################################
// function getSharedProtProb
//    input:   ppPeptide* - peptide to process
//    input:   ppProtein* - protein to use
//    output:  double - total of probabilities for all sibling peptides of passed peptide that occur in sequence of passed protein (potentially weighted by a couple of factors)
//##############################################################################
double getSharedProtProb( ppPeptide* pep, const ppProtein * prot )
{
   double numer_tot = 0.0;

   const orderedPeptideList& pip_ref = prot -> getPeptidesInProt();       // get set whose keys are all peptides that appear in sequence of passed protein
   for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )    // iterate through all peptides in set
   {
     ppPeptide *sibling_pep = *pip_iter;
     double pep_wt;
     double max_prob;
     
     if ( sibling_pep != pep )     // # no min prob or wt for denominator       //&& comment from Perl - not sure what it means
       {
	 const ppParentProtein &parentProt = sibling_pep->getParentProtein(prot);
	 //DDS: NSP computed here
	 if (INSTANCES) {
	   max_prob = parentProt.getInstsMaxProb();

	 }
	 else {
	   max_prob = parentProt.getOrigMaxProb();
	 }

	 if ( (pep_wt = parentProt.getPepWt()) >= ORIG_MIN_WT && 
	      max_prob >= ORIG_MIN_PROB )
	   {
	     numer_tot += pep_wt * max_prob;

	   }
       }

     // # this option invokes minimum prob and wt for denom and numerator...    //&& comment from Perl - doesn't particularly make sense
   }  // # next sibling
   
 
   // Normalize by protein length
   if (NORMPROTLEN && prot->getProteinLen() > 0 )
     numer_tot = numer_tot * mean_prot_len / prot->getProteinLen();
   
   return numer_tot;
}

//##############################################################################
// function getExpNumInstances
//    input:   ppPeptide* - peptide to process
//    output:  double - total of probabilities for all sibling peptides of passed peptide that occur in sequence of passed protein (potentially weighted by a couple of factors)
//##############################################################################
double getExpNumInstances( ppPeptide* pep, const ppProtein * prot)
{
   double numer_tot = 0.0;

   const ppParentProtein &parentProt = pep->getParentProtein(prot);
   numer_tot = parentProt.getInstsTotProb() - parentProt.getOrigMaxProb();

   if (numer_tot < 0.0)
     return 0.0;
   
   return numer_tot;
}


//##############################################################################
// function getSharedGroupProtProb
//    input:   ppPeptide* - peptide to process
//    input:   ppProtein* - protein to use
//    output:  double - total of probabilities for all sibling peptides of passed peptide that occur in sequence of passed protein (potentially weighted by a couple of factors)
//##############################################################################
double getSharedGroupProtProb( ppPeptide* pep, const ppProtein * prot, const  std::map< std::string, double > & gr_pep_wts)
{
   double numer_tot = 0.0;

   const orderedPeptideList& pip_ref = prot -> getPeptidesInProt();       // get set whose keys are all peptides that appear in sequence of passed protein
   for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )    // iterate through all peptides in set
   {
      ppPeptide *sibling_pep = *pip_iter;
      if ( sibling_pep != pep )     // # no min prob or wt for denominator       //&& comment from Perl - not sure what it means
      {
         double pep_wt;
         double max_prob;
         const ppParentProtein &parentProt = sibling_pep->getParentProtein(prot);
	 //DDS: NSP computed here
	 const char* pep_str = sibling_pep->getName();
	 if (INSTANCES) {
	   max_prob = parentProt.getInstsMaxProb();
	 }
	 else {
	   max_prob = parentProt.getOrigMaxProb();
	 }
	 if ( (pep_wt = gr_pep_wts.find( pep_str )->second) >=  ORIG_MIN_WT && 
	      max_prob  >=  ORIG_MIN_PROB )
         {
            numer_tot += pep_wt * max_prob;
         }
      }
      // # this option invokes minimum prob and wt for denom and numerator...    //&& comment from Perl - doesn't particularly make sense
   }  // # next sibling

   //Normalize by protein length
   if (NORMPROTLEN && prot->getProteinLen() > 0)
     numer_tot = numer_tot * mean_prot_len / prot->getProteinLen();
   
   return numer_tot;
}

//##############################################################################
// function getNumInstancesIndex
//    input:   double - shared protein probability    //&& need to check whether passed prob is really "shared" in any sense
//    output:  int - index to use in looking up values from pos_ and neg_shared_prot_distrs arrays
//##############################################################################
int getNumInstancesIndex( double niprob )
{
   for ( int k = 0; k < NUM_NI_PROB_THRESHES; k++ )
   {
      if ( niprob <= ni_prob_threshes[ k ] )
      {
         return k;
      }
   }
   return NUM_NI_PROB_THRESHES;
}

//##############################################################################
// function getSharedProtIndex
//    input:   double - shared protein probability    //&& need to check whether passed prob is really "shared" in any sense
//    output:  int - index to use in looking up values from pos_ and neg_shared_prot_distrs arrays
//##############################################################################
int getSharedProtIndex( double sharedprotprob )
{
   for ( int k = 0; k < NUM_SHARED_PROT_PROB_THRESHES; k++ )
   {
     if (!static_nsp) {
      if ( sharedprotprob <= shared_prot_prob_threshes[ k ] )
      {
         return k;
      }
     }
     else {
      if ( sharedprotprob < shared_prot_prob_threshes[ k ] )
      {
         return k;
      }
     }
   }
   return NUM_SHARED_PROT_PROB_THRESHES;
}



//##############################################################################
// function getFpkmProtIndex
//    input:   double - fpkm protein probability    //&& need to check whether passed prob is really "fpkm" in any sense
//    output:  int - index to use in looking up values from pos_ and neg_fpkm_prot_distrs arrays
//##############################################################################
int getFPKMProtIndex( double fpkm )
{
   for ( int k = 0; k < NUM_FPKM_PROT_PROB_THRESHES; k++ )
   {
     if (!static_fpkm) {
      if ( fpkm <= fpkm_prot_prob_threshes[ k ] )
      {
         return k;
      }
     }
     else {
      if ( fpkm < fpkm_prot_prob_threshes[ k ] )
      {
         return k;
      }
     }
   }
   return NUM_FPKM_PROT_PROB_THRESHES;
}


//##############################################################################
// function getNSPAdjustedProb
//    input:   double - probability to be adjusted
//    input:   int - index into NSP bin arrays
//    output:  double - adjusted probability
//##############################################################################
double getNSPAdjustedProb ( double prob, int nsp_index )
{
   const double USE_MIN_PROB = 0.45;
   const double USE_MAX_DIFF = 0.3;
   if ( prob == 0.0 )
   {
      return 0.0;
   }
   int eff_nsp_index = NSP_BIN_EQUIVS[ nsp_index ];            

   double pos = prob * pos_shared_prot_distrs[ eff_nsp_index ];
   if ( pos == 0.0 )
   {
      return 0.0;
   }
   double neg = ( 1 -  prob ) * neg_shared_prot_distrs[ eff_nsp_index ];
   double new_prob = pos / ( pos + neg );
   if ( new_prob > prob )
   {
       if ( USE_MIN_PROB && prob < USE_MIN_PROB )
       {
          return prob;
       }
       if ( USE_MAX_DIFF && new_prob > prob + USE_MAX_DIFF )
       {
          return ( prob + USE_MAX_DIFF );
       }
   }
   return ( pos / ( pos + neg ) );
}


//##############################################################################
// function getNSPAdjustedProb
//    input:   double - probability to be adjusted
//    input:   int - index into NSP bin arrays
//    output:  double - adjusted probability
//##############################################################################
double getFPKMAdjustedProb ( double prob, int fpkm_index )
{
   const double USE_MIN_PROB = 0.45;
   const double USE_MAX_DIFF = 0.3;
   if ( prob == 0.0 )
   {
      return 0.0;
   }
   int eff_fpkm_index = FPKM_BIN_EQUIVS[ fpkm_index ];            

   double pos = prob * pos_fpkm_prot_distrs[ eff_fpkm_index ];
   if ( pos == 0.0 )
   {
      return 0.0;
   }
   double neg = ( 1 -  prob ) * neg_fpkm_prot_distrs[ eff_fpkm_index ];
   double new_prob = pos / ( pos + neg );
   if ( new_prob > prob )
   {
       if ( USE_MIN_PROB && prob < USE_MIN_PROB )
       {
          return prob;
       }
       if ( USE_MAX_DIFF && new_prob > prob + USE_MAX_DIFF )
       {
          return ( prob + USE_MAX_DIFF );
       }
   }
   return ( pos / ( pos + neg ) );
}

bool updateFPKMDistributions()
{
   bool output = false;
   int fpkm_ind = 0;           // FPKM bin for protein being processed
   double nextprob = 0.0;
   double totpos = 0.0;       // total of positive FPKM contributions for all proteins associated with peptide being processed    //&& check definition
   double totneg = 0.0;       // total of negative FPKM contributions for all proteins associated with peptide being processed    //&& check definition
   double* newpos = new double[ NUM_FPKM_BINS ];
   double* newneg = new double[ NUM_FPKM_BINS ];
   const double MIN_DIFF = 0.02;
   double fpkm_prot_prob_pseudocounts = FPKM_PSEUDOS*NUM_FPKM_BINS;   // # how many times the total num to add as pseudocounts to each bin
   memset(newpos,0,sizeof(double)* NUM_FPKM_BINS );
   memset(newneg,0,sizeof(double)* NUM_FPKM_BINS );
   spectrumMap::const_iterator spec_iter = spectra.begin();
   while ( spec_iter != spectra.end() )
   {
      for ( int ch = 0; ch < MAX_PREC_CHARGE - 1; ch++ )
      {
         const ppPeptide *pep = spec_iter -> second -> getSpecPeps( ch );
         if ( pep )
         {
            const ppParentProteinList& parent_proteins_list = pep -> getParentProteinList();
            ppParentProteinList::const_iterator parent_proteins_iter = parent_proteins_list.begin();
            while ( parent_proteins_iter != parent_proteins_list.end() )
            {
               const ppProtein *prot = parent_proteins_iter -> first;
               const ppParentProtein &parent = *parent_proteins_iter->second;
               fpkm_ind = parent.getFPKMBin();
               int prot_NTT = parent.getNTT();
               nextprob = getFPKMAdjustedProb( spec_iter -> second -> getSpecProbs( ch, prot_NTT ), fpkm_ind );
               double pep_wt = parent.getPepWt();
               newpos[ fpkm_ind ] += nextprob * pep_wt;
               totpos += nextprob * pep_wt;
               newneg[ fpkm_ind ] += ( 1 - nextprob ) * pep_wt;
               totneg += ( 1 - nextprob ) * pep_wt;
               ++parent_proteins_iter;
            }
         }
      }
      ++spec_iter;
   }
   // # now the singlys...
 	spec_iter = singly_spectra.begin();
   while ( spec_iter != singly_spectra.end() )
   {
      ppPeptide *pep = spec_iter -> second -> getSinglySpecPeps();
      if ( pep )
      {
         const ppParentProteinList& parent_proteins_list1 = pep -> getParentProteinList();
         ppParentProteinList::const_iterator parent_proteins_iter = parent_proteins_list1.begin();
         while ( parent_proteins_iter != parent_proteins_list1.end() )
         {
            const ppParentProtein &parent = *parent_proteins_iter->second;
            fpkm_ind = parent.getFPKMBin();
            int prot_NTT = parent.getNTT();
            nextprob = getFPKMAdjustedProb( spec_iter -> second -> getSinglySpecProbs( prot_NTT ), fpkm_ind );
            double pep_wt = parent_proteins_iter -> second->getPepWt();
            newpos[ fpkm_ind ] += nextprob * pep_wt;
            totpos += nextprob * pep_wt;
            newneg[ fpkm_ind ] += ( 1 - nextprob ) * pep_wt;
            totneg += ( 1 - nextprob ) * pep_wt;
            ++parent_proteins_iter;
         }
      }
      ++spec_iter;
   }

   if( SMOOTH >= 0.0 ) {
      double tot;
      const double neighbor_wt = SMOOTH;
      double* newerpos =  new double[ NUM_FPKM_BINS ];
      double* newerneg =  new double[ NUM_FPKM_BINS ];
      totpos = 0.0;
      totneg = 0.0;
      for ( int k = 0; k < NUM_FPKM_BINS; k++ )
      {
         tot = 0.0;
         if( k == 0 )
         {
            tot = 1.0 + neighbor_wt;
            newerpos[ k ] = ( newpos[ k ] + neighbor_wt * newpos[ k + 1 ] ) / tot;
            newerneg[ k ] = ( newneg[ k ] + neighbor_wt * newneg[ k + 1 ] ) / tot;
         }
         else if ( k == NUM_FPKM_BINS - 1 )
         {
            tot = 1.0 + neighbor_wt;
            newerpos[ k ] = ( newpos[ k ] + neighbor_wt * newpos[ k - 1 ] ) / tot;
            newerneg[ k ] = ( newneg[ k ] + neighbor_wt * newneg[ k - 1 ] ) / tot;
         }
         else
         {
            tot = 1.0 + 2.0 * neighbor_wt;
            newerpos[ k ] = ( newpos[ k ] + neighbor_wt * ( newpos[ k - 1 ] + newpos[ k + 1 ] ) ) / tot;
            newerneg[ k ] = ( newneg[ k ] + neighbor_wt * ( newneg[ k - 1 ] + newneg[ k + 1 ] ) ) / tot;
         }
      totpos += newerpos[ k ];
      totneg += newerneg[ k ];
      }
      memmove(newpos,newerpos,NUM_FPKM_BINS *sizeof(double));
      memmove(newneg,newerneg,NUM_FPKM_BINS *sizeof(double));
      delete [] newerpos;
      delete [] newerneg;
   }

   bool option = true;                   //&& no idea why this logical switch is hard coded here
   if ( option )
   {
      const double NUM_PSS = 2.0;
      double NUM_PSS2 = totpos > 0.0 ? NUM_PSS * totneg / totpos : NUM_PSS;
      for ( int k = 0; k < NUM_FPKM_BINS; k++ )
      {
         newpos[ k ] += NUM_PSS;
         totpos += NUM_PSS;
         newneg[ k ] += NUM_PSS2;
         totneg += NUM_PSS2;
      }
      for ( int k1 = 0; k1 < NUM_FPKM_BINS; k1++ )
      {
         if ( totpos > 0.0 )
         {
            newpos[ k1 ] /= totpos;
         }
         output = output || fabs( newpos[ k1 ] - pos_fpkm_prot_distrs[ k1 ] ) > MIN_DIFF;
         if ( totneg > 0.0 )
         {
            newneg[ k1 ] /= totneg;
         }
         output = output || fabs( newneg[ k1 ] - neg_fpkm_prot_distrs[ k1 ] ) > MIN_DIFF;
      }
   }
   else                                // if not option
   {
      // # add the pseudocounts here...
      for ( int k = 0; k < NUM_FPKM_BINS; k++ )
      {
         newpos[ k ] += fpkm_prot_prob_pseudocounts * totpos / NUM_FPKM_BINS;
         newneg[ k ] += fpkm_prot_prob_pseudocounts * totneg / NUM_FPKM_BINS;
      }
      for ( int k1 = 0; k1 < NUM_FPKM_BINS; k1++ )
      {
         if ( totpos > 0.0 )
         {
            newpos[ k1 ] /= ( totpos * ( 1.0 + fpkm_prot_prob_pseudocounts ) );
         }
         output = output || fabs( newpos[ k1 ] - pos_fpkm_prot_distrs[ k1 ] ) > MIN_DIFF;
         if( totneg > 0.0 )
         {
            newneg[ k1 ] /= ( totneg * ( 1.0 + fpkm_prot_prob_pseudocounts ) );
         }
         output = output || fabs( newneg[ k1 ] - neg_fpkm_prot_distrs[ k1 ] ) > MIN_DIFF;
      }
   }
   if ( ! output )
   {
      return false;                          // nothing to change
   }
   
   memmove(pos_fpkm_prot_distrs,newpos,NUM_FPKM_BINS *sizeof(double));
   memmove(neg_fpkm_prot_distrs,newneg,NUM_FPKM_BINS *sizeof(double));
   memset(FPKM_BIN_EQUIVS,0,sizeof(FPKM_BIN_EQUIVS));              // # reset these     //&& probably not necessary, since all elements get new values in next block

   //&& my @max_rat = (0, -1);               //&& in Perl, -1 indexes last element of array
   //&& my @min_rat = (9999, -1);
   double max_rat = 0.0;
   double min_rat = 9999.0;
   int max_bin_index = NUM_FPKM_BINS - 1;
   int min_bin_index = NUM_FPKM_BINS - 1;
   double next_rat;
   for ( int k1 = 0; k1 < NUM_FPKM_BINS; k1++ )
   {
      next_rat = neg_fpkm_prot_distrs[ k1 ] > 0.0 ? pos_fpkm_prot_distrs[ k1 ] / neg_fpkm_prot_distrs[ k1 ] : 9999.0;
      if ( next_rat < min_rat )
      {
         min_rat = next_rat;
         min_bin_index = k1;
      }
      if ( next_rat > max_rat )
      {
         max_rat = next_rat;
         max_bin_index = k1;
      }
      FPKM_BIN_EQUIVS[ k1 ] = next_rat < max_rat ? max_bin_index : k1;
   }
   // # now the ones less than min
   for ( int k = 1; k < min_bin_index && k < max_bin_index; k++ )
   {
      FPKM_BIN_EQUIVS[ k ] = min_bin_index;
   }

   //&& my $joined = join(',', @FPKM_BIN_EQUIVS);
   if(!SILENT) {
      std::cout <<  " ---fpkm bin equivs:";
      for (int k=0;k<(int)(sizeof(FPKM_BIN_EQUIVS)/sizeof(FPKM_BIN_EQUIVS[0]));k++) {
         std::cout << " " << FPKM_BIN_EQUIVS[ k ];
      }
      std::cout <<  "\n" ;
   }
   delete [] newpos;
   delete [] newneg;

   return output;
}

//##############################################################################
// function updateNSPDistributions
//    input:   none
//    output:  bool - true if signficant changes made to NSP distributions ( > MIN_DIFF )       //&& should double-check definition
//##############################################################################
bool updateNSPDistributions()
{
   bool output = false;
   int nsp_ind = 0;           // NSP bin for protein being processed
   double nextprob = 0.0;
   double totpos = 0.0;       // total of positive NSP contributions for all proteins associated with peptide being processed    //&& check definition
   double totneg = 0.0;       // total of negative NSP contributions for all proteins associated with peptide being processed    //&& check definition
   double* newpos = new double[ NUM_NSP_BINS ];
   double* newneg = new double[ NUM_NSP_BINS ];
   const double MIN_DIFF = 0.02;
   double shared_prot_prob_pseudocounts = NSP_PSEUDOS*NUM_NSP_BINS;   // # how many times the total num to add as pseudocounts to each bin
   memset(newpos,0,sizeof(double)* NUM_NSP_BINS );
   memset(newneg,0,sizeof(double)* NUM_NSP_BINS );
   spectrumMap::const_iterator spec_iter = spectra.begin();
   while ( spec_iter != spectra.end() )
   {
      for ( int ch = 0; ch < MAX_PREC_CHARGE - 1; ch++ )
      {
         const ppPeptide *pep = spec_iter -> second -> getSpecPeps( ch );
         if ( pep )
         {
            const ppParentProteinList& parent_proteins_list = pep -> getParentProteinList();
            ppParentProteinList::const_iterator parent_proteins_iter = parent_proteins_list.begin();
            while ( parent_proteins_iter != parent_proteins_list.end() )
            {
               const ppProtein *prot = parent_proteins_iter -> first;
               const ppParentProtein &parent = *parent_proteins_iter->second;
               nsp_ind = parent.getNSPBin();
               int prot_NTT = parent.getNTT();
               nextprob = getNSPAdjustedProb( spec_iter -> second -> getSpecProbs( ch, prot_NTT ), nsp_ind );
               double pep_wt = parent.getPepWt();
               newpos[ nsp_ind ] += nextprob * pep_wt;
               totpos += nextprob * pep_wt;
               newneg[ nsp_ind ] += ( 1 - nextprob ) * pep_wt;
               totneg += ( 1 - nextprob ) * pep_wt;
               ++parent_proteins_iter;
            }
         }
      }
      ++spec_iter;
   }
   // # now the singlys...
 	spec_iter = singly_spectra.begin();
   while ( spec_iter != singly_spectra.end() )
   {
      ppPeptide *pep = spec_iter -> second -> getSinglySpecPeps();
      if ( pep )
      {
         const ppParentProteinList& parent_proteins_list1 = pep -> getParentProteinList();
         ppParentProteinList::const_iterator parent_proteins_iter = parent_proteins_list1.begin();
         while ( parent_proteins_iter != parent_proteins_list1.end() )
         {
            const ppParentProtein &parent = *parent_proteins_iter->second;
            nsp_ind = parent.getNSPBin();
            int prot_NTT = parent.getNTT();
            nextprob = getNSPAdjustedProb( spec_iter -> second -> getSinglySpecProbs( prot_NTT ), nsp_ind );
            double pep_wt = parent_proteins_iter -> second->getPepWt();
            newpos[ nsp_ind ] += nextprob * pep_wt;
            totpos += nextprob * pep_wt;
            newneg[ nsp_ind ] += ( 1 - nextprob ) * pep_wt;
            totneg += ( 1 - nextprob ) * pep_wt;
            ++parent_proteins_iter;
         }
      }
      ++spec_iter;
   }

   if( SMOOTH >= 0.0 ) {
      double tot;
      const double neighbor_wt = SMOOTH;
      double* newerpos =  new double[ NUM_NSP_BINS ];
      double* newerneg =  new double[ NUM_NSP_BINS ];
      totpos = 0.0;
      totneg = 0.0;
      for ( int k = 0; k < NUM_NSP_BINS; k++ )
      {
         tot = 0.0;
         if( k == 0 )
         {
            tot = 1.0 + neighbor_wt;
            newerpos[ k ] = ( newpos[ k ] + neighbor_wt * newpos[ k + 1 ] ) / tot;
            newerneg[ k ] = ( newneg[ k ] + neighbor_wt * newneg[ k + 1 ] ) / tot;
         }
         else if ( k == NUM_NSP_BINS - 1 )
         {
            tot = 1.0 + neighbor_wt;
            newerpos[ k ] = ( newpos[ k ] + neighbor_wt * newpos[ k - 1 ] ) / tot;
            newerneg[ k ] = ( newneg[ k ] + neighbor_wt * newneg[ k - 1 ] ) / tot;
         }
         else
         {
            tot = 1.0 + 2.0 * neighbor_wt;
            newerpos[ k ] = ( newpos[ k ] + neighbor_wt * ( newpos[ k - 1 ] + newpos[ k + 1 ] ) ) / tot;
            newerneg[ k ] = ( newneg[ k ] + neighbor_wt * ( newneg[ k - 1 ] + newneg[ k + 1 ] ) ) / tot;
         }
      totpos += newerpos[ k ];
      totneg += newerneg[ k ];
      }
      memmove(newpos,newerpos,NUM_NSP_BINS *sizeof(double));
      memmove(newneg,newerneg,NUM_NSP_BINS *sizeof(double));
      delete [] newerpos;
      delete [] newerneg;
   }

   bool option = true;                   //&& no idea why this logical switch is hard coded here
   if ( option )
   {
      const double NUM_PSS = 2.0;
      double NUM_PSS2 = totpos > 0.0 ? NUM_PSS * totneg / totpos : NUM_PSS;
      for ( int k = 0; k < NUM_NSP_BINS; k++ )
      {
         newpos[ k ] += NUM_PSS;
         totpos += NUM_PSS;
         newneg[ k ] += NUM_PSS2;
         totneg += NUM_PSS2;
      }
      for ( int k1 = 0; k1 < NUM_NSP_BINS; k1++ )
      {
         if ( totpos > 0.0 )
         {
            newpos[ k1 ] /= totpos;
         }
         output = output || fabs( newpos[ k1 ] - pos_shared_prot_distrs[ k1 ] ) > MIN_DIFF;
         if ( totneg > 0.0 )
         {
            newneg[ k1 ] /= totneg;
         }
         output = output || fabs( newneg[ k1 ] - neg_shared_prot_distrs[ k1 ] ) > MIN_DIFF;
      }
   }
   else                                // if not option
   {
      // # add the pseudocounts here...
      for ( int k = 0; k < NUM_NSP_BINS; k++ )
      {
         newpos[ k ] += shared_prot_prob_pseudocounts * totpos / NUM_NSP_BINS;
         newneg[ k ] += shared_prot_prob_pseudocounts * totneg / NUM_NSP_BINS;
      }
      for ( int k1 = 0; k1 < NUM_NSP_BINS; k1++ )
      {
         if ( totpos > 0.0 )
         {
            newpos[ k1 ] /= ( totpos * ( 1.0 + shared_prot_prob_pseudocounts ) );
         }
         output = output || fabs( newpos[ k1 ] - pos_shared_prot_distrs[ k1 ] ) > MIN_DIFF;
         if( totneg > 0.0 )
         {
            newneg[ k1 ] /= ( totneg * ( 1.0 + shared_prot_prob_pseudocounts ) );
         }
         output = output || fabs( newneg[ k1 ] - neg_shared_prot_distrs[ k1 ] ) > MIN_DIFF;
      }
   }
   if ( ! output )
   {
      return false;                          // nothing to change
   }
   
   memmove(pos_shared_prot_distrs,newpos,NUM_NSP_BINS *sizeof(double));
   memmove(neg_shared_prot_distrs,newneg,NUM_NSP_BINS *sizeof(double));
   memset(NSP_BIN_EQUIVS,0,sizeof(NSP_BIN_EQUIVS));              // # reset these     //&& probably not necessary, since all elements get new values in next block

   //&& my @max_rat = (0, -1);               //&& in Perl, -1 indexes last element of array
   //&& my @min_rat = (9999, -1);
   double max_rat = 0.0;
   double min_rat = 9999.0;
   int max_bin_index = NUM_NSP_BINS - 1;
   int min_bin_index = NUM_NSP_BINS - 1;
   double next_rat;
   for ( int k1 = 0; k1 < NUM_NSP_BINS; k1++ )
   {
      next_rat = neg_shared_prot_distrs[ k1 ] > 0.0 ? pos_shared_prot_distrs[ k1 ] / neg_shared_prot_distrs[ k1 ] : 9999.0;
      if ( next_rat < min_rat )
      {
         min_rat = next_rat;
         min_bin_index = k1;
      }
      if ( next_rat > max_rat )
      {
         max_rat = next_rat;
         max_bin_index = k1;
      }
      NSP_BIN_EQUIVS[ k1 ] = next_rat < max_rat ? max_bin_index : k1;
   }
   // # now the ones less than min
   for ( int k = 1; k < min_bin_index && k < max_bin_index; k++ )
   {
      NSP_BIN_EQUIVS[ k ] = min_bin_index;
   }

   //&& my $joined = join(',', @NSP_BIN_EQUIVS);
   if(!SILENT) {
      std::cout <<  " ---nsp bin equivs:";
      for (int k=0;k<(int)(sizeof(NSP_BIN_EQUIVS)/sizeof(NSP_BIN_EQUIVS[0]));k++) {
         std::cout << " " << NSP_BIN_EQUIVS[ k ];
      }
      std::cout <<  "\n" ;
   }
   delete [] newpos;
   delete [] newneg;

   return output;
}

//##############################################################################
// function equivalentPeptide
//    input:   map< char, char >& - hash of amino acid symbol substitutions
//    input:   map< string, StringSet >& - hash of equivalent peptides wherein value is a set containing all actual
//                      peptides which map to that equivalent peptide
//    input:   string - peptide to be converted to equivalent peptide with standardized amino acid symbols
//    output:  string - converted (i.e. equivalent) peptide
// # generalization of equileucine to accommodate multiple aa substitutions (specified by hash) //&& equileucine appears to be a Perl subroutine in some other code
//##############################################################################
const std::string equivalentPeptide(   std::map< char, char >& substitution_aas, 
                                       std::map< std::string, StringSet >& equivalent_peps,
                                       const std::string& pep )
{
   bool abort = true;
   std::string output;

	std::map< char, char >::const_iterator subaa_iter = substitution_aas.begin();
	while ( subaa_iter != substitution_aas.end() )           // check to see if any substitutable aa symbols exist in passed peptide sequence
   {
      if ( pep.find( subaa_iter -> first ) != std::string::npos )
      {
         abort = false;
      }
      ++subaa_iter;
   }
   if ( abort )                                             // return original passed peptide if no substitutable symbols found
   {
      return pep;
   }
   char next;
   for ( unsigned int k = 0; k < pep.size(); k++ )          // replace substitutable symbols with equivalents from substitution_aas hash
   {
      next = pep[ k ];
      if ( ( subaa_iter = substitution_aas.find( next ) ) != substitution_aas.end() )
      {
         output += subaa_iter -> second;
      }
      else
      {
         output += next;
      }
   }
   std::map< std::string, StringSet >::const_iterator eqpep_iter;
   if ( ( eqpep_iter = equivalent_peps.find( output ) ) != equivalent_peps.end() )     // add original passed peptide sequence to set corresponding
   {                                                                                   //    to equivalent peptide (if not already in set)
      equivalent_peps[ output ].insert( pep );
   }
   else                                                                                // equivalent peptide not yet in hash
   {
      StringSet new_pep;
      new_pep.insert( pep );
      equivalent_peps.insert( std::make_pair( output, new_pep ) );                     // add both new equivalent peptide and original passed peptide sequence
   }
   return output;
}


//##############################################################################
// function equivIProphPeptide
//    input:   map< char, char >& - hash of amino acid symbol substitutions
//    input:   map< string, StringSet >& - hash of equivalent peptides wherein value is a set containing all actual
//                      peptides which map to that equivalent peptide
//    input:   map< string, ppPeptideSet >& - hash of equivalent peptides wherein value is a set containing all actual
//                      peptides which map to that equivalent peptide
//    input:   string - peptide to be converted to equivalent peptide with standardized amino acid symbols
//    input:   string - modified peptide to be converted to equivalent peptide with standardized amino acid symbols
//    input:   bool - update equivalent_peps map when true
//    output:  string - converted (i.e. equivalent) peptide
// # generalization of equileucine to accommodate multiple aa substitutions (specified by hash) //&& equileucine appears to be a Perl subroutine in some other code
//##############################################################################
const std::string equivIProphPeptide(   std::map< char, char >& substitution_aas, 
					std::map< std::string, StringSet >& equivalent_peps,
					//					std::map< std::string, ppPeptideSet >& equiv_pep_ptrs,
					const std::string& pep , const std::string& mpep, bool update)
{
   bool abort = true;
   std::string output;
   //   cerr << output << endl;
   std::map< char, char >::const_iterator subaa_iter = substitution_aas.begin();
   while ( subaa_iter != substitution_aas.end() )           // check to see if any substitutable aa symbols exist in passed peptide sequence
   {
      if ( pep.find( subaa_iter -> first ) != std::string::npos )
      {
         abort = false;
      }
      ++subaa_iter;
   }
	//if ( abort )                                             // return original passed peptide if no substitutable symbols found
	  //{
	//      return pep;
	//   }
   char next;
   for ( unsigned int k = 0; k < pep.size(); k++ )          // replace substitutable symbols with equivalents from substitution_aas hash
   {
      next = pep[ k ];
      if ( ( subaa_iter = substitution_aas.find( next ) ) != substitution_aas.end() )
      {
	output += subaa_iter -> second;
      }
      else
      {
	output += next;
	//cerr << output << endl;
      }
   }
   std::map< std::string, StringSet >::const_iterator eqpep_iter;
   if ( update && ( eqpep_iter = equivalent_peps.find( output ) ) != equivalent_peps.end() )     // add original passed peptide sequence to set corresponding
   {                                                                                   //    to equivalent peptide (if not already in set)
      equivalent_peps[ output ].insert( mpep );
   }
   else if (update)                                                                               // equivalent peptide not yet in hash
   {
      StringSet new_pep;
      new_pep.insert( mpep );
      equivalent_peps.insert( std::make_pair( output, new_pep ) );                     // add both new equivalent peptide and original passed peptide sequence
   }
   return output;
}

//##############################################################################
// function computeFinalProbs
//    input:   none//    output:  none
//##############################################################################
void computeFinalProbs()
{
   proteinMap::const_iterator prot_iter = proteins.begin(); 
   while ( prot_iter != proteins.end() )
   {     
     computeProtNSP( prot_iter -> second );
      if ( prot_iter -> second -> isDegen() )
      {
	prot_iter -> second -> setFinalProbability( computeDegenProtProb( prot_iter -> second ) );
      } else if (! prot_iter -> second -> isMember() )
      {
	prot_iter -> second -> setFinalProbability( computeProteinProb( prot_iter -> second ) );
      } else 
      {
         prot_iter -> second -> setFinalProbability( UNINIT_VAL );
      }
      ++prot_iter;
   }
   prot_iter = proteins.begin();
   while ( prot_iter != proteins.end() )
   {
      ppProtein *prot = prot_iter -> second;
      if ( prot -> hasFinalProbability() )                         // proceed only if final probability was set in one of loops above
      {
         prot -> setProbability( roundProtProbability(prot -> getFinalProbability()) );
         //&& $protein_probs{$_} = $final_prot_probs{$_} > 1 ? 1 : sprintf("%0.2f", $final_prot_probs{$_}); # truncate after 2 decimal places   //&& replaced by line immediately above
      } else {
         prot -> setProbability( UNINIT_VAL );
      }
      ++prot_iter;
   }
}

//##############################################################################
// function getSignPeps
//    input:   ppProtein * - 
//    input:   double - minimum weight required for inclusion of peptide
//    input:   double - minimum probability required for inclusion of peptide
//    input:   vector< ppPeptide* >& - reference to ppPeptide* vector.  Passed in by calling routine, cleared by function before use.
//                Modified by this function to add those peptides occuring in sequence of passed protein which have weight and 
//                probability greater than passed thresholds.
//##############################################################################
void getSignPeps( const ppProtein * prot, double min_wt, double min_prob, orderedPeptideList& peps )
{
   peps.clear();
   ppPeptide* pep = NULL;
   const orderedPeptideList& pip_ref = prot -> getPeptidesInProt();     // get set whose keys are all peptides that appear in sequence of passed protein
   for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )                             // iterate through all peptides in set
   {
      pep = *pip_iter;
      ppParentProtein &parentProt = pep->getParentProtein(prot);
     
      if (GROUPWTS) {
	if ( parentProt.getGroupPepWt() >= min_wt && parentProt.getMaxProb() >= min_prob )
	  {
	    peps.insert( pep );
	  }
    

      }
      else {

	if ( parentProt.getPepWt() >= min_wt && parentProt.getMaxProb() >= min_prob )
	  {
	    peps.insert( pep );
	  }
    
      }

   }
}

//##############################################################################
// function isSubOrSuperSet2
//    input:   ppProtein * - first protein 
//    input:   vector of ppPeptide* - first vector of peptides
//    input:   ppProtein * - second protein
//    input:   vector of ppPeptide* - second vector of peptides
//    output:  int -    -1  if first vector is subset of second vector
//                       0  if neither vector is subset of the other
//                       1  if second vector is subset of first vector
//##############################################################################
int isSubOrSuperset2(   const ppProtein * first_prot,
                        const orderedPeptideList& first_array,
                        const ppProtein * second_prot,
                        const orderedPeptideList& second_array,
                        bool use_ntt_info )
{
   //&& use_ntt_info was apparently being passed with value 2, but can't see why, except as clumsy method for controlling debug messages
   if ( first_array.empty() || second_array.empty() ||  (first_array.size() == second_array.size()) )
   {
      return 0;
   }
   //&& std::cout <<  "still here with ", scalar @{$firstptr}, " first and ", scalar @{$secondptr}, " second\n" if($use_ntt_info == 2);
   if ( first_array.size() > second_array.size() )
   {
      for ( orderedPeptideList::const_iterator k = second_array.begin(); k!=second_array.end(); ++k)
      {
       orderedPeptideList::const_iterator j;
       for ( j = first_array.begin(); j!=first_array.end(); ++j)
         {
            if ( *k == *j) {
               if ( ! use_ntt_info ) {
                  break; // matched
               }
               else if ((*k) -> getParentProteinNTT(second_prot) == 
                        (*j) -> getParentProteinNTT(first_prot ))  {
                  break; // matched
               } else {
                  return 0; // same peptide but no NTT match, may as well quit now
               }
            }
         } // # next first
         if ( j == first_array.end() )
         {
            return 0; // not found
         }
      } // # next second
      return 1;
   }
   else
   {
      for ( orderedPeptideList::const_iterator k = first_array.begin(); k!=first_array.end(); ++k)
      {
       orderedPeptideList::const_iterator j;
       for ( j = second_array.begin(); j!=second_array.end(); ++j)
         {
            if ( *k == *j) {
               if ( ! use_ntt_info ) {
                  break; // matched
               }
               else if ((*k) -> getParentProteinNTT(first_prot) == 
                        (*j) -> getParentProteinNTT(second_prot ))  {
                  break; // matched
               } else {
                  return 0; // same peptide but no NTT match, may as well quit now
               }
            }
         } // # next second
         if ( j == second_array.end() )
         {
            return 0; // not found
         }
      }  // # next first
      return -1;
   }
   return 0;
}

//##############################################################################
// function equalList
//    input:   vector of ppPeptide* - first vector of peptides
//    input:   vector of ppPeptide* - second vector of peptides
//    output:  bool - true if vectors are same size and contain elements of same value, false otherwise
//##############################################################################
bool equalList( const orderedPeptideList& first_array, const orderedPeptideList& second_array )
{
   if ( first_array.empty() || ( first_array.size() != second_array.size() ))
   {
      return false;
   }
   orderedPeptideList::const_iterator j = second_array.begin();
   for ( orderedPeptideList::const_iterator i = first_array.begin(); i!=first_array.end(); ++i) {
      if (*i != *j) {
         return false; // found an unmatched element
      }
      j++;
   }
   return true;
}

//##############################################################################
// function equalList2
//    input:   ppProtein * - first protein
//    input:   vector of ppPeptide* - first vector of peptides
//    input:   ppProtein * - second protein
//    input:   vector of ppPeptide* - second vector of peptides
//    output:  bool - true if vectors are same size and each pair of similarly indexed elements have same value, false otherwise
//
// this version requires that each pair of peptides have same NTT with respect to their proteins
//##############################################################################
bool equalList2(  const ppProtein * first_prot,
                  const orderedPeptideList& first_array,
                  const ppProtein * second_prot,
                  const orderedPeptideList& second_array )
{
   if ( first_array.empty() || (first_array.size() != second_array.size() ))
   {
      return false;
   }
   orderedPeptideList::const_iterator j = second_array.begin();
   for ( orderedPeptideList::const_iterator i = first_array.begin(); i!=first_array.end(); ++i) {
      if ((*i != *j) || ((*i) -> getParentProteinNTT( first_prot ) != (*j) -> getParentProteinNTT( second_prot) )) {
         return false; // NTT wrt proteins don't match
      }
      j++;
   }
   return true;
}

//##############################################################################
// function findDegenGroups3
//    input:   double - minimum weight required
//    input:   double - minimum probability required
//    input:   bool - flag for whether to use ntt_info
//    output:  none
//##############################################################################
void findDegenGroups3( double min_wt, double min_prob, bool use_ntt_info )
{
   // %degen = ();      //&& not necessary to clear or initialize; taken care of by Protein class constructor
   // %member = ();     //&& not necessary to clear or initialize; taken care of by Protein class constructor
   // my %included;     //&& not necessary to clear or initialize; taken care of by Protein class constructor
#ifdef _DEBUG
   bool newverbose = false;//true;
#endif
   double orig_min_prob = min_prob;
   std::string cluster;
   unsigned int cluster_maxlen = 0;
   ppProtein *clusterProt;
   ppPeptide* pep = NULL;
   std::vector< ppPeptide* > last_peps;
   orderedPeptideList parent_peps;
   ProteinSet subsumed; // list of proteins whose subsumed member has been set

   // # go through all proteins looking for those with no independent evidence
   std::vector< ppProtein* > prot_ptrs;     // pull out existing single-protein objects beforehand so that iteration over proteins hash
                                          //    is not mucked up by addition of multi-protein Protein objects to hash during iteration
   proteinMap::const_iterator prot_iter = proteins.begin();
   while ( prot_iter != proteins.end() )
   {
      prot_ptrs.push_back( prot_iter -> second );
      ++prot_iter;
   }
   std::sort( prot_ptrs.begin(), prot_ptrs.end(), compareProteinNamesAsc );  // sort proteins in ascending alphabet order

   for ( unsigned int i = 0; i < prot_ptrs.size(); i++ )
   {
      int num = 0;
      ppProtein *proti = prot_ptrs[ i ];
      min_prob = orig_min_prob;
      ProteinSet included; // for noting whether we updated subsume on this pass

      ppProtein *subsuming_prot = NULL;

      if ( ! proti  -> isMember() )
      { // if not already examined
         cluster = (clusterProt=proti)->getName();
	 if (PROTLEN || NORMPROTLEN)
	   cluster_maxlen = (clusterProt=proti)->getProteinLen();
         orderedPeptideList sign_peps;
         getSignPeps( proti, min_wt, min_prob, sign_peps );
         while ( sign_peps.size() == 0 && min_prob > 0.0 )
         {
            min_prob -= 0.1;
            getSignPeps( proti, min_wt, min_prob, sign_peps );
         }
         for ( orderedPeptideList::const_iterator k = sign_peps.begin(); k != sign_peps.end(); ++k )
         {
            pep = *k;
            const ppParentProteinList& parent_prots = pep -> getParentProteinList();
            ppParentProteinList::const_iterator new_prots_iter = parent_prots.begin();
            while ( new_prots_iter != parent_prots.end() )
            {
               ppProtein *new_prot = new_prots_iter -> first;
#ifdef _DEBUG
               if (newverbose) {
		            std::cout << proti->getName() << " with " << new_prot->getName() <<"\n" ;
               }
#endif
               if ( ( proti!=new_prot )  && 
                  ( ! new_prot -> isMember() ) && ( ! new_prot -> isDegen() ) )
               // if(! ($_ eq $new_prots[$j]) && ! exists $member{$new_prots[$j]} && ! exists $degen{$new_prots[$j]})   //&& replaced by line immediately above
               {
                  orderedPeptideList new_peps;
                  getSignPeps( new_prot, min_wt, min_prob, new_peps );          //&& new_peps was declared locally in Perl code
                  if( ( ( ! use_ntt_info ) && equalList( sign_peps, new_peps) ) || 
                     ( use_ntt_info && equalList2( proti, sign_peps, new_prot, new_peps ) ) )
                  {
                     // # add the members......
                     cluster = cluster + " " + new_prot->getName();
		     
		     if (PROTLEN || NORMPROTLEN)
		       cluster_maxlen = new_prot->getProteinLen() > cluster_maxlen ? new_prot->getProteinLen() : cluster_maxlen;
		     
		     num++;
                     new_prot -> setMember( true );
                     //&& print "adding $new_prots[$j] to member list $cluster\n" if($verbose);
#ifdef _DEBUG
                     if(newverbose) {
                     std::cout << "adding " << new_prot->getName() << " to member list " << cluster <<"\n" ;
                     }
#endif
                     /* BSP: this just gets cleared later, why bother
                     last_peps.clear();
                     for ( unsigned int i = 0; i < new_peps.size(); i++ )
                     {
                        last_peps.push_back( new_peps[ i ] );
                     }
                     */
			            if ( MERGE_SUBSETS && (new_prot!=subsuming_prot) )
                     {
                        included.insert(new_prot);
#ifdef _DEBUG
                        if (newverbose) {
                           std::cout << "1: " << new_prot->getName() << " subsumed by " << (subsuming_prot?subsuming_prot->getName():"") << "\n";
                        }
#endif
                     }
                  } // # if equal lists
		            else if ( MERGE_SUBSETS && !subsuming_prot )     // # not equal
                  {
                     int result = isSubOrSuperset2( proti, sign_peps, new_prot, new_peps, use_ntt_info );
                     if ( result == -1 )  // # first is subset of second, set first to 0 prob, give wts to second
                     {
#ifdef _DEBUG
                        if(newverbose) {
                           std::cout<< "2: "<<proti->getName() << " subsumed by " << new_prot->getName() << "\n";
                           for(orderedPeptideList::const_iterator z = sign_peps.begin(); z != sign_peps.end(); z++) {
                              std::cout<< (*z)->getName() << " ";
                           }
                           std::cout << "\n";
                           for(orderedPeptideList::const_iterator zz = new_peps.begin(); zz != new_peps.end(); zz++) {
                              std::cout << (*zz)->getName() << " ";
                           }
                           std::cout << "\n";          
                        }
#endif
                        // # climb up the tree as far as relevant for prot
                        ppProtein *parentProt = new_prot;
                        bool done = false;
                        while ( ! done && ( parentProt -> getSubsumingProtein() ) )
                        {
                           subsuming_prot = parentProt -> getSubsumingProtein();
                           getSignPeps( subsuming_prot, min_wt, min_prob, parent_peps );                          //&& parent_peps was declared locally in Perl code
                           if ( isSubOrSuperset2( proti, sign_peps, subsuming_prot, parent_peps, use_ntt_info ) == -1 )   //&& Perl code had no fifth arguemnt!!
                           {
                              parentProt = subsuming_prot;
                           }
                           else
                           {
                              done = true;
                           }
                        }
                        subsuming_prot = parentProt;
                        included.insert(proti);
                     }
                  }
               }     // end: if ( prot != new_prot && ...          //&& # if merge subsets - original Perl closing comment
               else if ( MERGE_SUBSETS && ( !subsuming_prot ) && new_prot -> isDegen() )
               {
                  orderedPeptideList new_peps;
                  getSignPeps( new_prot, min_wt, min_prob, new_peps );          //&& new_peps was declared locally in Perl code
#ifdef _DEBUG
                  if(newverbose) {
                     std::cout << "got " << new_peps.size() << " peps for " << new_prot->getName() << "\n";
                  }
#endif
                  int result = isSubOrSuperset2( proti, sign_peps, new_prot, new_peps, use_ntt_info );
                  if ( result == -1 ) // # first is subset of second, set first to 0 prob, give wts to second
                  {
#ifdef _DEBUG
                     if(newverbose) {
                        std::cout << "3: " << proti->getName() << " subsumed by " << new_prot->getName() << "\n";
                     }
#endif
                     // # go as high up family tree as warranted
                     bool done = false;
                     ppProtein *parent_prot = new_prot;
                     while ( ! done && ( parent_prot -> getSubsumingProtein() ) )
                     {
                        subsuming_prot = parent_prot -> getSubsumingProtein();
                        getSignPeps( subsuming_prot, min_wt, min_prob, parent_peps );                          //&& parent_peps was declared locally in Perl code
                        if ( isSubOrSuperset2( proti, sign_peps, subsuming_prot, parent_peps, use_ntt_info ) == -1)    //&& Perl code had no fifth arguemnt!!
                        {
                           parent_prot = subsuming_prot;
                        }
                        else
                        {
                           done = true;
                        }
                     }
                     subsuming_prot = parent_prot;
		     included.insert(proti);
                  }
               }
               ++new_prots_iter;
            }     // end: while ( new_prots_iter != ...
         }     // end: for ( int k = 0; k < sign_peps.size() ...
      }     // end: if ( ! getProtein( prot ) -> isMember() ) 

      if ( num > 0 )
      {
         proti ->setMember( true );                                        // # add self
         clusterProt = getProteinByName( cluster ); // creates a new protein entry
	 
	 if (PROTLEN || NORMPROTLEN)
	   clusterProt->setProteinLen( cluster_maxlen );
#ifdef _DEBUG
         if(newverbose) {
            std::cout << "adding " << proti->getName() << " to member list " ;
            std::cout << "\nhave new cluster: " << cluster <<"\n";
         }
#endif
         if ( MERGE_SUBSETS )
         {
            if ( subsuming_prot ) {
               subsumed.insert(clusterProt); // note that we set this
               clusterProt->setSubsumingProtein(subsuming_prot);
            }
            // # might have to go through all previous subsume targets and redirect them to $subsumed
            for (ProteinSet::iterator prot_iter = subsumed.begin();
                        prot_iter != subsumed.end(); ++prot_iter) {
               if ( included.find((*prot_iter)->getSubsumingProtein()) != included.end()  ) {
                  // previously subsumed, and updated on this round
#ifdef _DEBUG
                  if(newverbose) {
                     std::cout << "Redirecting " << (*prot_iter)->getName() <<" from " << 
                        ((*prot_iter)->getSubsumingProtein()?(*prot_iter)->getSubsumingProtein()->getName():"") <<" to " << 
                        (subsuming_prot?subsuming_prot->getName():"") <<"\n";
                  }
#endif
                  (*prot_iter)->setSubsumingProtein((subsuming_prot==NULL) ? clusterProt : subsuming_prot );
               }
            }
         }
	      // # add on low prob peptides that are shared among them all
         // BSP: perl comment says add on, but really it replaces
         getAllFPKMs( clusterProt);
	 getAllSharedPeptides( clusterProt, last_peps );
	
         for ( unsigned int m = 0; m < last_peps.size(); m++ )
         {
            ppPeptide* last_pep = last_peps[ m ];
            clusterProt -> addPeptide( last_pep );      // insert peptide into PeptidesInProt set for this protein (has no effect if peptide already in set)
	    ppParentProtein &parentCluster = last_pep->getParentProtein(clusterProt);
	    ppParentProtein &parentProt = last_pep->getParentProtein(proti);
	    parentCluster.setPepWt(( MERGE_SUBSETS && (subsumed.find(clusterProt)!=subsumed.end())) ? 0.0 : getDegenWt( clusterProt, *last_pep ));     // # set wt to 1  
	    parentCluster.setGroupPepWt(( MERGE_SUBSETS && (subsumed.find(clusterProt)!=subsumed.end())) ? 0.0 : getDegenWt( clusterProt, *last_pep ));     // # set wt to 1  

	    parentCluster.setOrigMaxProb( parentProt.getOrigMaxProb() );

	    if (INSTANCES) {
	      parentCluster.setMaxProb( parentProt.getInstsMaxProb() );
	      parentCluster.setInstsMaxProb( parentProt.getInstsMaxProb() );
	      parentCluster.setExpNI( parentProt.getExpNI() );
	      parentCluster.setNIBin( parentProt.getNIBin() );
	    }
	    else {
	      parentCluster.setMaxProb( parentProt.getOrigMaxProb() );
	    }
	    parentCluster.setNumInsts( parentProt.getNumInsts());
	    parentCluster.setNTT( parentProt.getNTT());
	    parentCluster.setInstsTotProb( parentProt.getInstsTotProb());
	    parentCluster.setOrigPepWt( parentCluster.getPepWt());
	    
	    // # reset all other proteins with shared peptides to that cluster
	   
	    ppParentProteinList& ppw_ref = last_pep -> getParentProteinListNonConst();
	    for ( ppParentProteinList::iterator ppw_iter = ppw_ref.begin(); ppw_iter != ppw_ref.end(); ++ppw_iter )
	      {
		ppw_iter ->second->setPepWt( ppw_iter ->second->getOrigPepWt());
		ppw_iter ->second->setGroupPepWt( ppw_iter ->second->getOrigPepWt());
	      }

         }
	 for ( unsigned int m = 0; m < last_peps.size(); m++ )
         {
	   ppPeptide* last_pep = last_peps[ m ];
	   ppParentProtein &parentCluster = last_pep->getParentProtein(clusterProt);
	   ppParentProtein &parentProt = last_pep->getParentProtein(proti);
	   double prob;
	   parentCluster.setEstNSP(prob=getSharedProtProb( last_pep, clusterProt ));
	   parentCluster.setNSPBin( getSharedProtIndex( prob ));

	   parentCluster.setFPKM( clusterProt->getFPKM() ) ;
	   parentCluster.setFPKMBin( getFPKMProtIndex(clusterProt->getFPKM()));

	   double max_prob;
	   if (INSTANCES) {
	     max_prob = parentProt.getInstsMaxProb();
	   }
	   else {
	     max_prob = parentProt.getOrigMaxProb();
	   }
	   max_prob = getNSPAdjustedProb( max_prob, parentCluster.getNSPBin() );

	   parentCluster.setMaxProb( getFPKMAdjustedProb( max_prob, parentCluster.getFPKMBin() ));

	   
	   // # NSP VALUE HERE....
	   // parentCluster.setEstNSP( parentProt.getEstNSP() * ( num + 1 ) * parentCluster.getPepWt());
	   //parentCluster.setNSPBin( getSharedProtIndex( parentCluster.getEstNSP() ));
	   
	 }
      } // # if num > 0

      if ( MERGE_SUBSETS && subsuming_prot)       // # do the transfers
      {
         for ( ProteinSet::const_iterator inclprot = included.begin(); inclprot != included.end(); ++inclprot )
         {
            orderedPeptideList all_sign_peps;
            getSignPeps( *inclprot, 0.0, 0.0, all_sign_peps );
#ifdef _DEBUG
            if(newverbose) {
               std::cout << "4: " << (*inclprot)->getName() << " subsumed by " << subsuming_prot->getName() << "\n";
            }
#endif
            for ( orderedPeptideList::iterator i = all_sign_peps.begin(); i != all_sign_peps.end(); ++i )
            {
               ppPeptide* all_sign_pep = *i;
               ppParentProtein &parentInclProt = all_sign_pep->getParentProtein(*inclprot);
               ppParentProteinList::iterator iterSubsumed = all_sign_pep -> getParentProteinListNonConst().find( subsuming_prot );
               if ( iterSubsumed != all_sign_pep -> getParentProteinList().end() )
               {
                  iterSubsumed->second->setPepWt(Min(1.0,iterSubsumed->second->getPepWt() + parentInclProt.getPepWt()));
                  iterSubsumed->second->setGroupPepWt(Min(1.0,iterSubsumed->second->getPepWt() + parentInclProt.getPepWt()));
                  iterSubsumed->second->setOrigPepWt(Min(1.0,iterSubsumed->second->getOrigPepWt() + parentInclProt.getOrigPepWt()));
                  //&& ${$pep_wts{$all_sign_peps[$i]}}{$subsumed} += ${$pep_wts{$sign_peps[$i]}}{$included[$in]};    //&& replaced by line immediately above; probable typo corrected (sign_peps -> all_sign_peps)
               }
       			// ${$pep_wts{$all_sign_peps[$i]}}{$included[$in]} = 0.0;
               parentInclProt.setPepWt(0.0);               
	       parentInclProt.setGroupPepWt(0.0);
     			   // ${$orig_pep_wts{$all_sign_peps[$i]}}{$included[$in]} = 0.0;
               parentInclProt.setOrigPepWt(0.0);
            } // # next pep
         } // # next included prot
#ifdef _DEBUG
         if(newverbose) {
            std::cout << "\n";
         }
#endif
         subsumed.insert(proti); // note that this has been set
         proti->setSubsumingProtein(subsuming_prot);
         // # update any references to prot  //&& in Perl code: # update any references to $_
         for ( ProteinSet::iterator prot_iter1 = subsumed.begin(); prot_iter1 != subsumed.end(); ++prot_iter1 )
         {
            if ((*prot_iter1) -> getSubsumingProtein() == proti) {
               (*prot_iter1) -> setSubsumingProtein(subsuming_prot);
            }
         }
      }
   } // next protein
}

//##############################################################################
// function getDegenWt
//    input:   string - protein name (may be concatenated)
//    input:   string - peptide name
//    output:  double - sum of weights for this peptide over all proteins in degenerate group 
//##############################################################################
double getDegenWt( const ppProtein * prot, ppPeptide& pep )
{
   if ( ! OCCAM )
   {
      return 1.0;
   }
   const degenList &prots = prot->getDegenList();
   double tot = 0.0;
   for ( unsigned int i = prots.size(); i--; )
   {
     //tot += pep.getParentProteinOrigPepWt(prots[ i ]);
     tot += pep.getParentProteinPepWt(prots[ i ]);
   }
   return tot;
}

//##############################################################################
// function computeDegenWts
//    input:      none
//    output:     none
//##############################################################################
void computeDegenWts()
{
   for ( peptideMap::const_iterator pep_iter = peptides.begin(); pep_iter != peptides.end(); ++pep_iter )
   {
      size_t have_cluster = 0;
      ppParentProteinList& ppw_ref = pep_iter -> second -> getParentProteinListNonConst();
      for ( ppParentProteinList::const_iterator ppw_iter = ppw_ref.begin(); ppw_iter != ppw_ref.end(); ++ppw_iter )
      {
         const ppProtein *prot = ppw_iter -> first;
         if ( prot -> isDegen() )
         {
            have_cluster += prot->getDegenList().size();
         }
      } // # next prot

      if ( have_cluster != 0 && ppw_ref.size() > have_cluster ) // # recompute wts
      //&& if($have_cluster & @prots - $have_cluster > 0) // # recompute wts  //&& replaced by line immediately above; probable typo corrected (& -> &&)
      {
         for ( ppParentProteinList::iterator ppw_iter1 = ppw_ref.begin(); ppw_iter1 != ppw_ref.end(); ppw_iter1++ )
         {
	   if (!ppw_iter1->first->getSubsumingProtein()) {
	    
	     if (SOFT_OCCAM) {
		 ppw_iter1 -> second->setPepWt( 1.0 / ( ppw_ref.size() - have_cluster ));
		 ppw_iter1 -> second->setGroupPepWt( 1.0 / ( ppw_ref.size() - have_cluster ));
	     }
	   }
	 }
      }
   } // # next pep
}


//##############################################################################
// function computeProtNSP
//    input:   string - protein name (may be concatenated)
//    output:  void
//##############################################################################
void computeProtFPKM( const ppProtein *prot )
{
  const degenList& members = prot -> getDegenList();       // get set whose keys are all peptides that appear in sequence of passed protein
  double maxFPKM = 0;
  for ( unsigned int k = 0; k < members.size(); k++ )
    {
      ppProtein *memberk = members[ k ];
       if (!k || memberk->getFPKM() > maxFPKM) {
	 maxFPKM = memberk->getFPKM();
       }
    }
  
   for ( unsigned int k = 0; k < members.size(); k++ )
     {
       ppProtein *memberk = members[ k ];
       //PARANOID_ASSERT(pep->hasParentProtein(memberk));
       
       memberk->setFPKM(maxFPKM);

     }

}

//##############################################################################
// function computeProtNSP
//    input:   string - protein name (may be concatenated)
//    output:  void
//##############################################################################
void computeProtNSP( const ppProtein *prot )
{
  const orderedPeptideList& pip_ref = prot -> getPeptidesInProt();
  for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )
    {
      ppPeptide* pep = *pip_iter;
      PARANOID_ASSERT(pep->hasParentProtein(prot));
      ppParentProtein &parentProt = pep->getParentProtein(prot);
      double prob;
      parentProt.setEstNSP(prob=getSharedProtProb( pep, prot ));
      parentProt.setNSPBin( getSharedProtIndex( prob ));
      double max_prob;
      if (INSTANCES) {
	max_prob = parentProt.getInstsMaxProb();
	  }
      else {
	max_prob = parentProt.getOrigMaxProb();
      }
      parentProt.setMaxProb( getNSPAdjustedProb(max_prob, parentProt.getNSPBin() ));

    }
}
//##############################################################################
// function computeProtNSP
//    input:   string - protein name (may be concatenated)
//    output:  void
//##############################################################################
void computeGroupProtNSP( const ppProtein *prot, const std::map< std::string, double > & gr_pep_wts )
{
  const orderedPeptideList& pip_ref = prot -> getPeptidesInProt();
  for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )
    {
      ppPeptide* pep = *pip_iter;
      PARANOID_ASSERT(pep->hasParentProtein(prot));
      ppParentProtein &parentProt = pep->getParentProtein(prot);
      double prob;
      if (GROUPWTS) {
	parentProt.setEstNSP(prob=getSharedGroupProtProb( pep, prot, gr_pep_wts ));
      }
      else {
	parentProt.setEstNSP(prob=getSharedProtProb( pep, prot));
      }
      parentProt.setNSPBin( getSharedProtIndex( prob ));
      double max_prob;
      if (INSTANCES) {
	max_prob = parentProt.getInstsMaxProb();
      }
      else {
	max_prob = parentProt.getOrigMaxProb();
      }
      parentProt.setMaxProb( getNSPAdjustedProb( max_prob, parentProt.getNSPBin() ));

    }
}

//##############################################################################
// function computeDegenProtProb
//    input:   string - protein name (may be concatenated)
//    output:  double - probability of degenerate protein
//##############################################################################
double computeDegenProtProb( const ppProtein *prot )
{
   const degenList &members = prot->getDegenList();
   double prob = 1.0;
   double nextprob = 0.0;
   double next_nsp = 0.0;

   const orderedPeptideList& pip_ref = prot -> getPeptidesInProt();
   for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )
   {
      ppPeptide* pep = *pip_iter;

      nextprob = 1.0;
      next_nsp = 0.0;
      PARANOID_ASSERT(pep->hasParentProtein(prot));
      ppParentProtein &parentProt = pep->getParentProtein(prot);
      double tot_wt = 0.0;
      double tot_max = 0.0;
      double orig_tot_max = 0.0;             // # compute the average max prob for peptide with respect to each member protein
      for ( unsigned int k = 0; k < members.size(); k++ )
      {
         ppProtein *memberk = members[ k ];
         PARANOID_ASSERT(pep->hasParentProtein(memberk));
         ppParentProtein &parentProt = pep->getParentProtein(memberk);
         tot_max += parentProt.getMaxProb()/ members.size();
	 double max_prob;
	 if (INSTANCES) {
	   max_prob = parentProt.getInstsMaxProb();
	 }
	 else {
	   max_prob = parentProt.getOrigMaxProb();
	 }
         orig_tot_max += max_prob / members.size();
         double pepwt = parentProt.getPepWt();
         PARANOID_ASSERT(isInit(pepwt));
         tot_wt += pepwt;
         nextprob -= parentProt.getMaxProb() * pepwt;
         next_nsp += parentProt.getNSPBin() * pepwt;          //&& this calculation doesn't make any sense
      }
      if ( !parentProt.hasMaxProb() )
      {
         parentProt.setMaxProb(tot_max);
      }
      if ( !parentProt.hasOrigMaxProb() )
      {
         parentProt.setOrigMaxProb(orig_tot_max);
      }
      if ( !isInit(parentProt.getPepWt() ))
      {
         parentProt.setPepWt(tot_wt);
      }
      if ( !isInit(parentProt.getGroupPepWt() ))
      {
         parentProt.setGroupPepWt(tot_wt);
      }

      if (GROUPWTS) {
	if ( parentProt.getGroupPepWt() >= MIN_WT && parentProt.getMaxProb() >= MIN_PROB )
	  {
	    prob *= ( 1 - ( parentProt.getMaxProb() ) * ( parentProt.getPepWt() ) );
	  }


      }
      else {
	if ( parentProt.getPepWt() >= MIN_WT && parentProt.getMaxProb() >= MIN_PROB )
	  {
	    prob *= ( 1 - ( parentProt.getMaxProb() ) * ( parentProt.getPepWt() ) );
	  }
      }

      if ( nextprob < 0.0 )
      {
         nextprob = 0.0;            // # just in case of roundoff
      }
      if ( !isInit(parentProt.getNSPBin() ))
      {
         if ( OCCAM )
         {
            parentProt.setNSPBin(( int )( ( parentProt.getPepWt() > 0.0 ) ? ( next_nsp / parentProt.getPepWt() ) : 0 ));    //&& this calculation doesn't make a lot of sense, and is potentially dangerous (i.e. if lhs is assigned a value >= 8 )
         }
         else
         {
            parentProt.setNSPBin(( int )( ( tot_wt > 0.0 ) ? ( next_nsp / tot_wt ) : 0 ));          //&& this calculation doesn't make a lot of sense, and is potentially dangerous (i.e. if lhs is assigned a value >= 8 )
         }
      }
   } // # next peptide
   return ( 1 - prob );
}

//##############################################################################
// function strip
//    input:   string - name of peptide
//    output:  string - name of peptide stripped of everything but uppercase alpha characters
//##############################################################################
std::string strip( const std::string& pep )
{
   std::string output;
   for (const char *c=pep.c_str();*c;c++)
   {
      if (isupper(*c))
      {
         output += *c;
      }
   }
   return output;
}

//##############################################################################
// function findCommonPeps
//    input:   vector< ppPeptide* >& - vector of peptides in which to check names for common substrings       //&& check that this is what is actually done
//    input:   map< string, string >& - hash of peptide names and corresponding peptide group designators (as chars)
//                NOTE: function modifies external values of this input via reference
//    output:  none
//##############################################################################
void findCommonPeps( const std::vector< ppPeptide* >& peps, std::map< std::string, std::string >& pep_grp_desigs )
{
   int ch=0;
   for ( unsigned int k = 0; k < peps.size(); k++ )
   {
      std::string pep_k(peps[ k ] -> getName());
      for ( unsigned int j = 0; j < k; j++ )
      {
         std::string pep_j(peps[ j ] -> getName());
         if ( ( pep_k.substr( 2, pep_k.size() - 2 ) ) == ( pep_j.substr( 2, pep_j.size() - 2 ) ) )   // # if equal peptides (other than charge)
         {
            if ( pep_grp_desigs.find( pep_k ) != pep_grp_desigs.end() )
            {
               pep_grp_desigs[ pep_j ] = pep_grp_desigs[ pep_k ];
            }
            else if ( pep_grp_desigs.find( pep_j ) != pep_grp_desigs.end() )
            {
               pep_grp_desigs[ pep_k ] = pep_grp_desigs[ pep_j ];
            }
            else
            { // new group, a,b,c,...z,aa,ab,ac...az,ba,bb,bc...
               int radix = ('z'-'a')+1;
               std::string str;
               int val=ch; 
               int place = 0;
               do {
                  char c[2] = {((val%radix)+'a'-(place>0)),0};
                  str = c+str;
                  place++;
               } while (val/=radix);
               ch++;
               pep_grp_desigs[ pep_j ] = pep_grp_desigs[ pep_k ] = str;
            }
         }
      }  // # next pep j
   }     // # next pep k
}

//##############################################################################
// function getNumInstancesPeptide
//    input:   double - minimum probability allowed for peptide to be counted
//    input:   bool - flag for whether to use nsp
//    output:  none
//
// # new subroutine calculates numInstancesPeptides ahead of time
// # TS Price @ U Penn Oct 2005                         
//##############################################################################
void getNumInstancesPeptide( double min_prob, bool use_nsp )
{

  spectrumMap::const_iterator spec_iter = singly_spectra.begin();
   while ( spec_iter != singly_spectra.end() )
   {
      ppPeptide *pep = spec_iter -> second -> getSinglySpecPeps();
      ppParentProteinList& prot_ntt_ref = pep -> getParentProteinListNonConst();
      ppParentProteinList::iterator prot_ntt_iter = prot_ntt_ref.begin();
      while ( prot_ntt_iter != prot_ntt_ref.end() ) // for each protein containing the peptide
      {
         ppParentProtein &parentProt = * prot_ntt_iter -> second;
	 double prob = spec_iter -> second -> getSinglySpecProbs( parentProt.getNTT() );
	 if (use_nsp) 
	   prob = getNSPAdjustedProb( prob, parentProt.getNSPBin() );

         if (prob >= min_prob)
	   {
	     parentProt.setNumInsts(parentProt.hasNumInsts()?parentProt.getNumInsts()+1:1);
	     parentProt.setInstsTotProb(parentProt.hasInstsTotProb()?parentProt.getInstsTotProb()+prob:prob);
	   }
         ++prot_ntt_iter;
      }
      ++spec_iter;
   }

	spec_iter = spectra.begin();
   while ( spec_iter != spectra.end() )
   {
      for ( int i = 0; i <= MAX_PREC_CHARGE - 2; i++ )
      {
         ppPeptide *pep = spec_iter -> second -> getSpecPeps( i );
         if ( pep )
         {
            ppParentProteinList& prot_ntt_ref = pep -> getParentProteinListNonConst();
            ppParentProteinList::iterator prot_ntt_iter = prot_ntt_ref.begin();
            while ( prot_ntt_iter != prot_ntt_ref.end() ) // for each protein containing the peptide
            {
               ppParentProtein &parentProt = *prot_ntt_iter -> second;
               double prob = spec_iter -> second -> getSpecProbs( i, parentProt.getNTT() );
	       if (use_nsp) 
		 prob = getNSPAdjustedProb( prob, parentProt.getNSPBin() );
	       if (prob >= min_prob)
		 {
		   parentProt.setNumInsts(parentProt.hasNumInsts()?parentProt.getNumInsts()+1:1);
		   parentProt.setInstsTotProb(parentProt.hasInstsTotProb()?parentProt.getInstsTotProb()+prob:prob);
		 }
               ++prot_ntt_iter;
            }
         }
      }
      ++spec_iter;
   }
}

//##############################################################################
// function getNumberDegenProteins
//    input:   ppPeptide* - peptide to process
//    output:  int - number of proteins which have weight assigned from that peptide 
//##############################################################################
int getNumberDegenProteins( ppPeptide* pep )                 //&& argument should be const, but gives compile errors
{
   int tot = 0;
   const ppParentProteinList& ppw_ref = pep -> getParentProteinList();
   for ( ppParentProteinList::const_iterator ppw_iter = ppw_ref.begin(); ppw_iter != ppw_ref.end(); ++ppw_iter )
   {
      if ( ! ppw_iter -> first -> isMember() )
      {
         tot++;
      }
   }
   if ( tot == 0 )
   {
      std::cout << "*** Warning: " << pep -> getName() << " found to have " << tot << " non-group member proteins" << std::endl;
      tot++;
   }
   return tot;
}

//##############################################################################
// function getAllSharedPeptides
//    input:   ppProtein * - name may be concatenated
//    input:   vector< ppPeptide* >& - vector of peptides which are shared by all proteins in input
//                NOTE: function modifies external values of this input via reference
//    output:  none
//##############################################################################
void getAllSharedPeptides( ppProtein * cluster, std::vector< ppPeptide* >& shared_peps )
{
   shared_peps.clear();
   const degenList &prots = cluster->getDegenList();
   const orderedPeptideList& pip_ref = prots[ 0 ] -> getPeptidesInProt();
   for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )
   {
      ppPeptide* pep = *pip_iter;
      bool ok = true;
      for ( unsigned int k = 1; k < prots.size(); k++ )                       //&& index k should probably start at 1
      {
         if (! prots[ k ]->hasPeptide(pep)) {               // peptide is found in both PeptidesInProt sets
            ok = false;
            break;
         }
      }
      if ( ok )
      {
         shared_peps.push_back( pep );
      }
   }
}

void getAllFPKMs( ppProtein * cluster)
{

   const degenList &members = cluster->getDegenList();
   double maxFPKM = 0;
   for ( unsigned int k = 0; k < members.size(); k++ ) {
     ppProtein *memberk = members[ k ];
     PARANOID_ASSERT(pep->hasParentProtein(memberk));
     //ppParentProtein &parentProt = pep->getParentProtein(memberk);
     
     if (!k || memberk->getFPKM() > maxFPKM) {
       maxFPKM = memberk->getFPKM();
     }
    
   }
   cluster->setFPKM(maxFPKM);
   //cluster->setFPKMBin(getFPKMProtIndex(maxFPKM));
}


//##############################################################################
// function maxPeptideLength
//    input:   none
//    output:  int - length of longest peptide name (among those peptides with entries in their hash ProteinMaxProb)
//##############################################################################
int maxPeptideLength()
{
   int length = 0;

  	peptideMap::const_iterator pep_iter = peptides.begin();
   while ( pep_iter != peptides.end() )                                 // process all peptides stored in peptides hash
   {
      if ( pep_iter -> second -> getParentProteinList( ).size() )
      {
         int next_len = strlen( pep_iter -> first);
         if ( next_len > length )
         {
            length = next_len;
         }
      }
      ++pep_iter;
   }
   return length;
}

//##############################################################################
// function hasIndependentEvidence
//    input:   ppProtein * - the protein
//    input:   double - minimum probability required for peptides associated with the protein
//    output:  bool - true if any peptide associated with the protein is associated with only that protein (and surpasses probability threshold), false otherwise
//##############################################################################
bool hasIndependentEvidence( const ppProtein * prot, double min_prob )
{
   const orderedPeptideList& pip_ref = prot -> getPeptidesInProt();
   for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )
   {
      ppPeptide* pep = *pip_iter;
      if ( pep -> getParentProteinMaxProb( prot ) >= min_prob && getNumberDegenProteins( pep ) == 1 )
      {
         return true;
      }
   }
   return false;           // no independent evidence
}

bool hasIndependentEvidence( const ppProtein * prot, double min_prob, double min_indep_ratio)
{
   const orderedPeptideList& pip_ref = prot -> getPeptidesInProt();

   double shared = 0;
   double indep = 0;
   double rat = -1;
   bool minprob_ok = false;
   for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )
     {
       ppPeptide* pep = *pip_iter;
       if (getNumberDegenProteins( pep ) > 1 ) {
	 shared += pep -> getParentProteinMaxProb( prot );
       }
       else {
	 indep += pep -> getParentProteinMaxProb( prot );
	 if ( pep -> getParentProteinMaxProb( prot ) > min_prob) {
	   minprob_ok = true;
	 }
       }

     }
   
   if (shared <= 0 && indep <= 0) {
     return minprob_ok;
   }

   if (shared > 0 && indep > 0 && indep / (shared+indep) >= min_indep_ratio) {
     return minprob_ok;
   }   
   
   return false;

   // no independent evidence
}

//##############################################################################
// function group
//    input:   string - first protein name
//    input:   string - second protein name
//    output:  bool - true if sufficient proportion of peptides associated with the two proteins are common, false otherwise
//##############################################################################
// returns: -1=can't succeed (no peps overlap) 0=didn't succeed 1=success
static int group_helper( const ppProtein *prot_entry1, const ppProtein * prot_entry2 ) {

   const orderedPeptideList& pip_ref1 = prot_entry1 -> getPeptidesInProt();
   if (pip_ref1.empty()) { // it happens sometimes in degenerate proteins (no common peps found)
      return -1; // can't succeed
   }
   const orderedPeptideList& pip_ref2 = prot_entry2 -> getPeptidesInProt();
   if (pip_ref2.empty()) { // it happens sometimes in degenerate proteins (no common peps found)
      return -1; // can't succeed
   }

   // quick range overlap check
   const ppPeptide *lastpep2 = *(pip_ref2.rbegin()); // last item in list2
   if ((*(pip_ref1.rbegin()) < *(pip_ref2.begin())) || // range 1 ends before range 2 starts
       (*(pip_ref1.begin()) > lastpep2)) { // or range 1 starts after range 2 ends
      return -1; // can't succeed
   }

   const double min_prob = 0.; // TODO why aren't these input args? (it's the same way in .pl)
   const double min_pct = 0.4; // TODO why aren't these input args?
   const int suff_num = 5; // TODO why aren't these input args?

   int num_common = 0;
   int num_tot = 0;
   for ( orderedPeptideList::const_iterator pip_iter1 = pip_ref1.begin(); pip_iter1 != pip_ref1.end(); pip_iter1++ )
   {
      ppPeptide *pep1 = *pip_iter1;
      if (lastpep2 < pep1) { // iter1 is past end of iter2 range
         if (!num_common) { // we didn't find any commonality
            return 0; // didn't succeed
         }
      }
      if (  pep1 -> getParentProteinMaxProb(prot_entry1) >= min_prob ) {
         num_tot++;
         if (pip_ref2.find(pep1)!=pip_ref2.end()) { // same peptide in both prots
            if (++num_common == suff_num) { // solid, we can stop search
               return 1; // success
            }
         }
      }
   }
   return ( num_tot && (((double)num_common / (double)num_tot) >= min_pct ) )?1:0;
}

bool group( const ppProtein *prot_entry1, const ppProtein * prot_entry2 )
{
   if ( prot_entry1 == prot_entry2 )
   {
      return false;
   }
   switch (group_helper(prot_entry1,prot_entry2)) {
      case -1: // peptide lists don't overlap at all
         return false;
         break;
      case 0: // not an absolute failure, try looking at it from the other direction
         return (group_helper(prot_entry2,prot_entry1)>0);
         break;
      case 1: // they group
         return true;
         break;
      default:
         assert(false); // never happens
         return false; // so compiler is happy about all control paths returning a value
         break;
   }
}

//##############################################################################
// function findGroups
//    input:   none
//    output:  none
//##############################################################################
void findGroups()
{
   double min_evid_prob = 0.5;
   unsigned int min_num_peps = 0;         //&& #3;
   double max_prot_prob = 1.0;            //&& #0.05;
   int group_index = 0;
   std::vector< ppProtein* > prot_ptrs;
   for ( proteinMap::const_iterator prot_iter = proteins.begin(); prot_iter != proteins.end(); ++prot_iter )
   {
      if (prot_iter->second->hasProbability()) {
         prot_ptrs.push_back( prot_iter -> second );
      }
   }
   std::sort( prot_ptrs.begin(), prot_ptrs.end(), compareProteinProbabilitiesAsc );       // sort proteins in order of ascending probability
   for ( unsigned int p1 = 0; p1 < prot_ptrs.size(); p1++ )
   {
      ppProtein* prot1 = prot_ptrs[ p1 ];
      const degenList &parsed = prot1->getDegenList();
      double prob1 = prot1 -> getProbability();
      if( prob1 > max_prot_prob )           // # done
      {
         break;
      }
      else if ( ( parsed[ 0 ] -> getPeptidesInProt().size() >= min_num_peps ) ) 

	//&& ( ! hasIndependentEvidence( prot1, min_evid_prob, MIN_INDEP_RAT ) ) )
      {
         for( unsigned int p2 = 0; p2 < p1; p2++ )
         {
            ppProtein* prot2 = prot_ptrs[ p2 ];
            const degenList &parsed = prot2 -> getDegenList();
            double prob2 = prot2 -> getProbability();
            if( prob2 > max_prot_prob )           // # done
            {
               break;
            }
            else if ( ( parsed[ 0 ]-> getPeptidesInProt().size() >= min_num_peps ) && 
                      ( group( prot1, prot2) ) && // this is now cheaper than hasIndependentEvidence(), call first
                      ( ! hasIndependentEvidence( prot1, min_evid_prob, MIN_INDEP_RAT ) || ! hasIndependentEvidence( prot2, min_evid_prob, MIN_INDEP_RAT ) ))
            {
               if ( (!prot1 -> hasGroupMembership()) && !(prot2 -> hasGroupMembership()) )
               {
                  prot1 -> setGroupMembership( group_index );
                  prot2 -> setGroupMembership( group_index );
                  std::vector< ppProtein * > next;
                  next.push_back( prot1 );
                  next.push_back( prot2 );
                  protein_groups.push_back( next );
                  group_index++;
               }
               else if ( !(prot1 -> hasGroupMembership()) )
               {
                  int grp_mem2 = prot2 -> getGroupMembership();
                  prot1 -> setGroupMembership( grp_mem2 );
                  protein_groups[ grp_mem2 ].push_back( prot1 );
               }
               else if ( !(prot2 -> hasGroupMembership()) )
               {
                  int grp_mem1 = prot1 -> getGroupMembership();
                  prot2 -> setGroupMembership( grp_mem1 );
                  protein_groups[ grp_mem1 ].push_back( prot2 );
               }
               else                                         // # reassign and merge
               {
                  int grp_mem1 = prot1 -> getGroupMembership();
                  int grp_mem2 = prot2 -> getGroupMembership();
                  if ( grp_mem1 != grp_mem2 )
                  {
                     for ( unsigned int k = 0; k < protein_groups[ grp_mem2 ].size(); k++ )
                     {
                        protein_groups[ grp_mem1 ].push_back( protein_groups[ grp_mem2 ][ k ] );
                        protein_groups[ grp_mem2 ][ k ]->setGroupMembership(grp_mem1);
                     }
                     protein_groups[ grp_mem2 ].clear();            // # reset
                     PARANOID_ASSERT(prot2 -> getGroupMembership() == grp_mem1 );
                  } // # if not same groups
               }
            } // # if prob 0 and groups
         } // # next p2
      } // # if prob 0 for p1
   } // # next p1
   // # now display each group      //&& comment from Perl - doesn't make a whole lot of sense
   grp_indices.clear();
   for ( int k = 0; k < group_index; k++ )
   {
      if ( protein_groups[ k ].size() > 0 )                      // # if group
      {
         group_probs.push_back( computeGroupProb( k ) );
         grp_indices.push_back( k );
      }
      else                                               // no group
      {
         group_probs.push_back( UNINIT_VAL );
      }
   } // # next index

   orderGroups(); // order within each group
   // sort group indices in descending order of corresponding group probabilities
   std::sort( grp_indices.begin(), grp_indices.end(), compareGroupIndexProbabilities );
   //&& @grp_indices = reverse sort { $group_probs{$a} <=> $group_probs{$b} } keys %group_probs;   //&& replaced by line immediately above
}

//##############################################################################
// function computeGroupProb
//    input:   int - group index
//    output:  double - group probability
//##############################################################################
double computeGroupProb( int ind )
{
   double prob = 1.0;
   std::map< std::string, double > gr_pep_wts;
   std::map< std::string, double > gr_pep_probs;
   // First pass, get peptide group weights
   for ( unsigned int k = 0; k < protein_groups[ ind ].size(); k++ )
   {
      ppProtein * next_prot = protein_groups[ ind ][ k ];
      const degenList &parsed = next_prot->getDegenList();
      const orderedPeptideList& pip_ref = parsed[ 0 ] -> getPeptidesInProt();
      for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )
      {
         ppPeptide* pep = *pip_iter;
         const char *pep_str = pep -> getName();
         if ( gr_pep_wts.find( pep_str ) == gr_pep_wts.end() )
         {
            gr_pep_wts[ pep_str ] = 0.0;
         }
         if ( gr_pep_probs.find( pep_str ) == gr_pep_probs.end() )
         {
            gr_pep_probs[ pep_str ] = 0.0;
         }
         if (pep->hasParentProtein(next_prot)) {
            ppParentProtein &parentProt = pep->getParentProtein(next_prot);
            gr_pep_wts[ pep_str ] += parentProt.getPepWt();
	   
         }
      }  // # next peptide
   }  // # next protein

  for ( unsigned int k = 0; k < protein_groups[ ind ].size(); k++ )
   {
      ppProtein * next_prot = protein_groups[ ind ][ k ];
      const degenList &parsed = next_prot->getDegenList();
      const orderedPeptideList& pip_ref = parsed[ 0 ] -> getPeptidesInProt();
      for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )
      {
         ppPeptide* pep = *pip_iter;
         const char *pep_str = pep -> getName();
         if ( gr_pep_wts.find( pep_str ) == gr_pep_wts.end() )
         {
            gr_pep_wts[ pep_str ] = 0.0;
         }
         if ( gr_pep_probs.find( pep_str ) == gr_pep_probs.end() )
         {
            gr_pep_probs[ pep_str ] = 0.0;
         }
         if (pep->hasParentProtein(next_prot)) {
            ppParentProtein &parentProt = pep->getParentProtein(next_prot);
    	    parentProt.setGroupPepWt( gr_pep_wts[ pep_str ]);
         }
      }  // # next peptide
   }  // # next protein


   //DDS: Second Compute NSP using group peptide weights
   for ( unsigned int k = 0; k < protein_groups[ ind ].size(); k++ )
   {
      ppProtein * next_prot = protein_groups[ ind ][ k ];
      if (GROUPWTS) {
	computeGroupProtNSP(next_prot, gr_pep_wts); 
      }
      else {
	computeProtNSP(next_prot);                                                
      }
      computeProtFPKM(next_prot);       
   }

   //DDS: Third pass recompute the peptide probs
   for ( unsigned int k = 0; k < protein_groups[ ind ].size(); k++ )
     {
       ppProtein * next_prot = protein_groups[ ind ][ k ];

       const degenList &parsed = next_prot->getDegenList();
       const orderedPeptideList& pip_ref = parsed[ 0 ] -> getPeptidesInProt();
       for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )
	 {
	   ppPeptide* pep = *pip_iter;
	   const char *pep_str = pep -> getName();	   
	   if (pep->hasParentProtein(next_prot)) {
	     ppParentProtein &parentProt = pep->getParentProtein(next_prot);
	     parentProt.setFPKM(next_prot->getFPKM());
	     parentProt.setFPKMBin(getFPKMProtIndex(next_prot->getFPKM()));
	     if ( parentProt.getMaxProb() > gr_pep_probs[ pep_str ] )
	       {
		 gr_pep_probs[ pep_str ] = parentProt.getMaxProb();
		 parentProt.setGroupPepWt( gr_pep_wts[ pep_str ]);
	       }
	   }
      }  // # next peptide
   }  // # next protein
   
   //DDS: Fourth pass recompute the prot probs
   for ( unsigned int k = 0; k < protein_groups[ ind ].size(); k++ )
     {
       ppProtein * next_prot = protein_groups[ ind ][ k ];
       if (GROUPWTS) {
	 next_prot->setProbability(computeGroupProteinProb(next_prot, gr_pep_wts));
       }
       else {
	 next_prot->setProbability(computeProteinProb(next_prot));
       }
     }

   // # now final prob.....
   if( OCCAM )
   {
      //&& foreach(keys %pep_wts)         //&& Perl iterates over keys of pep_wts - don't understand this
      std::map< std::string, double >::const_iterator gpw_iter = gr_pep_wts.begin();
      while ( gpw_iter != gr_pep_wts.end() )
      {
         double wt = gpw_iter -> second;
         double pepprob = gr_pep_probs[ gpw_iter -> first ];
          
	 if ( wt >= MIN_WT && 
	      pepprob >= MIN_PROB )
         {
            // watch for accumulated rounding error in wt
            prob *= ( 1 - ( ((wt>1.0)?1.0:wt)  * pepprob ) );
         }
         ++gpw_iter;
      }
   }
   else           // # all wts are 1
   {
      //&& foreach(keys %pep_wts)         //&& Perl iterates over keys of pep_wts - don't understand this
      std::map< std::string, double >::const_iterator gpw_iter1 = gr_pep_wts.begin();
      while ( gpw_iter1 != gr_pep_wts.end() )
      {
         double pepprob = gr_pep_probs[ gpw_iter1 -> first ];
         if ( pepprob >= MIN_PROB )
         {
            prob *= ( 1 - pepprob );
         }
         gpw_iter1++;
      }
   }
   return ( 1 - prob );
}

//##############################################################################
// function maxNTT
//    input:   ppProtein * - protein (name may be concatenated)
//    input:   ppPeptide* - peptide to process
//    output:  int - maximum NTT value among proteins associated with this peptide
//##############################################################################
int maxNTT( const ppProtein * prot_entry, ppPeptide* pep )      //&& second argument should be const, but gives compile errors
{
   const degenList &prots = prot_entry->getDegenList();
   //&& die "error with prot entry $prot_entry and pep $pep\n" if(@prots == 0);
   int max = -1;
   for ( unsigned int k = prots.size(); k--; )
   {
      int ntt = pep->getParentProteinNTT(prots[k]);
      if ( ntt > max )
      {
         max = ntt;
      }
      if ( max == 2 )
      {
         return max;
      }
   }
   return max;
}

//##############################################################################
// function weightedPeptideProbs
//    input:   ppProtein * - protein for which to calculated weighted probabilities
//    input:   double - extra weighting factor to apply to unique peptides
//    output:  double - total weight of all peptides which appear in sequence of that protein
//##############################################################################
double weightedPeptideProbs( const ppProtein * prot, double unique_wt )
{
   double tot = 0.0;
   ppPeptide* pep = NULL;
   const orderedPeptideList& pip_ref = prot -> getPeptidesInProt();   // get set whose keys are all peptides that appear in sequence of passed protein
   for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )    // iterate through all peptides in set
   {
      pep = *pip_iter;
      double max_prob = pep -> getParentProteinMaxProb(prot);     //&& should we be checking first to see whether prot entry exists in ProteinMaxProb hash for this peptide?
      if ( pep -> getUnique() )                                      // use unique_wt multiplier if peptide appears in only one protein
      {
         tot += max_prob * unique_wt;                        
      }
      else
      {
         tot += max_prob;
      }
   }
   return tot;
}

//##############################################################################
// function numUniquePeps
//    input:   protein *
//    output:  int - number of peptides associated with that protein which do not correspond to any other protein
//##############################################################################
int numUniquePeps( const ppProtein * prot )
{
   int num = 0;
   const orderedPeptideList& pip_ref = prot -> getPeptidesInProt();      // get set whose keys are all peptides that appear in sequence of passed protein
   for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )                            // iterate through all peptides in set
   {
      if ( ( *pip_iter ) -> getUnique() )                          // increment num if peptide appears in only one protein
      {
         num++;
      }
   }
   return num;
}

//##############################################################################
// function orderGroups
//    input:   none
//    output:  none
//
// sorts each group of proteins by descending order of their protein probabilities
//##############################################################################
void orderGroups()
{
   for ( unsigned int i = 0; i < protein_groups.size(); i++ )
   {
      std::sort( protein_groups[ i ].begin(), protein_groups[ i ].end(), compareGroupMemberProbabilities );
   }
}

//##############################################################################
// function Bool2Alpha
//		input:	bool - truth value of argument to convert
//		output:	string - corresponding string "Y" or "N"
//##############################################################################
const std::string Bool2Alpha( bool yesno )
{
	return yesno ? "Y" : "N";
}


//##############################################################################
// function isSTYMod
//    input:   string - peptide name
//    output:  bool - whether name contains substring indicating modification on S, T, or Y
//##############################################################################
bool isSTYMod( const std::string& pep )
{
   boost::RegEx rX( "[STY][\\*@#]" );
   return rX.Search( pep );
   //&&   return $pep =~ /[S,T,Y][\*,\@,\#]/;
}


//##############################################################################
// function interpretStdModifiedPeptide
//    input:   string - standard name of peptide
//    input:   string& - string reference in which to place stripped peptide sequence
//    input:   double& - double reference in which to place mass of N-terminal modification
//    input:   double& - double reference in which to place mass of C-terminal modification
//    input:   map< int, double > - hash reference in which to place positions and masses of modifications internal to sequence
//                         NOTE: function modifies external values of above four inputs via reference
//    output:  none
//
// # given std name, extract stripped peptide seq, nterm, cterm, and modification info
//##############################################################################
static boost::RegEx rX_pep( "^(\\d_)(\\S+)$" );
static boost::RegEx rX_aa( "^([A-Zcn])\\[([\\d\\.]*)\\]" );         //&& Perl regex: $next =~ /^([A-Z,c,n])\[(.*?)\]/
static boost::RegEx rX_az( "[A-Z]" );
void interpretStdModifiedPeptide( const std::string& std_pep_in, std::string& pep_seq, double& nterm, double& cterm, ModsMap& mods )
{
   // make sure all the variables passed by reference are cleared
   pep_seq.clear();
   nterm = 0.0;
   cterm = 0.0;
   mods.clear();

   std::string next_aa;
   double next_mass = 0.0;

   std::string peptide;
   std::string std_pep(std_pep_in);
   int counter = 1;                    // # expect first amino acid to be 'n'    //&& Perl comment; is this always true?
   
   // # strip off preceding charge
   if ( rX_pep.Search( std_pep ) )
   {
      peptide = rX_pep[ 1 ];
      std_pep = rX_pep[ 2 ];
   }

   int pep_size = std_pep.size();
   for ( int k = 0; k < pep_size; k++ )      // scan sequence for masses inside square brackets
   {
      std::string next(std_pep.substr( k, pep_size - k ));
      if ( rX_aa.Search( next ) )
      {
         next_aa = rX_aa[ 1 ];
         const std::string &next_orig_mass = rX_aa[ 2 ];
         next_mass = atof( next_orig_mass.c_str() );
         // # now go back to the original to get full mass
         if ( MODIFICATION_MASSES.find( next_aa ) != MODIFICATION_MASSES.end() )
         {
            for ( unsigned int j = 0; j < MODIFICATION_MASSES[ next_aa ].size(); j++ )
            {
               if ( withinError( next_mass, MODIFICATION_MASSES[ next_aa ][ j ], MODIFICATION_ERROR ) )
               {
                  next_mass = MODIFICATION_MASSES[ next_aa ][ j ];
                  break;
               }
            }
         }
         if ( next_aa == "n" )
         {
            nterm = next_mass;
         }
         else if ( next_aa == "c" )
         {
            cterm = next_mass;
         }
         else
         {
            mods[ counter ] = next_mass;
         }
         k += next_orig_mass.size() + 2;     // # advance
      }
      else
      {
         next_aa = next.substr( 0, 1 );
      }
      if ( rX_az.Search( next_aa ) )
      {
         peptide += next_aa;                 // # except for "n" and "c"
         counter++;
      }
   } // # next pos
   pep_seq = peptide;
}

//##############################################################################
// function streamlineStdModifiedPeptide
//    input:   string - peptide name in standard modified form
//    output:  string - peptide name in streamlined standard modified form
//##############################################################################
const std::string streamlineStdModifiedPeptide( const std::string& std_pep_in )
{
   if( ! OMIT_CONST_STATICS )
   {
      return std_pep_in;
   }
   std::string std_pep(std_pep_in);
   std::string peptide;
   // # strip off preceding charge
   if ( rX_pep.Search( std_pep ) )
   {
      peptide = rX_pep[ 1 ];
      std_pep = rX_pep[ 2 ];
   }
   int pep_size = std_pep.size();
   boost::RegEx rX_aa( "^([A-Zcn])\\[(.*)\\]" );         //&& Perl regex: $next =~ /^([A-Z,c,n])\[(.*?)\]/

   for ( int k = 0; k < pep_size; k++ )            // scan sequence for masses inside square brackets
   {
      std::string next(std_pep.substr( k, pep_size - k ));
      bool omit = false;
      int next_mass = 0;                             // # default
      if ( rX_aa.Search( next ) )
      {
         std::string next_aa(rX_aa[ 1 ]);
         std::string next_orig_mass(rX_aa[ 2 ]);                          // will have integer and decimal portions
         next_mass = atoi( next_orig_mass.c_str() );        // extract integer portion only as number
         format_int(next_mass, next_orig_mass );   // extract integer portion only as string
         omit = ( constant_static_mods.find( next_aa ) != constant_static_mods.end() &&
            constant_static_mods[ next_aa ].find( next_mass ) != constant_static_mods[ next_aa ].end() &&
            constant_static_mods[ next_aa ][ next_mass ] >= constant_static_tots );
         if ( omit )
         {
            if ( rX_az.Search( next_aa ) )            // # except for "n" and "c"
            {
               peptide += next_aa;
            }
         }
         else
         {
            peptide += next_aa + '[' + next_orig_mass + ']';
         }
         k += next_orig_mass.size() + 2;           // # advance
      }
      else
      {
         std::string next_aa(next.substr( 0, 1 ));
         if ( rX_az.Search( next_aa ) )
         {
            peptide += next_aa;                    // # except for "n" and "c"
         }
      }
   } // # next position in sequence
   return peptide;
}

//##############################################################################
// function makeStdModifiedPeptide
//    input:   string& - name (sequence) of peptide
//    input:   double - mass of N-terminal modification
//    input:   double - mass of C-terminal modification
//    input:   map< int, double >& - hash with locations and masses of non-terminal peptide modifications
//    input:   double - if mass of aa mod is within this range of a previously found value for the mod, the previous value is used instead
//    output:  string& - modified name (sequence) of peptide
// # given (stripped) peptide seq and modification info, returns std modified peptide name (while entering modified masses in REF)
//##############################################################################
void makeStdModifiedPeptide( std::string& peptide, double nterm, double cterm, ModsMap& mods, double error )
{ 
   std::string pep;
   if ( rX_pep.Search( peptide ) )                  // split off D_ if exists at beginning
   {
      peptide = rX_pep[ 1 ]; // preserve the D_
      pep = rX_pep[ 2 ]; // copy of input less the D_
   }
   else                                         // otherwise process passed peptide sequence as is
   {
      pep = peptide; // copy of input
      peptide.clear();
   }
   if ( nterm > 0.0 )
   {
      appendStdModifiedAminoAcid( peptide, "n", nterm, error );
   }
   ModsMap::const_iterator mods_iter;
   std::string next("x");
   for( unsigned int k = 0; k < pep.size(); k++ ) {      // check peptide sequence for mods one residue at a time
      next[0] = pep[k];
      if ( ( mods_iter = mods.find( k + 1 ) ) != mods.end() )
      {
         appendStdModifiedAminoAcid( peptide, next, mods_iter->second, error );
      }
      else
      {
         peptide += next;
      }
   }
   if ( cterm > 0.0 ) {
      appendStdModifiedAminoAcid( peptide, "c", cterm, error );
   }
}

//##############################################################################
// function appendStdModifiedAminoAcid
//    input:   string& - peptide being built
//    input:   string& - symbol of modified amino acid
//    input:   double - mass of modified amino acid (as reported for peptide by pepXML)
//    output:  string - symbol of modified amino acid together with standardized mass, taken from table, in brackets
//##############################################################################
void appendStdModifiedAminoAcid( std::string &pep, const std::string& aa, double mass, double error )
{
   if ( mass == 0.0 )
   {
      pep += aa;
      return;
   }

   std::vector< double > &modmass = MODIFICATION_MASSES[ aa ]; // will create if needed
   int k;
   for( k = (int)modmass.size(); k--; )
   {
      if ( withinError( mass, modmass[ k ], error ) )      //&& Perl version passed stdmass, not mass
      {
         mass = modmass[ k ];
         break;
      }
   }
   if (k < 0) {
      modmass.push_back( mass );       // didn't find a mass for this aa mod within +/- error of passed mass; make new mass entry for this mod
   }
   std::stringstream ss;
   // my $stdmass = sprintf("%0.0f", $mass); # ensure 2 decimal places //&& uh, actually 0 places
   ss << std::setiosflags( std::ios::fixed ) << std::setprecision( 0 ) << mass;

   pep += aa + '[' + ss.str() + ']';
   return;
}

//##############################################################################
// function withinError
//    input:   double - first in pair of values to compare
//    input:   double - second in pair of values to compare
//    input:   double - maximum allowed difference between two values
//    output:  bool - indicates whether difference between values was <= error
//##############################################################################
bool withinError( double first, double second, double error)
{
   if ( first == second ) {
      return true;
   }
   if ( first < second && first >= second - error ) {
      return true;
   }
   if ( second < first && second >= first - error ) {
      return true;
   }
   return false;
}

//##############################################################################
//	function getTPPVersionInfo
//		input:	none
//		return:	string - holds current version of TPP build
//
//&&  THIS IS A TEMPARY HARD-CODED LOOKUP OF TPP VERSION NUMBER - IT NEEDS TO BE CHANGED SO IT
//&&	LOOKS FOR VERSION NUMBER IN EXTERNAL CONFIG FILE 
//
//##############################################################################
const std::string getTPPVersionInfo()
{
	return std::string( szTPPVersionInfo );
}

//##############################################################################
//&& function writeHashContentsToStdOutput
//&&    input:   none
//&&    output:  none
//&& for test purposes only - writes out selected contents of peptide, protein, and spectrum hashes
//##############################################################################
void writeHashContentsToStdOutput()
{
   int counter = 0;
   std::cout << "Writing out names of Spectrum objects in hash spectra:" << std::endl;
	spectrumMap::const_iterator spec_iter = spectra.begin();
   while ( spec_iter != spectra.end() )
   {
      std::cout << "   " << spec_iter -> first << std::endl;
		++spec_iter;
      counter++;
   }
   std::cout << "Number of Spectrum objects written out = " << counter << std::endl << std::endl;
   counter = 0;
   std::cout << "Writing out names of Spectrum objects in hash singly_spectra:" << std::endl;
   spec_iter = singly_spectra.begin();
   while ( spec_iter != singly_spectra.end() )
   {
      std::cout << "   " << spec_iter -> first << std::endl;
		++spec_iter;
      counter++;
   }
   std::cout << "Number of Spectrum objects written out = " << counter << std::endl << std::endl;
   counter = 0;
   std::cout << "Writing out names of Peptide objects in hash peptide:" << std::endl;
	peptideMap::const_iterator pep_iter = peptides.begin();
   while ( pep_iter != peptides.end() )
   {
      std::cout << "   " << pep_iter -> first << std::endl;
		++pep_iter;
      counter++;
   }
   std::cout << "Number of Peptide objects written out = " << counter << std::endl << std::endl;
   counter = 0;
   std::cout << "Writing out names of Protein objects in hash protein:" << std::endl;
	proteinMap::const_iterator prot_iter = proteins.begin();
   while ( prot_iter != proteins.end() )
   {
      ppProtein* prot = prot_iter -> second;
      std::cout << "   " << prot -> getName() << std::endl;
      prot_iter -> second -> outputPeptidesInProt();
      std::cout << "      isMember = " << ( prot -> isMember() ? "true" : "false" ) << std::endl;
      std::cout << "      probability       = " << prot -> getProbability() << std::endl;
      std::cout << "      final probability = " << prot -> getFinalProbability() << std::endl;
		++prot_iter;
      counter++;
   }
   std::cout << "Number of Protein objects written out = " << counter << std::endl << std::endl;
   return;
}

//######################################################################################################
//#                                                                                                    #
//#                                        UTILITY FUNCTIONS                                           #
//#                                                                                                    #
//######################################################################################################

//##############################################################################
// function cleanUpPathSeps
//    input:   string - path name   NOTE: function modifies external value of this input via reference
//    output:  none
//
// makes sure all path separators point upwards and to the right
//##############################################################################
void cleanUpPathSeps( std::string& path_name )
{
   for ( unsigned int i = 0; i < path_name.size(); i++ ) 
   {
      if ( path_name[ i ] == '\\' )
      {
         path_name[ i ] = '/';
      }
   }
   return;
}

//##############################################################################
// function cleanUpProteinDescription
//    modifies the input value
//    input:   string - raw FASTA header line
//    output:  string - reference to input string with modifications to allow
//                      output to a XML attribute
//##############################################################################
std::string& cleanUpProteinDescription( std::string& prot_descr )
{
     // replace newlines with spaces
     std::replace(prot_descr.begin(),prot_descr.end(),'\n',' ');
     // replace pipe chars with spaces
     std::replace(prot_descr.begin(),prot_descr.end(),'|',' ');
     // truncate at the first separator character
     size_t pos = prot_descr.find('\01');
     if (pos != std::string::npos)
         prot_descr.erase(pos);
     if (prot_descr[0]=='>') {
        prot_descr.erase(0,1); // strip leading '>'
     }

     return prot_descr;
}

//##############################################################################
// function fileExists
//    input:   string - path name of file
//    output:  bool - true if file exists at specified path, false otherwise
//##############################################################################
bool fileExists( std::string& path_name )
{
  if (path_name.empty()) return (false);
  
   bool able_to_open = false;
   std::string local_path_name(path_name);
   for (int retry=2;retry--;) {
      pwiz::util::random_access_compressed_ifstream inFile( local_path_name.c_str()); // can read gzipped files
	   able_to_open = inFile.good();
	   inFile.close();
      if (able_to_open) {
         break;
      }
      // not found - cygwin weirdness?
      char *fname = strdup(path_name.c_str());
      unCygwinify(fname); // no effect in Cygwin builds
	  local_path_name = resolve_root(fname); // web vs filesys issue?
	  free(fname);
   }
   if (able_to_open) {
      path_name = local_path_name; // in case we de-cygwinified it
   }
	return able_to_open;
}

//##############################################################################
// function splitString
//    input:   string - concatenated string to be split
//    input:   vector< string >& - vector of strings derived from splitting input
//                NOTE: function modifies external values of this input via reference
//    output:  none
//##############################################################################
void splitString( const std::string &concat_string, std::vector< std::string >& split_strings ) {
   splitString(concat_string.c_str(),split_strings);
}
void splitString( const char* concat_string, std::vector< std::string >& split_strings )
{
   char *parse_string = strdup(concat_string);
   char *cp = parse_string;
   char *space = parse_string;

   while (*space)
   {
      if (isspace(*space))                 // whitespace found in parse_string
      {
         *space = 0;
         if (*cp) {
            split_strings.push_back( cp );
         }
         cp = ++space; // advance to start of next substring
      } else {
         space++;
      }
      }
   if (cp != space)                                          // no whitespace in parse_string
      {
      if (*cp) {
         split_strings.push_back( cp );
      }
   }
   free(parse_string);
}

//##############################################################################
// function splitStringSorted - like splitString but alpha sorts the result
//    input:   string - concatenated string to be split
//    input:   vector< string >& - vector of strings derived from splitting input
//                NOTE: function modifies external values of this input via reference
//    output:  none
//##############################################################################
void splitStringSorted( const char * concat_string, std::vector< std::string >& split_strings )
{
   splitString(concat_string, split_strings);
   std::sort(split_strings.begin(),split_strings.end());
}

//##############################################################################
// function compareProteinProbabilitiesDesc
//		input:	ppProtein* - pointer to first protein
//    input:   ppProtein* - pointer to second protein
//		output:	true if probability of first protein is greater than that of second protein, false otherwise
//
// supports sorting of an STL vector of proteins into descending order of probability,
//    with secondary sort by protein name in ascending order
//##############################################################################
bool compareProteinProbabilitiesDesc( ppProtein* first_prot, ppProtein* second_prot )    //&& arguments should be const, but gives compile errors
{
   int prob1 = rankProtProbability(first_prot -> getProbability());      
   int prob2 = rankProtProbability(second_prot -> getProbability());     
   if ( prob1 > prob2 )
   {
      return true;
   }
   else if ( prob1 == prob2 && (strcmp(first_prot -> getName(),second_prot -> getName())<0) )
   {
      return true;
   }
   else
   {
      return false;
   }
}

//##############################################################################
// function compareProteinProbabilitiesAsc
//		input:	ppProtein* - pointer to first protein
//    input:   ppProtein* - pointer to second protein
//		output:	true if probability of first protein is less than that of second protein, false otherwise
//
// supports sorting of an STL vector of proteins into ascending order of probability,
//    with secondary sort by protein name in ascending order
//##############################################################################
bool compareProteinProbabilitiesAsc( ppProtein* first_prot, ppProtein* second_prot )    //&& arguments should be const, but gives compile errors
{
   int prob1 = rankProtProbability(first_prot -> getProbability());      
   int prob2 = rankProtProbability(second_prot -> getProbability());     
   if ( prob1 < prob2 )
   {
      return true;
   }
   else if ( prob1 == prob2 && (strcmp(first_prot -> getName(),second_prot -> getName())<0) )
   {
      return true;
   }
   else
   {
      return false;
   }
}

//##############################################################################
// function compareCoveredProteinDBIndices
//		input:	CoveredProt - first covered protein
//    input:   CoveredProt - second covered protein
//		output:	true if IPI database index of first protein is less than that of second protein, false otherwise
//
// supports sorting of an STL vector of covered proteins (as CoveredProt structs) in order of ascending IPI database index
//##############################################################################
bool compareCoveredProteinDBIndices( const CoveredProt& first_covered_prot, const CoveredProt& second_covered_prot )    //&& arguments should be const, but gives compile errors
{
   if ( first_covered_prot.db_index < second_covered_prot.db_index )
   {
      return true;
   }
   else
   {
      return false;
   }
}

//##############################################################################
// function compareGroupIndexProbabilities
//		input:	int - first group index
//    input:   int - second group index
//		output:	true if group probability referenced by first index is greater than that referenced by second index, false otherwise
//
// supports sorting of an STL vector of group indices in order of descending group probabilities referenced by the indices
//##############################################################################
bool compareGroupIndexProbabilities( int first_group_index, int second_group_index )
{
   PARANOID_ASSERT(isInit(group_probs[ first_group_index ]));
   PARANOID_ASSERT(isInit(group_probs[ second_group_index ]));
   int prob1 = rankProtProbability( group_probs[ first_group_index ]);
   int prob2 = rankProtProbability( group_probs[ second_group_index ]);
   if ( prob1 > prob2 )
   {
      return true;
   }
   else if ( (prob1 == prob2) && 
      (strcmp(protein_groups[ first_group_index ][0]->getName(),protein_groups[ second_group_index ][0]->getName())<0))
   {
      return true;
   } else 
   {
      return false;
   }
}

//##############################################################################
// function compareGroupMemberProbabilities
//		input:	ppProtein * - first group member
//    input:   ppProtein * - second group member
//		output:	true if protein probability of first member is greater than that of second member, false otherwise
//
// supports sorting of an STL vector of group members in descending order of their protein probabilities,
//    with secondary sort by protein name in ascending order
//##############################################################################
bool compareGroupMemberProbabilities( const ppProtein * first_protein, const ppProtein * second_protein )
{
   int prob1 = rankProtProbability(first_protein -> getProbability());
   int prob2 = rankProtProbability(second_protein -> getProbability());
   int uniq1 = getTotalUniqPeps(first_protein);
   int uniq2 = getTotalUniqPeps(second_protein);
   int num1 = getTotalNumPeps(first_protein);
   int num2 = getTotalNumPeps(second_protein);

   if ( prob1 > prob2 )
   {
     return true;
   }
   else if ( (prob1 == prob2) && (uniq1 > uniq2) ) {
     return true;
   }
   else if ( (prob1 == prob2) && (uniq1 == uniq2)  && (num1 > num2)) {
     return true;
   }
   else if ( (prob1 == prob2) && (uniq1 == uniq2)  && (num1 == num2) && (strcmp(first_protein->getName(),second_protein->getName())<0))
   {
     return true;
   }
   else
   {
     return false;
   }
}

//##############################################################################
// function comparePeptideNamesAsc
//		input:	ppPeptide* - pointer to first peptide
//    input:   ppPeptide* - pointer to second peptide
//		output:	true if name of first peptide precedes that of second peptide in lexicographic order, false otherwise
//
// supports sorting of an STL vector of peptides by peptide name in ascending order
//##############################################################################
bool comparePeptideNamesAsc( const ppPeptide* first_pep, const ppPeptide* second_pep )        
{
   return (strcmp(first_pep -> getName(),second_pep -> getName() ) < 0);
}

//
// grab annotations from DB for each protein we're interested in
//
void getBatchAnnotation( const std::string& in_db_name ) {
  
  if (in_db_name.empty()) {
    std::cout << "WARNING: No database specified. No annotation information is available.\n";
    return;
  }
  // understand /dbase/blah to mean $WEBSERVER_ROOT/dbase/blah if /dbase/blah does not exist
  std::string db_name(resolve_root(in_db_name.c_str()));
  pwiz::util::random_access_compressed_ifstream DB( db_name.c_str() ); // can read gzipped files
  if (!DB.is_open()) {
     std::cout << "WARNING: Cannot open DB \"" << db_name << "\". No annotation information is available.\n";
     //     myexit(1);
     return;
  }
  char *stringbuf = new char[LINE_WIDTH];
  boost::RegEx rX( "^>(\\S+)\\s+(\\S.*\\S)" );

  while( DB.getline( stringbuf, LINE_WIDTH ) ) {
    if(rX.Search(stringbuf)) {
       ppProtein *new_annot = proteinExists(rX[1]);
       if (new_annot) {
          char *annotation = strdup(rX[2].c_str());
	       // get rid of illegal xml symbols
          for (char *c=annotation;c && *c;c++) {
             switch(*c) {
             case '\"':
                *c = '\'';
                break;
             case '&':
                *c = '+';
                break;
             case '>':
                *c = ')';
                break;
             case '<':
                *c = '(';
                break;
             default:
                break;
             }
          }
          new_annot->setAnnotation(annotation);
	  string* annot = new string(annotation);
	  size_t start = annot->find("FPKM=");
	  size_t end=-1;
	  if (start != string::npos) {
	    start+=5;
	    end = annot->find_first_of(" \t\f\v\n\r", start);
	    double FPKM = atof(annot->substr(start, end-start).c_str());
	    new_annot->setFPKM(FPKM);
	  }
	  else {
	    new_annot->setFPKM(0);
	  }
	  delete annot;
          free(annotation);
       }
    } // end if a new protein entry in DB
  }
  DB.close();
  delete[] stringbuf;
}


//##############################################################################
//
// Excel output
//
//##############################################################################

static void writeProteinExcel(double group_prob, const ppProtein *entry_prot, 
                              int index, int group_index, double min_pep_prob, int prot_id,
                              std::ofstream &EXCEL) {

   if(entry_prot->getProbability() < EXCEL_MINPROB) {
      return;
   }
   
bool PRINT_NUM_PEP_INSTANCES = 1;
double min_pep_instance_prob = 0.2; // only record peptide instance if (NSP adjusted) prob at least 0.2

   std::ostringstream HEADER;
   const orderedPeptideList& pip_ref = entry_prot -> getPeptidesInProt();
   if ( pip_ref.empty() )
   {
      return;
   }

   // do selection sort to order peptides according their max probs for this protein (highest prob first)
   std::vector< PepSorter > sortpeps;
   for ( orderedPeptideList::const_iterator pip_iter = pip_ref.begin(); pip_iter != pip_ref.end(); ++pip_iter )
   {
      sortpeps.push_back( PepSorter(*pip_iter,(*pip_iter)->getParentProtein(entry_prot).getMaxProb() ));
   }
   std::sort(sortpeps.begin(),sortpeps.end(),ltpepsorter);
   std::vector<ppPeptide *> peps;
   for (unsigned int p=0;p<sortpeps.size();p++) {
      peps.push_back(sortpeps[p].pep);
}

   int totuniquepeps = 0;
   for (int pp = peps.size();pp--;) {
      if(peps[pp]->getParentProteinMaxProb(entry_prot) >= min_pep_prob) {
        totuniquepeps++;
      }
   }

   //my @entries = split(' ', $entry);
   //$HEADER .= "$index\t$group_prob\t" . join(',', @entries) . "\t";
   HEADER << index << "\t" << std::setprecision(2) << group_prob << "\t";
   for (int dg=0;dg<(int)entry_prot->getDegenList().size();dg++) {
      if (dg) {
         HEADER << ",";
      }
      HEADER << entry_prot->getDegenList()[dg]->getName();
   }
   HEADER << "\t";
   // $HEADER .= sprintf("%0.2f\t", $protein_probs{$entry});
   HEADER << std::setprecision(4) << entry_prot->getProbability() << "\t";

   double cov = getCoverageForEntry(entry_prot, PRINT_PROT_COVERAGE); 
   if(isInit(cov)) {
      HEADER << std::setprecision(2) << cov << "\t";
}
else {
      HEADER << "\t";
}

   /* TODO not ported, appears to rely on executable that does not exist
if($ASAP) {
    my $result = getASAPRatio($ASAP_IND, $entry, \@peps);
    print "result for $entry: $result\n" if($verbose);

    my $pro_index = '';
    if($result =~ /proIndx\=(\d+)/) {
	$pro_index = $1;
    }

    # now strip off html tags
    while($result =~ /(.*)\<.*?\>(.*)/) {
	$result = $1 . $2;
    }
    if($result =~ /ASAP\:\s+(\S+)\s+\+\-\s+(\S+)\s+\(\S+\)\s+(\d+)\s?$/) {
	$HEADER .= sprintf("%0.2f\t%0.2f\t%d\t", $1, $2, $3);
    }
    else {
	$HEADER .= "\t\t\t"; # no info
    }
}
   */
   HEADER << totuniquepeps << "\t" << getTotalNumPeps(entry_prot,peps) << "\t";

   // # annotation
   std::vector<std::string> prot_descr;
   std::vector<std::string> ipi;
   std::vector<std::string> refseq;
   std::vector<std::string> swissprot;
   std::vector<std::string> ensembl;
   std::vector<std::string> trembl;

   if ( ANNOTATION )
   {
      // getAnnotation($entry, $database, -1);           //&& ignore for now since input comes from XML ( XML_INPUT == true )
      const degenList &entries = entry_prot->getDegenList();
      for (int k=0;k<(int)entries.size();k++) {
         if(IPI_DATABASE) {
            std::string next_annot = entries[ k ] -> getAnnotation();
            if ( next_annot.empty() )
            {
               next_annot = entries[ k ]->getName();
            }
            std::string k_prot_descr;

            std::string k_ipi;
            std::string k_refseq;
            std::string k_swissprot;
            std::string k_ensembl;
            std::string k_trembl;
            
            parseIPI( next_annot, k_prot_descr, k_ipi, k_refseq, k_swissprot, k_ensembl, k_trembl);
            prot_descr.push_back(k_prot_descr);
            ipi.push_back(k_ipi);
            refseq.push_back(k_refseq);
            swissprot.push_back(k_swissprot);
            ensembl.push_back(k_ensembl);
            trembl.push_back(k_trembl);
         } else if(entries[k]->getAnnotation().length()) {
            std::string description(entries[k]->getAnnotation());
            // replace newlines with spaces
            std::replace(description.begin(),description.end(),'\n',' '); 
            // truncate at the first separator charachter
            size_t pos = description.find('\01');
            if (pos != std::string::npos)
                description.erase(pos);
            // # clean up
            std::string test(">");
            test+=entries[k]->getName();
            if(!strncmp(description.c_str(),test.c_str(),test.length())) {
               description = description.substr(test.length()+1);
	    }
            prot_descr.push_back(description);
	}
      } // # next indisting
}

   //  now write them
   if(IPI_DATABASE) {
      HEADER << join(',', ipi) << "\t";
   }
   HEADER << join(',', prot_descr) << "\t"; 
   if(IPI_DATABASE) {
      HEADER << join(',', ensembl) << "\t" << join(',', trembl) << "\t" << 
         join(',', swissprot) << "\t" << join(',', refseq) << "\t" ;
   }

   std::map< std::string, std::string > pep_grp_desigs;            //&& should this be made a data member of Peptide?
   findCommonPeps( peps, pep_grp_desigs );

for(int pep = 0; pep < (int)peps.size(); pep++) {
    EXCEL << HEADER.str();
    // $pep = scalar @peps if(! $EXCEL_PEPTIDES && $pep == 0); # done
    if(! EXCEL_PEPTIDES && !pep) {
      break;
    } // done
    const ppParentProtein &parentProt = peps[pep]->getParentProtein(entry_prot);
    double maxprob =parentProt.getMaxProb();
    if(EXCEL_PEPTIDES) {
	    // next if(${$pep_max_probs{$peps[$pep]}}{$entry} < $min_pep_prob);  # skip if not high enough prob
       if(maxprob < min_pep_prob) { // skip if not high enough prob
          continue;
       }

       double pepwt = parentProt.getPepWt();
       bool color =  ((maxprob >= FINAL_PROB_MIN_PROB) && (pepwt >= FINAL_PROB_MIN_WT));

	// charge
	EXCEL << std::setprecision(2) << pepwt << "\t"; // wt
	EXCEL << *peps[pep]->getName() << "\t"; // charge
	// # must deal with the equivalent peptide names....

	std::vector< std::string > actualpeps;
	std::string pep_str(peps[pep] -> getName());

      if ( equivalent_peps.find( pep_str ) != equivalent_peps.end() ) {
         StringSet& act_peps_ref = equivalent_peps[ pep_str ];       // convert STL set of strings to vector of strings for local use
         for ( StringSet::const_iterator act_peps_iter = act_peps_ref.begin(); act_peps_iter != act_peps_ref.end(); ++act_peps_iter )
         {
            actualpeps.push_back( *act_peps_iter );
         }
	}
      if ( ! actualpeps.empty() ) {
         EXCEL <<  actualpeps[ 0 ].substr( 2, actualpeps[ 0 ].size() - 2 );
      } else {
         EXCEL <<  pep_str.substr( 2, pep_str.size() - 2 );
      }
      EXCEL << "\t";
      EXCEL << std::setprecision(4);
      EXCEL << maxprob << "\t"; // ${$pep_max_probs{$peps[$pep]}}{$entry}); #nsp adj

      EXCEL << parentProt.getOrigMaxProb() << "\t"; // ${$orig_pep_max_probs{$peps[$pep]}}{$entry});
      EXCEL << maxNTT(entry_prot, peps[pep]) << "\t";
      EXCEL << parentProt.getEstNSP() << "\t";
      EXCEL << parentProt.getNumInsts() << "\t";

      if ( pep_grp_desigs.find( pep_str ) != pep_grp_desigs.end() ) {
      	EXCEL << pep_grp_desigs[ pep_str ]; // # pep group desig
	}
	EXCEL << "\t";
	   EXCEL << Bool2Alpha(getNumberDegenProteins(peps[pep]) == 1) << "\t";
      EXCEL << Bool2Alpha(color);
	EXCEL << "\n";
    } // # next pep
   } //# if peps
   if(! EXCEL_PEPTIDES) {
    EXCEL << "\n";
}
}
 

static void writeGroupExcel(int group_ind, double min_pep_prob, int index, std::ofstream &EXCEL){
   if (group_probs[group_ind] < EXCEL_MINPROB) {
      return;
   }
   
   int id = 'a';
   for(int g = 0; g < (int)protein_groups[group_ind].size(); g++) {
      writeProteinExcel(group_probs[group_ind], protein_groups[group_ind][g], index, g+1, min_pep_prob, id++, EXCEL);
   }
}

void writeExcelOutput(double min_prot_prob, double min_pep_prob, const std::string &excelfile) {
   std::ofstream EXCEL( excelfile.c_str() );

   if (!EXCEL.is_open()) {
      std::cout << "cannot open " << excelfile << " for writing\n";
      myexit(1);
   }
   // write header info
   EXCEL << "entry no.\tgroup probability\tprotein\tprotein probability\tpercent coverage\t";
   if(ASAP) {
       EXCEL << "ratio mean\tratio stdev\tratio num peptides\t";
   }
   EXCEL << "num unique peps\ttot num peps\t";
   if(IPI_DATABASE) {
      EXCEL << "ipi\t";
   }
   EXCEL << "description\t";
   if(IPI_DATABASE) {
      EXCEL << "ensembl\ttrembl\tswissprot\trefseq\t";
   }
   if(EXCEL_PEPTIDES) {
      EXCEL << "weight\tprecursor ion charge\tpeptide sequence\tnsp adjusted probability\tinitial probability\tnumber tolerable termini\testimated num sibling peptides\tnum instances\tpeptide group designator\tunique\tis contributing evidence";
   }
   EXCEL << "\n";


   int index = 1;
   int PRINT_NUM_PEP_INSTANCES = 1;
   double min_pep_instance_prob = 0.2; //# only record peptide instance if (NSP adjusted) prob at least 0.2

   if(ASAP) {
      ASAP_IND = 0; // # reset counter
    }

   int MAX_PEPLENGTH = 45;
   int max_peplength = maxPeptideLength();
   if(max_peplength > MAX_PEPLENGTH) {
      max_peplength = MAX_PEPLENGTH;
}
   int init = 1;
	unsigned int grp_counter = 0;

   std::vector< ppProtein* > prot_ptrs;
   proteinMap::const_iterator prot_iter = proteins.begin();
   while ( prot_iter != proteins.end() )
   {
      if (prot_iter -> second -> hasProbability()) {
         prot_ptrs.push_back( prot_iter -> second );
}
      ++prot_iter;
   }
   std::sort( prot_ptrs.begin(), prot_ptrs.end(), compareProteinProbabilitiesDesc );    // sort proteins in order of descending probability

   for ( unsigned int i = 0; i < prot_ptrs.size(); i++ ) {
      ppProtein* prot = prot_ptrs[ i ];
      double prob = roundProtProbability(prot -> getProbability());
      while( grp_counter < grp_indices.size() ) {
         PARANOID_ASSERT(isInit(group_probs[ grp_indices[ grp_counter ] ]));
         double group_prob = roundProtProbability(group_probs[ grp_indices[ grp_counter ] ]);   // round probability to nearest 0.01 before making comparison to protein probability
         if ( group_prob > prob && group_prob >= min_prot_prob ) {
            writeGroupExcel(grp_counter, min_pep_prob, index, EXCEL); 
            grp_counter++;
            index++;
         } else {
            break;
         }
      }
      if ( prob >= min_prot_prob && !prot -> hasGroupMembership() ) {
         writeProteinExcel(prob, prot, index++, 0, min_pep_prob, 'a', EXCEL);
      }
   } // next prot entry
   //  any stragglers????
   while(grp_counter < grp_indices.size()) {
      writeGroupExcel(grp_counter,min_pep_prob, index, EXCEL); 
      grp_counter++;
      index++;
   }
}
