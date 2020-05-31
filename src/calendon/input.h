#ifndef CN_INPUT_H
#define CN_INPUT_H

#include <calendon/cn.h>

#include <calendon/compat-sdl.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Handling of player inputs.
 */
#define CN_KEY_LIST_MAX_SIZE 128

typedef struct {
	SDL_Keycode keys[CN_KEY_LIST_MAX_SIZE];
	uint32_t size;
} CnKeySet;

/**
 * Lists of which keys are down, and which are up.
 */
typedef struct {
	CnKeySet down;
	CnKeySet up;
} CnKeyInputs;

typedef struct {
	int x, y;
	int dx, dy;
} CnMouse;

CN_API void cnKeySet_Reset(CnKeySet* list);
CN_API bool cnKeySet_Add(CnKeySet* list, SDL_Keycode key);
CN_API void cnKeySet_Remove(CnKeySet* list, SDL_Keycode key);
CN_API bool cnKeySet_Contains(CnKeySet* list, SDL_Keycode key);

void cnMouse_Still(CnMouse* m);
void cnMouse_Move(CnMouse* m, int32_t x, int32_t y, int32_t dx, int32_t dy);

#ifdef __cplusplus
}
#endif

#endif /* CN_INPUT */
