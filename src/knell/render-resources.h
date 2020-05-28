/*
 * Both high and low level renderers need access to sprite handles, as well as
 * clients who wish to use sprites.  It doesn't feel right to put this by
 * itself and it seems indicative of a poor design choice.
 */
#ifndef KN_RENDER_RESOURCES_H
#define KN_RENDER_RESOURCES_H

#include <knell/kn.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque handle used to coordinate with the renderer to uniquely identify
 * sprites.
 */
typedef uint32_t SpriteId;

/**
 * Opaque handle for identifying font resources.
 */
typedef uint32_t FontId;

enum {
	PrintDirectionLeftToRight,
	PrintDirectionRightToLeft,
	LayoutHorizontal,
	LayoutVertical
};

/**
 * Various ways that text drawing can be manipulated.
 */
typedef struct {
	float2 position;
	rgba8i color;
	uint32_t layout;
	uint32_t printDirection;
} TextDrawParams;

#ifdef __cplusplus
}
#endif

#endif /* KN_RENDER_RESOURCES_H */
