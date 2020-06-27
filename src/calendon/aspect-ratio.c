#include "aspect-ratio.h"

CnTransform2 cnAspectRatio_fit(CnDimension2u32 desired, CnDimension2u32 actual)
{
	CN_UNUSED(desired);
	CN_UNUSED(actual);
	return cnTransform2_MakeIdentity();
}
