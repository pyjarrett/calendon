#include "input.h"

void KeySet_reset(KeySet* list)
{
	list->size = 0;
}

bool KeySet_add(KeySet* list, SDL_Keycode key)
{
	if (KeySet_contains(list, key)) {
		return false;
	}

	if (list->size + 1 >= KN_KEY_LIST_MAX_SIZE) {
		return false;
	}
	list->keys[list->size] = key;
	list->size = list->size + 1;
	return true;
}

static void KeySet_removeIndex(KeySet* list, uint32_t index)
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

void KeySet_remove(KeySet* list, SDL_Keycode key)
{
	for (uint32_t i = 0; i < list->size; ++i) {
		if (list->keys[i] == key) {
			KeySet_removeIndex(list, i);
		}
	}
}

bool KeySet_contains(KeySet* list, SDL_Keycode key)
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
void Mouse_Still(Mouse* m)
{
	KN_ASSERT(m, "Cannot keep a null mouse still");
	m->dx = 0;
	m->dy = 0;
}

void Mouse_Move(Mouse* m, int32_t x, int32_t y, int32_t dx, int32_t dy)
{
	KN_ASSERT(m, "Cannot move a null mouse");
	m->x = x;
	m->y = y;
	m->dx = dx;
	m->dy = dy;
}
