#ifndef CN_COMPAT_SPNG_H
#define CN_COMPAT_SPNG_H

#ifdef __GNUC__
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#endif

#ifdef __clang__
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#endif

#ifdef _MSC_VER
	#pragma warning(push)
	// "different 'const' qualifiers"
	#pragma warning(disable: 4090)

	// "conversion from 'size_t' to 'uInt', possible loss of data"
	#pragma warning(disable: 4267)
#endif

#include <spng.h>

#ifdef __GNUC__
	#pragma GCC diagnostic pop
#endif

#ifdef __clang__
	#pragma clang diagnostic pop
#endif

#ifdef _MSC_VER
	#pragma warning(pop)
#endif

#endif /* CN_COMPAT_SPNG_H */
