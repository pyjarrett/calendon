#ifndef CN_ASPECT_RATIO_H
#define CN_ASPECT_RATIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <calendon/math2.h>

CN_API CnTransform2 cnAspectRatio_fit(CnDimension2u32 desired, CnDimension2u32 actual);

#ifdef __cplusplus
}
#endif

#endif /* CN_ASPECT_RATIO_H */
