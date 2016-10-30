/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.



	// The representation: nested maps.
	StateTo< SymbolTo< State, t_alphabet, alphabetsize >, t_alphabet, alphabetsize> rep;
	// BFT stuff:
	StateTo<int, t_alphabet, alphabetsize> depth;

