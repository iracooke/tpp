/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.



	// The order of the following is critical. See acmopt.hpp for
	// more on why.
	FTrie<t_alphabet,alphabetsize> *trie;

	// Implementation is through the extended forward trie, the
	// forward failure function and the ACOutput function.
	EFTrie<t_alphabet,alphabetsize> tauef;
	FFail<t_alphabet,alphabetsize> ff;
	ACOutput<t_alphabet,alphabetsize> out;

