#ifndef CN_COLOR_H
#define CN_COLOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
} CnRGBA8u;

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} CnRGB8u;

#ifdef __cplusplus
}
#endif

#endif /* CN_COLOR_H */

