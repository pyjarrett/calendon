#ifndef KN_SPRITE_H
#define KN_SPRITE_H

/*
 * Both high and low level renderers need access to sprite handles, as well as
 * clients who wish to use sprites.  It doesn't feel right to put this by
 * itself and it seems indicative of a poor design choice.
 */

/**
 * Opaque handle used to coordinate with the renderer to uniquely identify
 * sprites.
 */
KN_API typedef uint32_t SpriteId;

#endif /* KN_SPRITE_H */
