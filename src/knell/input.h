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

void KeySet_reset(KeySet* list);
bool KeySet_add(KeySet* list, SDL_Keycode key);
void KeySet_remove(KeySet* list, SDL_Keycode key);
bool KeySet_contains(KeySet* list, SDL_Keycode key);

/**
 * Lists of which keys are down, and which are up.
 */
typedef struct {
	KeySet down;
	KeySet up;
} KeyInputs;
