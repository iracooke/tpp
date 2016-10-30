/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.



	// The implementation is through a trie, a failure
	// function and the ACOutput function.
	FTrie<t_alphabet,alphabetsize> tau;
	FFail<t_alphabet,alphabetsize> ff;
	ACOutput<t_alphabet,alphabetsize> out;

