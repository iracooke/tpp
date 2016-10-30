/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.



	// Need a reverse trie, a Commentz-Walter output function,
	// and a shifter.
	// The order here is crucial.
	RTrie<t_alphabet,alphabetsize> trie;
	CWOutput<t_alphabet,alphabetsize> out;
	T shift;

