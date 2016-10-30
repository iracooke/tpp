/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.



	// The shift array. It is important that we use Array of
	// SymbolTo, and not the other way around, for resizing
	// purposes.
	std::vector< SymbolTo<int,t_alphabet,alphabetsize> > rep;

