#ifndef CN_FLOAT_H
#define CN_FLOAT_H

#include <calendon/cn.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Floating point utilities.
 */

CN_API float cnFloat_RelativeDiff(float a, float b);

CN_API float cnFloat_Clamp(const float value, const float min, const float max);

#ifdef __cplusplus
}
#endif

#endif /* CN_FLOAT_H */
