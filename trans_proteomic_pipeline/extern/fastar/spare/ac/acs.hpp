/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#ifndef ACS_HPP
#define ACS_HPP
#define IN_ACS_HPP

#include "pm-ac.hpp"
#include "acmopt.hpp"
#include "acmfail.hpp"
#include "acmkmpfl.hpp"

// A couple of Aho-Corasick pattern matchers.
typedef ACMachineOpt<standardAlphabet,standardAlphabetSize> ACstdOpt;
typedef PMAC<ACstdOpt,standardAlphabet,standardAlphabetSize> PMACOpt_std;
typedef ACMachineOpt<peptideAlphabet,peptideAlphabetSize> ACpepOpt;
typedef PMAC<ACpepOpt,peptideAlphabet,peptideAlphabetSize> PMACOpt_peptides;

typedef ACMachineFail<standardAlphabet,standardAlphabetSize> ACstdFail;
typedef PMAC<ACstdFail,standardAlphabet,standardAlphabetSize> PMACFail_std;
typedef ACMachineFail<peptideAlphabet,peptideAlphabetSize> ACpepFail;
typedef PMAC<ACpepFail,peptideAlphabet,peptideAlphabetSize> PMACFail_peptides;

typedef ACMachineKMPFail<standardAlphabet,standardAlphabetSize> ACstdKMPFail;
typedef PMAC<ACstdKMPFail,standardAlphabet,standardAlphabetSize> PMACKMPFail_std;
typedef ACMachineKMPFail<peptideAlphabet,peptideAlphabetSize> ACpepKMPFail;
typedef PMAC<ACpepKMPFail,peptideAlphabet,peptideAlphabetSize> PMACKMPFail_peptides;

#undef IN_ACS_HPP
#endif

