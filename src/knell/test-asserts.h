#ifndef KN_TEST_ASSERTS_H
#define KN_TEST_ASSERTS_H

/*
 * Knell uses its own assertion mechanism throughout its code to verify
 * assumptions which must be true for correct and safe code execution.
 *
 * To support, document, and verify assertions, Knell provides assertion
 * checking as part of its testing capabilities, so tests can be written to
 * verify presence or lack of assertions.  Note that this verifies any assertion
 * was trigger, not a particular assertion.
 */

#if KN_TESTING
#include <setjmp.h>
extern KN_API jmp_buf knTest_AssertJumpBuffer;

/**
 * Value set when code is expected to trigger an assertion.
 */
extern KN_API bool knTest_ExpectingAssert;

/**
 * Long jump value returned when an assertion was expected.
 */
#define KN_TEST_ASSERTION_OCCURRED 1357
#endif

#endif /* KN_TEST_ASSERTS_H */
