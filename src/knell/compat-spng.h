#pragma once

#if __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#endif

#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#endif

#include <spng.h>

#if __clang__
#pragma clang diagnostic pop
#endif

#if __GNUC__
#pragma GCC diagnostic pop
#endif
