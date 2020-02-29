#pragma once

#include <knell/kn.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Prevent pollution of the symbol space with local testing functions.
 */
#define KN_TEST_HARNESS_API static

typedef struct {
	const char* name;
	uint32_t testsPassed, testsFailed;
} knTestSuiteReport;

typedef struct {
	const char* name;
	uint32_t runsLeft;
	uint32_t assertsFailed;
	bool failureForced;
} knTestUnitReport;

KN_TEST_HARNESS_API knTestSuiteReport suiteReport;
KN_TEST_HARNESS_API knTestUnitReport unitReport;

/**
 * A visibly noticable marker to use for when things fail.
 */
static const char* knTestFailMarker =
	"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";

KN_TEST_HARNESS_API void knTest_UnitInit(knTestUnitReport* u, const char* name) {
	if (!u) abort();
	u->name = name;
}

KN_TEST_HARNESS_API void knTest_UnitStart(knTestUnitReport* u, const char* name) {
	if (!u) abort();
	u->name = name;
	u->runsLeft = 1;
	u->assertsFailed = 0;
	u->failureForced = false;
	printf("  [ %-6s ] %s\n", "", name);
}

KN_TEST_HARNESS_API void knTest_UnitAssertFailed(knTestUnitReport* u) {
	if (!u) abort();
	++u->assertsFailed;
}

KN_TEST_HARNESS_API bool knTest_UnitSucceeded(knTestUnitReport* u) {
	if (!u) abort();
	return u->assertsFailed == 0 && !u->failureForced;
}

KN_TEST_HARNESS_API void knTest_SuiteInit(knTestSuiteReport* r) {
	if (!r) abort();
	r->testsPassed = 0;
	r->testsFailed = 0;
}

KN_TEST_HARNESS_API void knTest_SuiteStart(knTestSuiteReport* r, const char* name) {
	r->name = name;
	printf("Test suite: %s\n", name);
}

KN_TEST_HARNESS_API void knTest_SuitePrintResults(knTestSuiteReport* r) {
	if (!r) abort();
	printf("  [________]\n");
	if (r->testsFailed > 0) {
		printf(knTestFailMarker);
		printf(">>[%8s]" " %s ( %" PRIu32 " failed )\n",
			"FAILED ", r->name, r->testsFailed);
	}
	else {
		printf("  [%8s]" " %s ( %" PRIu32 " ) \n",
			"PASSED ", r->name, r->testsPassed);
	}
}

KN_TEST_HARNESS_API void knTest_SuiteShutdown(knTestSuiteReport* r) {
	if (!r) abort();
	knTest_SuitePrintResults(&suiteReport);
}

KN_TEST_HARNESS_API void knTest_SuiteAddCompletedUnit(knTestSuiteReport* r, knTestUnitReport* u) {
	if (!r) abort();
	if (!u) abort();
	if (!u->name) abort();

	if (knTest_UnitSucceeded(u)) {
		++r->testsPassed;
		printf("  [ %+6s ] %s\n", "PASS", u->name);
	}
	else {
		++r->testsFailed;
		printf(knTestFailMarker);
		printf(">>[ %-6s ] %s\n",
			"FAILED", u->name);
	}
}

KN_TEST_HARNESS_API void knTest_CleanUpPreviousUnit(knTestSuiteReport* r, knTestUnitReport* u) {
	if (!r) abort();
	if (!u) abort();
	if (u->name) {
		knTest_SuiteAddCompletedUnit(r, u);
	}
}

#if _WIN32
#define KN_TEST_SUITE_BEGIN(name) int main() { \
	SetConsoleOutputCP(CP_UTF8); \
	knTest_SuiteInit(&suiteReport); \
	knTest_UnitInit(&unitReport, NULL); \
	knTest_SuiteStart(&suiteReport, name);
#else
#define KN_TEST_SUITE_BEGIN(name) int main() { \
	knTest_SuiteInit(&suiteReport); \
	knTest_UnitInit(&unitReport, NULL); \
	knTest_SuiteStart(&suiteReport, name);
#endif

