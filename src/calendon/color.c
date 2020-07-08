#include "color.h"

CnOpaqueColor cnOpaqueColor_MakeRGBf(float red, float green, float blue)
{
	return (CnOpaqueColor) { .red = red, .green = green, .blue = blue };
}

CnOpaqueColor cnOpaqueColor_MakeRGBu8(uint8_t red, uint8_t green, uint8_t blue)
{
	return cnOpaqueColor_MakeRGBf(
		(float)red / 255.0f,
		(float)green / 255.0f,
		(float)blue / 255.0f);
}
