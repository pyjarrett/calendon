#pragma once

/**
 * @file test.h
 * @addtogroup CalendonTest
 *
 * A very simple test framework for Calendon.
 *
 * # Intent
 *
 * I didn't originally intend to write an entire test framework for Calendon.  I
 * was looking for something with automatic test discovery and execution,
 * so simple it would be hard to screw up, and still able to cover many possible
 * test cases.  Additionally, I wanted that would eventually allow for testing
 * at runtime within the driver using similar notations.  None of the frameworks
 * I looked at seemed lightweight enough to necessitate bringing in another
 * outside dependency.
 *
 * # Usage
 *
 * Uses of this testing framework should only concern themselves with the macros
 * prefixed by `CN_TEST_`, and call none of the functions contained here
 * directly.
 *
 * Only one test suite may be written per file.  When compiled, each test
 * source file generates a separate executable, which returns `EXIT_SUCCESS` on
 * a successful run, and `EXIT_FAILURE` if any test fails.
 *
 * The `CN_TESTING` macro causes Calendon's library to expose functions annotated
 * with the `CN_TEST_API` macro to be exported, which is done to create the
 * `calendon-testable` target.  This allows normally hidden functions within
 * Calendon to be tested, and also serves to document which functions cannot or are
 * not tested.  The `CN_API` and `CN_TEST_API` macros to annotate a
 * function are exclusive, annotating a function with both is an error.
 *
 * # Limits
 *
 * `CN_TEST_*` doesn't require test registration because every test within a
 * suite does not actually generate a separate test function.  Unfortunately
 * this precludes reordering tests within a suite, and allocating extremely
 * large or numerous objects on the stack may cause warnings due to excessive
 * stack usage.  However, the effects of this are reduced because every test
 * suite resides in its own test executable.  When using declaring gigantic
 * objects, mark them as `static` to keep them off the stack, or heap allocate
 * them.
 *
 * # Assertion (Precondition) Testing
 *
 * A side benefit of writing my own framework has been the opportunity to
 * leverage Calendon's assertion mechanism, `CN_ASSERT` into testing.  Tests can be
 * written to verify that preconditions catch bad execution conditions with
 * `CN_TEST_PRECONDITION(function_name)`.  This allows tests to provide
 * documentation of function preconditions while also verifying them.
 *
 */
#include <calendon/cn.h>
#include <calendon/float.h>

#include <string.h>

/**
 * An annotation to prevent pollution of the symbol space with local
 * testing functions.
 *
 * @ingroup CalendonTestDetails
 */
#define CN_TEST_HARNESS_API static

/**
 * @addtogroup CalendonTestDetails
 * @{
 */
typedef struct {
	const char* name;
	uint32_t testsPassed, testsFailed;
} cnTestSuiteReport;

typedef struct {
	const char* name;
	uint32_t runsLeft;
	uint32_t assertsFailed;
	bool failureForced;
} cnTestUnitReport;

CN_TEST_HARNESS_API cnTestSuiteReport suiteReport;
CN_TEST_HARNESS_API cnTestUnitReport unitReport;
CN_TEST_HARNESS_API char errorFormatBuffer[4096];

/**
 * A visibly noticable marker to use for when things fail.
 */
static const char* cnTestFailMarker =
	"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";

CN_TEST_HARNESS_API void cnTest_UnitInit(cnTestUnitReport* u, const char* name) {
	if (!u) abort();
	u->name = name;
}

CN_TEST_HARNESS_API void cnTest_UnitStart(cnTestUnitReport* u, const char* name) {
	if (!u) abort();
	u->name = name;
	u->runsLeft = 1;
	u->assertsFailed = 0;
	u->failureForced = false;
	printf("  [ %-6s ] %s\n", "", name);
}

CN_TEST_HARNESS_API void cnTest_UnitAssertFailed(cnTestUnitReport* u) {
	if (!u) abort();
	++u->assertsFailed;
}

