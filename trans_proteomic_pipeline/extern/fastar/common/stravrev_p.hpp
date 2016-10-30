/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.

//
// changes for nearly-zero-copy operation by bpratt Insilicos LLC July 2006
//


	// Hide the argumentless constructor.
	STravREV() :
			len( -1 ) {
	}

	// Keep the length of the traversed kw_t.
	const int len;