#define KN_TEST_SUITE_END \
	knTest_CleanUpPreviousUnit(&suiteReport, &unitReport); \
	knTest_SuiteShutdown(&suiteReport); \
	return suiteReport.testsFailed > 0 ? EXIT_FAILURE : EXIT_SUCCESS; }

#define KN_TEST_UNIT(name) \
	knTest_CleanUpPreviousUnit(&suiteReport, &unitReport); \
	knTest_UnitStart(&unitReport, name); \
    { \
		int assertionStatus = setjmp(knTest_AssertUnexpectedJumpBuffer); \
		if (assertionStatus == KN_TEST_ASSERTION_UNEXPECTED) { \
			unitReport.runsLeft = 0; \
			unitReport.failureForced = true; \
		} \
    } \
	for (uint32_t knUnitTestLoop = 0; knUnitTestLoop < unitReport.runsLeft; ++knUnitTestLoop)

#ifndef KN_TESTING
	#error "KN_TESTING must be defined for assertion checking."
#endif
#include <knell/test-asserts.h>

#define KN_TEST_PRECONDITION(fn) { \
		knTest_ExpectingAssert = true; \
		int assertionStatus = setjmp(knTest_AssertJumpBuffer); \
		if (assertionStatus == 0) { fn; }; \
		if (assertionStatus != KN_TEST_ASSERTION_OCCURRED) { \
			knTest_UnitAssertFailed(&unitReport); \
			printf("%s:%i  Assertion not triggered: " #fn, \
				__FILE__, __LINE__); \
			break; \
		} \
		knTest_ExpectingAssert = false; \
	}


#define KN_TEST_ASSERT_EQ_I8(a, b) KN_TEST_ASSERT_EQ_GENERIC(a, b, PRIi8)
#define KN_TEST_ASSERT_EQ_I16(a, b) KN_TEST_ASSERT_EQ_GENERIC(a, b, PRIi16)
#define KN_TEST_ASSERT_EQ_I32(a, b) KN_TEST_ASSERT_EQ_GENERIC(a, b, PRIi32)
#define KN_TEST_ASSERT_EQ_I64(a, b) KN_TEST_ASSERT_EQ_GENERIC(a, b, PRIi64)
#define KN_TEST_ASSERT_EQ_U8(a, b) KN_TEST_ASSERT_EQ_GENERIC(a, b, PRIu8)
#define KN_TEST_ASSERT_EQ_U16(a, b) KN_TEST_ASSERT_EQ_GENERIC(a, b, PRIu16)
#define KN_TEST_ASSERT_EQ_U32(a, b) KN_TEST_ASSERT_EQ_GENERIC(a, b, PRIu32)
#define KN_TEST_ASSERT_EQ_U64(a, b) KN_TEST_ASSERT_EQ_GENERIC(a, b, PRIu64)

#define KN_TEST_ASSERT_EQ_GENERIC(a, b, formatter) \
	if ((a) != (b)) { \
		knTest_UnitAssertFailed(&unitReport); \
		printf("%s:%i  \"" #a " != " #b "\" (%" formatter " != %" formatter ")\n", \
			__FILE__, __LINE__, (a), (b)); \
		break; \
	}

/**
 * Assertion macro for testing null-terminated string equality.
 */
#define KN_TEST_ASSERT_EQ_STR(expected, actual) \
	if (strcmp(expected, actual) != 0) { \
		knTest_UnitAssertFailed(&unitReport); \
		printf("%s:%i  " #expected " != " #actual " (\"%s\" != \"%s\")\n", \
			__FILE__, __LINE__, (expected), (actual)); \
		break; \
	}

#define KN_TEST_ASSERT_TRUE(expr) { \
		if (!(expr)) { \
			knTest_UnitAssertFailed(&unitReport); \
			printf("%s:%i  \"" #expr "\" should be true\n", \
				__FILE__, __LINE__); \
			break; \
		} \
	}

#define KN_TEST_ASSERT_FALSE(expr) { \
		if (!!(expr)) { \
			knTest_UnitAssertFailed(&unitReport); \
			printf("%s:%i  \"" #expr "\" should be false\n", \
				__FILE__, __LINE__); \
			break; \
		} \
	}
