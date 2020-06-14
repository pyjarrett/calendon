#include "input-mouse.h"

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
