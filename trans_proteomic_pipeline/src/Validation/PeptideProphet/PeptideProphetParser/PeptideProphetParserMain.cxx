
#include "PeptideProphetParser.h"
#include "Parsers/Parser/Parser.h"
#include "common/TPPVersion.h" 
#include "Parsers/Parser/TagListComparator.h" // for REGRESSION_TEST_CMDLINE_ARG defn

int main(int argc, char** argv) {
  hooks_tpp handler(argc,argv); // set up install paths etc

  // regression test stuff - bpratt Insilicos LLC, Nov 2005
  char *testArg=NULL;
  for (int argNum=1;argNum<argc;argNum++) {
    if (!strncmp(argv[argNum],REGRESSION_TEST_CMDLINE_ARG,strlen(REGRESSION_TEST_CMDLINE_ARG))) {
        // learn or run a test
        testArg = argv[argNum];
        // collapse down over this
        while (argNum < argc-1) {
           argv[argNum] = argv[argNum+1];
           argNum++;
        }
        argc--;
     }
  }

  if (argc < 2) {
   cerr <<  argv[0] << " (" << szTPPVersionInfo << ")" << endl;
   cerr << "usage: PeptideProphetParser <pepxmlFile> [<options> ...]" << endl; 
   cerr << "options: " << endl;

   cerr << "\tEXCLUDE : exclude deltaCn*, Mascot*, and Comet* results from results (default Penalize * results)" << endl;
   cerr << "\tLEAVE : leave alone deltaCn*, Mascot*, and Comet* results from results (default Penalize * results)" << endl;
   cerr << "\tPERFECTLIB : multiply by SpectraST library probability (default NO)" << endl;
   cerr << "\tICAT : apply ICAT model (default Autodetect ICAT)" << endl;
   cerr << "\tNOICAT : do no apply ICAT model (default Autodetect ICAT)" << endl;
   cerr << "\tZERO : report results with minimum probability 0 (default 0.05)" << endl;
   cerr << "\tACCMASS : use Accurate Mass model binning (default NO)" << endl;
   cerr << "\tCLEVEL=<number> : set Conservative Level in neg_stdev from the neg_mean, low numbers are less conservative, high numbers are more conservative (default 0)" << endl;
   cerr << "\tPPM : use ppm mass error instead of Daltons for mass modeling (default Daltons)" << endl;
   cerr << "\tNOMASS : disable mass model (default NO)" << endl;
   cerr << "\tPI : enable peptide pI model (default NO)" << endl;
   cerr << "\tMINPINTT=<number> : minimum number of NTT in a peptide used for positive pI model (default 2)"<< endl;
   cerr << "\tMINPIPROB=<number> : minimum probability after first pass of a peptide used for positive pI model (default 0.9)" << endl;
   cerr << "\tRT : enable peptide RT model (default NO)" << endl;
   cerr << "\tMINRTNTT=<number> : minimum number of NTT in a peptide used for positive RT model (default 2)" << endl;
   cerr << "\tMINRTPROB=<number> : minimum probability after first pass of a peptide used for positive RT model (default 0.9)" << endl;
   cerr << "\tRTCAT=<rtcatalog file> : enable peptide RT model, use <rtcatalog_file> peptide RTs when available as the theoretical value" << endl;
   cerr << "\tGLYC : enable peptide Glyco motif model (default NO)" << endl;
   cerr << "\tPHOSPHO : enable peptide Phospho motif model (default NO)" << endl;
   cerr << "\tMALDI : enable MALDI mode (default NO)" << endl;
   cerr << "\tINSTRWARN : warn and continue if combined data was generated by different instrument models (default NO)" << endl;
   cerr << "\tMINPROB=<number> : report results with minimum probability <number> (default 0.05)" << endl;
   cerr << "\tDECOY=<decoy_prot_prefix> : semi-supervised mode, protein name prefix to identify Decoy entries" << endl;
   cerr << "\tDECOYPROBS : compute possible non-zero probabilities for Decoy entries on the last iteration " << endl;
   cerr << "\tIGNORECHG=<charge> : can be used multiple times to specify all charge states to exclude from modeling" << endl;
   cerr << "\tNONTT : disable NTT enzymatic termini model (default NO)" << endl;
   cerr << "\tNONMC : disable NMC missed cleavage model (default NO)" << endl;
   cerr << "\tEXPECTSCORE : use expectation value as the only contributor to the f-value for modeling (default NO)" << endl;
   cerr << "\tNONPARAM : use semi-parametric modeling, must be used in conjunction with DECOY= option (default NO)" << endl;
   cerr << "\tNEGGAMMA : use Gamma distribution to model the negative hits" << endl;
   cerr << "\tFORCEDISTR : bypass quality control checks, report model despite bad modelling" << endl;
   cerr << "\tOPTIMIZEFVAL : (SpectraST only) optimize f-value function f(dot,delta) using PCA (default NO)" << endl;
   cerr << endl;
   
   exit(1);
  }

  // options are all args further down...
  int k,opt_len = 2;
  for(k = 2; k < argc; k++) {
    opt_len += strlen(argv[k]) + 1;
    //if(k > 2)
    // opt_len++;
  }
  char* options = new char[opt_len];
  options[0] = ' ';
  options[1] = 0;
  for(k = 2; k < argc; k++) {
    strcat(options, argv[k]);
    strcat(options, " ");
  }
    
  PeptideProphetParser *p;
  if(argc > 1)
    p = new PeptideProphetParser(argv[1], options, testArg);
  else {
	  std::string fname("new");
	  fname += get_pepxml_dot_ext();
	  p = new PeptideProphetParser(fname.c_str(), "", testArg);
  }
  bool success = p && p->success();
  delete p;
  delete[] options;
  return success?0:1;
}
