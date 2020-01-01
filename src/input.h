#include "kn.h"
/*
 * Handling of player inputs.
 */
#define KN_KEY_LIST_MAX_SIZE 128

typedef struct {
	SDL_Scancode keys[KN_KEY_LIST_MAX_SIZE];
	uint32_t size;
} KeySet;

void KeySet_reset(KeySet* list);
bool KeySet_add(KeySet* list, SDL_Scancode key);
void KeySet_remove(KeySet* list, SDL_Scancode key);
bool KeySet_contains(KeySet* list, SDL_Scancode key);

/**
 * Lists of which keys are down, and which are up.
 */
typedef struct {
	KeySet down;
	KeySet up;
} KeyInputs;

