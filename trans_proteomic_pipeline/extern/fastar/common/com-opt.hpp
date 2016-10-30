/* (c) Copyright 1995-2004 by Bruce W. Watson / Loek Cleophas */
// SPARE Parts class library.
#ifndef COM_OPT_HPP
#define COM_OPT_HPP
#define IN_COM_OPT_HPP

#ifdef SPAREPARTS_DEBUG
#define SPAREPARTS_ASSERT assert
#else
#define SPAREPARTS_ASSERT(stmt)
#endif

#ifdef INLINING
	#define INLINE inline
#else
	#define INLINE
#endif

#undef IN_COM_OPT_HPP
#endif

