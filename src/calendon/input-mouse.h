#ifndef CN_INPUT_MOUSE_H
#define CN_INPUT_MOUSE_H

#include <calendon/cn.h>

typedef struct {
	int x, y;
	int dx, dy;
} CnMouse;

void cnMouse_Still(CnMouse* m);
void cnMouse_Move(CnMouse* m, int32_t x, int32_t y, int32_t dx, int32_t dy);

#endif /* CN_INPUT_MOUSE_H */
