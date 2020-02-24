#ifndef KN_ASSERTION_TESTING_H
#define KN_ASSERTION_TESTING_H

#if KN_ASSERTION_TESTING
#include <setjmp.h>
extern KN_API jmp_buf knTest_AssertJumpBuffer;
extern KN_API bool knTest_ExpectingAssert;
#define KN_TEST_ASSERTION_OCCURRED 1357
#endif

#endif /* KN_ASSERTION_TESTING_H */
