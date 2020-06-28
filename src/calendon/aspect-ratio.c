#include "aspect-ratio.h"

#include <math.h>

/**
 * Create a transform to put content in the desired resolution onto a screen of
 * a different set of dimensions.
 */
CnTransform2 cnAspectRatio_fit(CnDimension2u32 desired, CnDimension2u32 actual)
{
	CN_ASSERT(desired.width != 0, "Cannot fit from a zero width resolution.");
	CN_ASSERT(desired.height != 0, "Cannot fit from a zero height resolution.");
	CN_ASSERT(actual.width != 0, "Cannot fit into a zero width resolution.");
	CN_ASSERT(actual.height != 0, "Cannot fit into a zero height resolution.");

	const float heightRatio = (float)actual.height / desired.height;
	const float widthRatio = (float)actual.width / desired.width;

	const float scale = fminf(heightRatio, widthRatio);
	const float dx = (actual.width - scale * desired.width) / 2.0f;
	const float dy = (actual.height - scale * desired.height) / 2.0f;

	CN_ASSERT(0.0f < scale, "Scale must be greater than zero.");
	CN_ASSERT(scale * desired.width <= actual.width, "Incorrect scale chosen based on width.");
	CN_ASSERT(scale * desired.height <= actual.height, "Incorrect scale chosen based on height.");
	CN_ASSERT(abs(actual.width - scale * desired.width) < 2
		|| abs(actual.height - scale * desired.height) < 2,
		"Did not fit the desired resolution into either actual dimension.");

	return cnTransform2_Combine(
		cnTransform2_MakeTranslateXY(dx, dy),
		cnTransform2_MakeUniformScale(scale));
}
