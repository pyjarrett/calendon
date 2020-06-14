#include "input-keyset.h"

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
