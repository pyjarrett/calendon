#ifndef KN_INPUT_H
#define KN_INPUT_H

#include <knell/kn.h>

#include <knell/compat-sdl.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Handling of player inputs.
 */
#define KN_KEY_LIST_MAX_SIZE 128

typedef struct {
	SDL_Keycode keys[KN_KEY_LIST_MAX_SIZE];
	uint32_t size;
} KeySet;

/**
 * Lists of which keys are down, and which are up.
 */
typedef struct {
	KeySet down;
	KeySet up;
} KeyInputs;

typedef struct {
	int x, y;
	int dx, dy;
} Mouse;

KN_API void KeySet_Reset(KeySet* list);
KN_API bool KeySet_Add(KeySet* list, SDL_Keycode key);
KN_API void KeySet_Remove(KeySet* list, SDL_Keycode key);
KN_API bool KeySet_Contains(KeySet* list, SDL_Keycode key);

void Mouse_Still(Mouse* m);
void Mouse_Move(Mouse*, int32_t x, int32_t y, int32_t dx, int32_t dy);

#ifdef __cplusplus
}
#endif

#endif /* KN_INPUT */
