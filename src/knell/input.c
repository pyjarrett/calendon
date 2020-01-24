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
