/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts and FIRE Lite class libraries.


#ifndef STATE_HPP
#define STATE_HPP
#define IN_STATE_HPP

#ifdef _MSC_VER // microsoft weirdness
#pragma warning(disable:4786) // don't bark about "identifier was truncated to '255' characters in the browser information"
#endif


// Define a State to be just an integer.
// Used in tries and so on.
typedef int State;

// Define a first State.
const State FIRSTSTATE = 0;
// Define an invalid State.
const State INVALIDSTATE = -1;

#undef IN_STATE_HPP
#endif

