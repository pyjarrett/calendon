#ifndef CN_HANDLE_H
#define CN_HANDLE_H

/*
 * Defines a handle type, with an adjustable size.
 */

#include <calendon/cn.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Declares a new handle type with its own initialization and create function.
 */
#define CN_DEFINE_HANDLE_TYPE(Type, Prefix, HandleName) \
	void Prefix ## HandleName ## Init(void); \
	bool Prefix ## Create ## HandleName(Type* t);

/**
 * Defines a new type with associated handle counter, maximum value and
 * increment function.
 */
#define CN_DECLARE_HANDLE_TYPE(Type, Prefix, HandleName, maxValue) \
	static Type next ## HandleName ## Id; \
	enum { Max ## HandleName ## Id = maxValue }; \
	void Prefix ## HandleName ## Init(void) { \
		next ## HandleName ## Id = 0; \
	} \
	bool Prefix ## Create ## HandleName (Type* t) { \
		CN_ASSERT(t != NULL, "Cannot assign a " #HandleName " to a NULL."); \
		++ next ## HandleName ## Id; \
		*t = next ## HandleName ## Id; \
		return true; \
	}

#ifdef __cplusplus
}
#endif

#endif /* CN_HANDLE_H */
