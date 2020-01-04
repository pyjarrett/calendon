#include "bits.h"

bool isPowerOfTwo64(uint64_t n)
{
	return !(n & (n - 1LL));
}

