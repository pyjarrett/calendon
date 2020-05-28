#ifndef KN_DIMENSION_H
#define KN_DIMENSION_H

/**
 * Provides width/height dimensions in a known order, to prevent errors where
 * parameters are ordered incorrectly.
 */

#include <knell/kn.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	float width, height;
} Dimension2f;

typedef struct {
	uint32_t width, height;
} Dimension2u32;


#ifdef __cplusplus
}
#endif

#endif /* KN_DIMENSION_H */
