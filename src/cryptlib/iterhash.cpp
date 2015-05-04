// iterhash.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "iterhash.h"

#ifdef WORD64_AVAILABLE
template<> class IteratedHash<word64>;
#endif

template<> class IteratedHash<word32>;

