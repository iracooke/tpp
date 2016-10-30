/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.


#ifndef BMS_HPP
#define BMS_HPP
#define IN_BMS_HPP

#include "stravfwd.hpp"
#include "stravrev.hpp"
/* Not implemented.
#include "stravom.hpp"
#include "stravran.hpp"*/

#include "bmslnone.hpp"
#include "bmslsfc.hpp"
#include "bmslfst1.hpp"
#include "bmslfst2.hpp"
///* Not implemented.
//#include "bmslslfc.hpp"*/

#include "bmshnaiv.hpp"
#include "bmsh1-1.hpp"
#include "bmsh1-2.hpp"

#include "pm-bm.hpp"

// Define some of the classic Boyer-Moore variants.
// It's not worth typedefing all of them. Just instantiate them
// in the client code.
// 	For example: PMBM< STravFWD,SLNone,BMShift11<STravFWD> > M


#undef IN_BMS_HPP
#endif

