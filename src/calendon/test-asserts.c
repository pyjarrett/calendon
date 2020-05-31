#if CN_TESTING
#include <calendon/kn.h>
#include <setjmp.h>
CN_API jmp_buf cnTest_AssertJumpBuffer;
CN_API jmp_buf cnTest_AssertUnexpectedJumpBuffer;
CN_API bool cnTest_ExpectingAssert = false;
#endif
