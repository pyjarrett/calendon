#ifndef KN_HANDLE_H
#define KN_HANDLE_H

/*
 * Defines a handle type, with an adjustable size.
 */

#include <knell/kn.h>

#define KN_HANDLE_TYPE_DECL(Type, Prefix, HandleName) \
	void Prefix ## HandleName ## Init(void); \
	bool Prefix ## Create ## HandleName(Type* t);

#define KN_HANDLE_TYPE_DEFN(Type, Prefix, HandleName, idBits) \
	KN_STATIC_ASSERT((idBits >> 3) < sizeof(Type), \
		"More bits used for handle id than available."); \
	static Type next ## HandleName ## Id; \
	void Prefix ## HandleName ## Init() { \
		next ## HandleName ## Id = 0; \
	} \
 	\
	bool Prefix ## Create ## HandleName (Type* t) { \
		KN_ASSERT(t != NULL, "Cannot assign a " #HandleName " to a NULL."); \
		++ next ## HandleName ## Id; \
		return true; \
	}

#endif /* KN_HANDLE_H */
