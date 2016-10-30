/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#ifndef CWS_HPP
#define CWS_HPP
#define IN_CWS_HPP

#include "pm-cw.hpp"
#include "cwshnla.hpp"
#include "cwshnorm.hpp"
#include "cwshwbm.hpp"
#include "cwshopt.hpp"
#include "cwshrla.hpp"
#include "cwshnaiv.hpp"

// A couple of Commentz-Walter pattern matchers.
typedef CWShiftRLA<standardAlphabet,standardAlphabetSize> stdRLA;
typedef PMCW<stdRLA,standardAlphabet,standardAlphabetSize> PMCWRLA;
typedef CWShiftOpt<standardAlphabet,standardAlphabetSize> stdOpt;
typedef PMCW<stdOpt,standardAlphabet,standardAlphabetSize> PMCWOpt;
typedef CWShiftNorm<standardAlphabet,standardAlphabetSize> stdNor;
typedef PMCW<stdNor,standardAlphabet,standardAlphabetSize> PMCWNorm;
typedef CWShiftWBM<standardAlphabet,standardAlphabetSize> stdWBM;
typedef PMCW<stdWBM,standardAlphabet,standardAlphabetSize> PMCWWBM;
typedef CWShiftNLA<standardAlphabet,standardAlphabetSize> stdNLA;
typedef PMCW<stdNLA,standardAlphabet,standardAlphabetSize> PMCWNLA;
typedef CWShiftNaive<standardAlphabet,standardAlphabetSize> stdNai;
typedef PMCW<stdNai,standardAlphabet,standardAlphabetSize> PMCWNaive;

#undef IN_CWS_HPP
#endif

