/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.



	// The order of the following is critical, since it affects
	// the constructor order!

	// In order to construct the above functions, we need the
	// following; the wasteage is a tradeoff for speed.
	FTrie<t_alphabet,alphabetsize> *trie;
	FFail<t_alphabet,alphabetsize> *fail;

	// The actual implementation.
	Gamma<t_alphabet,alphabetsize> gf;
	ACOutput<t_alphabet,alphabetsize> out;

