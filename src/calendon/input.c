#include "input.h"

void cnKeySet_Reset(CnKeySet* list)
{
	list->size = 0;
}

bool cnKeySet_Add(CnKeySet* list, SDL_Keycode key)
{
	if (cnKeySet_Contains(list, key)) {
		return false;
	}

	if (list->size + 1 >= CN_KEY_LIST_MAX_SIZE) {
		return false;
	}
	list->keys[list->size] = key;
	list->size = list->size + 1;
	return true;
}

static void cnKeySet_RemoveIndex(CnKeySet* list, uint32_t index)
{
	if (list->size == 0 || index >= list->size) {
		return;
	}

	if (list->size == 1) {
		list->size = 0;
		return;
	}

	list->keys[index] = list->keys[list->size - 1];
	list->size = list->size - 1;
}

void cnKeySet_Remove(CnKeySet* list, SDL_Keycode key)
{
	for (uint32_t i = 0; i < list->size; ++i) {
		if (list->keys[i] == key) {
			cnKeySet_RemoveIndex(list, i);
		}
	}
}

bool cnKeySet_Contains(CnKeySet* list, SDL_Keycode key)
{
	for (uint32_t i = 0; i < list->size; ++i) {
		if (list->keys[i] == key) {
			return true;
		}
	}
	return false;
}

/**
 * The mouse remained still, so clear out dx and dy movements.
 */
void cnMouse_Still(CnMouse* m)
{
	CN_ASSERT(m, "Cannot keep a null mouse still");
	m->dx = 0;
	m->dy = 0;
}

void cnMouse_Move(CnMouse* m, int32_t x, int32_t y, int32_t dx, int32_t dy)
{
	CN_ASSERT(m, "Cannot move a null mouse");
	m->x = x;
	m->y = y;
	m->dx = dx;
	m->dy = dy;
}
