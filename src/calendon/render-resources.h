/*
 * Both high and low level renderers need access to sprite handles, as well as
 * clients who wish to use sprites.  It doesn't feel right to put this by
 * itself and it seems indicative of a poor design choice.
 */
#ifndef CN_RENDER_RESOURCES_H
#define CN_RENDER_RESOURCES_H

#include <calendon/cn.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque handle used to coordinate with the renderer to uniquely identify
 * sprites.
 */
typedef uint32_t CnSpriteId;

/**
 * Opaque handle for identifying font resources.
 */
typedef uint32_t CnFontId;

enum {
	CnPrintDirectionLeftToRight,
	CnPrintDirectionRightToLeft,
};

enum {
	CnLayoutHorizontal,
	CnLayoutVertical
};

/**
 * Various ways that text drawing can be manipulated.
 */
typedef struct {
	CnFloat2 position;
	CnRGBA8u color;
	uint32_t layout;
	uint32_t printDirection;
} CnTextDrawParams;

#ifdef __cplusplus
}
#endif

#endif /* CN_RENDER_RESOURCES_H */
