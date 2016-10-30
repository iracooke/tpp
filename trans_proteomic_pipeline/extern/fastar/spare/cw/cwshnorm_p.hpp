/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.



	// The order of the following members is critical for the
	// correct constructor invocation order!

	// The following is wasted space, since it is only used in
	// the ctor.
	RFail<t_alphabet,alphabetsize> *fr;
	// The real representation.
	D1<t_alphabet,alphabetsize> d1;
	D2<t_alphabet,alphabetsize> d2;
	CharCW<t_alphabet,alphabetsize> charCW;