CN_TEST_HARNESS_API bool cnTest_UnitSucceeded(cnTestUnitReport* u) {
	if (!u) abort();
	return u->assertsFailed == 0 && !u->failureForced;
}

CN_TEST_HARNESS_API void cnTest_SuiteInit(cnTestSuiteReport* r) {
#if _WIN32
	SetConsoleOutputCP(CP_UTF8);
#endif
	if (!r) abort();
	r->testsPassed = 0;
	r->testsFailed = 0;
}

CN_TEST_HARNESS_API void cnTest_SuiteStart(cnTestSuiteReport* r, const char* name) {
	r->name = name;
	printf("Test suite: %s\n", name);
}

CN_TEST_HARNESS_API void cnTest_SuitePrintResults(cnTestSuiteReport* r) {
	if (!r) abort();
	printf("  [________]\n");
	if (r->testsFailed > 0) {
		printf(cnTestFailMarker);
		printf(">>[%8s]" " %s ( %" PRIu32 " failed )\n",
			"FAILED ", r->name, r->testsFailed);
	}
	else {
		printf("  [%8s]" " %s ( %" PRIu32 " ) \n",
			"PASSED ", r->name, r->testsPassed);
	}
}

CN_TEST_HARNESS_API void cnTest_SuiteShutdown(cnTestSuiteReport* r) {
	if (!r) abort();
	cnTest_SuitePrintResults(&suiteReport);
}

 CN_TEST_HARNESS_API void cnTest_SuiteAddCompletedUnit(cnTestSuiteReport* r, cnTestUnitReport* u) {
	if (!r) abort();
	if (!u) abort();
	if (!u->name) abort();

	if (cnTest_UnitSucceeded(u)) {
		++r->testsPassed;
		printf("  [ %+6s ] %s\n", "PASS", u->name);
	}
	else {
		++r->testsFailed;
		printf(cnTestFailMarker);
		printf(">>[ %-6s ] %s\n",
			"FAILED", u->name);
	}
}

CN_TEST_HARNESS_API void cnTest_CleanUpPreviousUnit(cnTestSuiteReport* r, cnTestUnitReport* u) {
	if (!r) abort();
	if (!u) abort();
	if (u->name) {
		cnTest_SuiteAddCompletedUnit(r, u);
	}
}
/**
 * @}
 */
// group CalendonTestDetails

/**
 * Marks a group of tests to be executed within a source file.  Due to some
 * underlying technical reasons, your test code should be within
 * `CN_TEST_SUITE_BEGIN` and `CN_TEST_SUITE_END`.  You **may** put your code
 * within curly braces, but it is not required.
 *
 * The code between `CN_TEST_SUITE_BEGIN` and `CN_TEST_SUITE_END` gets executed
 * as a continuous block, so you can add code in between your unit tests like
 * in a function.  Functions may not be declared between these labels,
 * but can be declared outside and used within them.
 *
 * Usage:
 * \code
 * // test-foo.c
 * #include <calendon/test.h>
 *
 * // Helper functions declared outside `CN_TEST_SUITE_BEGIN` and `CN_TEST_SUITE_END`
 * void helperFunction1() {}
 * void helperFunction2() {}
 *
 * CN_TEST_SUITE_BEGIN("A short battery of tests")
 *
 * // Code can be put here if desired.
 * printf("Running my test suite\n");
 * helperFunction1();
 *
 * CN_TEST_UNIT("A test") {
 *     helperFunction2();
 * }
 *
 * CN_TEST_UNIT("Another test") {
 * }
 *
 * CN_TEST_SUITE_END
 * \endcode
 *
 * @ingroup CalendonTest
 */
#define CN_TEST_SUITE_BEGIN(name) int main() { \
	cnTest_SuiteInit(&suiteReport); \
	cnTest_UnitInit(&unitReport, NULL); \
	cnTest_SuiteStart(&suiteReport, name);

