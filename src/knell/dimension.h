#ifndef KN_DIMENSION_H
#define KN_DIMENSION_H

/**
 * Provides width/height dimensions in a known order, to prevent errors where
 * parameters are ordered incorrectly.
 */

#include <knell/kn.h>

typedef struct {
	float width, height;
} Dimension2f;

typedef struct {
	uint32_t width, height;
} Dimension2u32;

#endif /* KN_DIMENSION_H */
