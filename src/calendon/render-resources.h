#ifndef CN_RENDER_RESOURCES_H
#define CN_RENDER_RESOURCES_H

/**
 * @file render-resources.h
 *
 * Resources common to both the interface for rendering, as well as the low
 * level internasl of rendering.
 *
 * Both high and low level renderers need access to sprite handles, as well as
 * clients who wish to use sprites.  It doesn't feel right to put this by
 * itself and it seems indicative of a poor design choice.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <calendon/cn.h>

/**
 * Opaque handle used to coordinate with the renderer to uniquely identify
 * sprites.
 */
typedef uint32_t CnSpriteId;

/**
 * Opaque handle for identifying font resources.
 */
typedef uint32_t CnFontId;

/**
 * The horizontal direction in which text glyphs are written.
 */
typedef enum {
	CnTextDirectionLeftToRight,
	CnTextDirectionRightToLeft,
} CnTextDirection;

typedef enum {
	CnLayoutDirectionHorizontal,
	CnLayoutDirectionVertical
} CnLayoutDirection;

/**
 * Various ways that text drawing can be manipulated.
 */
typedef struct {
	CnFloat2 position;
	CnRGBA8u color;
	CnLayoutDirection layout;
	CnTextDirection printDirection;
} CnTextDrawParams;

#ifdef __cplusplus
}
#endif

#endif /* CN_RENDER_RESOURCES_H */