/**
 * Wraps up testing.  Completes a block of testing in a source file started by
 * `CN_TEST_SUITE_BEGIN`.
 *
 * @ingroup CalendonTest
 */
#define CN_TEST_SUITE_END \
	cnTest_CleanUpPreviousUnit(&suiteReport, &unitReport); \
	cnTest_SuiteShutdown(&suiteReport); \
	return suiteReport.testsFailed > 0 ? EXIT_FAILURE : EXIT_SUCCESS; }

/**
 * Starts a new unit test within a suite.  Treat this sort of like a control
 * structure and put your test within braces (`{}`).
 *
 * Usage:
 * \code
 * CN_TEST_SUITE_BEGIN("My test suite")
 *
 * CN_TEST_UNIT("My test") {
 *   const int32_t x = 10;
 *   CN_TEST_ASSERT_EQ_I32(10, x);
 * }
 *
 * CN_TEST_UNIT("Another test") {
 *   // More assertions.
 * }
 *
 * CN_TEST_UNIT("A third test") {
 *   // More assertions
 * }
 *
 * CN_TEST_SUITE_END
 *
 * \endcode
 *
 * @ingroup CalendonTest
 */
#define CN_TEST_UNIT(name) \
	cnTest_CleanUpPreviousUnit(&suiteReport, &unitReport); \
	cnTest_UnitStart(&unitReport, name); \
    { \
		int assertionStatus = setjmp(cnTest_AssertUnexpectedJumpBuffer); \
		if (assertionStatus == CN_TEST_ASSERTION_UNEXPECTED) { \
			unitReport.runsLeft = 0; \
			unitReport.failureForced = true; \
		} \
    } \
	for (uint32_t cnUnitTestLoop = 0; cnUnitTestLoop < unitReport.runsLeft; ++cnUnitTestLoop)

#ifndef CN_TESTING
	#error "CN_TESTING must be defined for assertion checking."
#endif
#include <calendon/test-asserts.h>

/**
 * Executes a statement, expecting that it triggers a `CN_ASSERT` assertion
 * somewhere in the control flow.  Use this to verify that inputs to a control
 * flow get flagged as illegal.
 *
 * This only covers checks by `CN_ASSERT` and any encountered `CN_FATAL_ERROR`
 * will still cause a program crash.
 *
 * @ingroup CalendonTest
 */
#define CN_TEST_PRECONDITION(stmt) { \
		cnTest_ExpectingAssert = true; \
		int assertionStatus = setjmp(cnTest_AssertJumpBuffer); \
		if (assertionStatus == 0) { stmt; } \
		if (assertionStatus != CN_TEST_ASSERTION_OCCURRED) { \
			cnTest_UnitAssertFailed(&unitReport); \
			printf("%s:%i  Assertion not triggered: " #stmt, \
				__FILE__, __LINE__); \
			break; \
		} \
		cnTest_ExpectingAssert = false; \
	}

/**
 * Prevents problems with duplicate evaluation of test inputs, by creating
 * appropriate functions into which to make the test macros.
 *
 * @ingroup CalendonTestDetail
 */
#define CN_TEST_ASSERT_FN_DEFN(type, formatter) bool cnTest_Assert_##type( \
	type a, type b, const char* left, const char* right, char* file, uint64_t line) \
{ \
	if (a != b) { \
		cnTest_UnitAssertFailed(&unitReport); \
		const int written = sprintf(errorFormatBuffer, "%s:%" PRIu64  "  \"%%%s != %%%s\" (%s != %s)\n", \
			file, line, formatter, formatter, left, right); \
		printf(errorFormatBuffer, a, b); \
		return false; \
	} \
	return true; \
}

