#ifndef CN_DIMENSION_H
#define CN_DIMENSION_H

/**
 * Provides width/height dimensions in a known order, to prevent errors where
 * parameters are ordered incorrectly.
 */

#include <calendon/cn.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	float width, height;
} CnDimension2f;

typedef struct {
	uint32_t width, height;
} CnDimension2u32;


#ifdef __cplusplus
}
#endif

#endif /* CN_DIMENSION_H */
