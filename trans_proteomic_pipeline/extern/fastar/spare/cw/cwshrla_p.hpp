/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.



	// The order of the following members is critical to the
	// correctness of the constructors.

	// The following is wasted space, since it is only used in
	// the ctor.
	RFail<t_alphabet,alphabetsize> *fr;

	// The actual implementation.
	DOpt<t_alphabet,alphabetsize> dopt;
	D2<t_alphabet,alphabetsize> d2;
	CharRLA<t_alphabet,alphabetsize> rla;

