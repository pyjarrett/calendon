#include <knell/binary.h>

/**
 * If n is a power of two, then (n-1) will be all 1 bits, and (n-1) and n will
 * share no bits.  If n is not a power of two, then subtracting 1 will result in
 * some bits not flipping in n, so n and (n-1) will share bits.
 */
KN_TEST_API bool isPowerOfTwo(const uint32_t n) {
	if (n == 0 || n == 1) return true;
	return !(n & (n-1));
}
