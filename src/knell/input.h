#ifndef KN_INPUT_H
#define KN_INPUT_H

#include "kn.h"
#include "compat-sdl.h"

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

void KeySet_Reset(KeySet* list);
bool KeySet_Add(KeySet* list, SDL_Keycode key);
void KeySet_Remove(KeySet* list, SDL_Keycode key);
bool KeySet_Contains(KeySet* list, SDL_Keycode key);

void Mouse_Still(Mouse* m);
void Mouse_Move(Mouse*, int32_t x, int32_t y, int32_t dx, int32_t dy);

#endif /* KN_INPUT */
