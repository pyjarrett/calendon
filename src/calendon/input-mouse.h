#ifndef CN_INPUT_MOUSE_H
#define CN_INPUT_MOUSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <calendon/cn.h>

typedef struct {
	int x, y;
	int dx, dy;
} CnMouse;

void cnMouse_Still(CnMouse* m);
void cnMouse_Move(CnMouse* m, int32_t x, int32_t y, int32_t dx, int32_t dy);

#ifdef __cplusplus
}
#endif

#endif /* CN_INPUT_MOUSE_H */
