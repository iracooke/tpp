/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.



	// The order of the following is critical to the correct
	// order in the constructor.

	// The following is wasted space, since it is only used in
	// the ctor.
	RFail<t_alphabet,alphabetsize> *fr;

	// The actual implementation.
	DOpt<t_alphabet,alphabetsize> dopt;
	D2<t_alphabet,alphabetsize> d2;

