#ifndef KN_HANDLE_H
#define KN_HANDLE_H

/*
 * Defines a handle type, with an adjustable size.
 */

#include <knell/kn.h>

/**
 * Declares a new handle type with its own initialization and create function.
 */
#define KN_DEFINE_HANDLE_TYPE(Type, Prefix, HandleName) \
	void Prefix ## HandleName ## Init(void); \
	bool Prefix ## Create ## HandleName(Type* t);

/**
 * Defines a new type with associated handle counter, maximum value and
 * increment function.
 */
#define KN_DECLARE_HANDLE_TYPE(Type, Prefix, HandleName, maxValue) \
	static Type next ## HandleName ## Id; \
	enum { Max ## HandleName ## Id = maxValue }; \
	void Prefix ## HandleName ## Init(void) { \
		next ## HandleName ## Id = 0; \
	} \
	bool Prefix ## Create ## HandleName (Type* t) { \
		KN_ASSERT(t != NULL, "Cannot assign a " #HandleName " to a NULL."); \
		++ next ## HandleName ## Id; \
		*t = next ## HandleName ## Id; \
		return true; \
	}

#endif /* KN_HANDLE_H */