CN_TEST_ASSERT_FN_DEFN(size_t, "zu")
CN_TEST_ASSERT_FN_DEFN(int8_t, PRIi8)
CN_TEST_ASSERT_FN_DEFN(int16_t, PRIi16)
CN_TEST_ASSERT_FN_DEFN(int32_t, PRIi32)
CN_TEST_ASSERT_FN_DEFN(int64_t, PRIi64)
CN_TEST_ASSERT_FN_DEFN(uint8_t, PRIu8)
CN_TEST_ASSERT_FN_DEFN(uint16_t, PRIu16)
CN_TEST_ASSERT_FN_DEFN(uint32_t, PRIu32)
CN_TEST_ASSERT_FN_DEFN(uint64_t, PRIu64)

/**
 * @addtogroup CalendonTest
 * @{
 */
#define CN_TEST_ASSERT_EQ_SIZE_T(a, b) if (!cnTest_Assert_size_t(a, b, #a, #b, __FILE__, __LINE__)) break;
#define CN_TEST_ASSERT_EQ_I8(a, b)  if (!cnTest_Assert_int8_t(a, b, #a, #b, __FILE__, __LINE__)) break;
#define CN_TEST_ASSERT_EQ_I16(a, b) if (!cnTest_Assert_int16_t(a, b, #a, #b, __FILE__, __LINE__)) break;
#define CN_TEST_ASSERT_EQ_I32(a, b) if (!cnTest_Assert_int32_t(a, b, #a, #b, __FILE__, __LINE__)) break;
#define CN_TEST_ASSERT_EQ_I64(a, b) if (!cnTest_Assert_int64_t(a, b, #a, #b, __FILE__, __LINE__)) break;
#define CN_TEST_ASSERT_EQ_U8(a, b)  if (!cnTest_Assert_uint8_t(a, b, #a, #b, __FILE__, __LINE__)) break;
#define CN_TEST_ASSERT_EQ_U16(a, b) if (!cnTest_Assert_uint16_t(a, b, #a, #b, __FILE__, __LINE__)) break;
#define CN_TEST_ASSERT_EQ_U32(a, b) if (!cnTest_Assert_uint32_t(a, b, #a, #b, __FILE__, __LINE__)) break;
#define CN_TEST_ASSERT_EQ_U64(a, b) if (!cnTest_Assert_uint64_t(a, b, #a, #b, __FILE__, __LINE__)) break;
/**
 * @}
 */

/**
 * @ingroup CalendonTest
 */
#define CN_TEST_ASSERT_CLOSE_F(a, b, pct) \
	if (cnFloat_RelativeDiff(a, b) > pct) { \
		cnTest_UnitAssertFailed(&unitReport); \
		printf("%s:%i  \"" #a " is not within %f%% of " #b "\" (%f != %f)\n", \
			__FILE__, __LINE__, (10.0f * (float)pct), ((float)(a)), ((float)(b))); \
		break; \
	}

/**
 * Assertion macro for testing null-terminated string equality.
 *
 * @ingroup CalendonTest
 */
#define CN_TEST_ASSERT_EQ_STR(expected, actual) \
	if (strcmp(expected, actual) != 0) { \
		cnTest_UnitAssertFailed(&unitReport); \
		printf("%s:%i  " #expected " != " #actual " (\"%s\" != \"%s\")\n", \
			__FILE__, __LINE__, (expected), (actual)); \
		break; \
	}

/**
 * @ingroup CalendonTest
 */
#define CN_TEST_ASSERT_TRUE(expr) { \
		if (!(expr)) { \
			cnTest_UnitAssertFailed(&unitReport); \
			printf("%s:%i  \"" #expr "\" should be true\n", \
				__FILE__, __LINE__); \
			break; \
		} \
	}

/**
 * @ingroup CalendonTest
 */
#define CN_TEST_ASSERT_FALSE(expr) { \
		if (!!(expr)) { \
			cnTest_UnitAssertFailed(&unitReport); \
			printf("%s:%i  \"" #expr "\" should be false\n", \
				__FILE__, __LINE__); \
			break; \
		} \
	}
