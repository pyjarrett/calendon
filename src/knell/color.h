#ifndef KN_COLOR_H
#define KN_COLOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif /* KN_COLOR_H */

