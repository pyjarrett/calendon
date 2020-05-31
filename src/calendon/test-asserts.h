#ifndef CN_TEST_ASSERTS_H
#define CN_TEST_ASSERTS_H

/*
 * Calendon uses its own assertion mechanism throughout its code to verify
 * assumptions which must be true for correct and safe code execution.
 *
 * To support, document, and verify assertions, Calendon provides assertion
 * checking as part of its testing capabilities, so tests can be written to
 * verify presence or lack of assertions.  Note that this verifies any assertion
 * was trigger, not a particular assertion.
 */

#if CN_TESTING
#include <setjmp.h>
extern CN_API jmp_buf cnTest_AssertJumpBuffer;
extern CN_API jmp_buf cnTest_AssertUnexpectedJumpBuffer;

/**
 * Value set when code is expected to trigger an assertion.
 */
extern CN_API bool cnTest_ExpectingAssert;

/**
 * Long jump value returned when an assertion was expected.
 */
#define CN_TEST_ASSERTION_OCCURRED 1357

/**
 * Long jump value returned when an assertion was unexpected.
 */
#define CN_TEST_ASSERTION_UNEXPECTED 631
#endif

#endif /* CN_TEST_ASSERTS_H */
