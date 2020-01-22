#ifndef KN_COLOR_H
#define KN_COLOR_H

#include <stdint.h>

typedef struct {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
} rgba8i;

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb8;

#endif /* KN_COLOR_H */

