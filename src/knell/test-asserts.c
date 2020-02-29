#if KN_TESTING
#include <knell/kn.h>
#include <setjmp.h>
KN_API jmp_buf knTest_AssertJumpBuffer;
KN_API bool knTest_ExpectingAssert = false;
#endif
