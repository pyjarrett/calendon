/*
 * Both high and low level renderers need access to sprite handles, as well as
 * clients who wish to use sprites.  It doesn't feel right to put this by
 * itself and it seems indicative of a poor design choice.
 */
#ifndef KN_RENDER_RESOURCES_H
#define KN_RENDER_RESOURCES_H

/**
 * Opaque handle used to coordinate with the renderer to uniquely identify
 * sprites.
 */
KN_API typedef uint32_t SpriteId;

/**
 * Opaque handle for identifying font resources.
 */
KN_API typedef uint32_t FontId;

#endif /* KN_RENDER_RESOURCES_H */
