#pragma once

#include <knell/kn.h>

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KN_TEST_API static

typedef struct {
	uint32_t testsPassed, testsFailed;
	uint32_t assertsPassed, assertsFailed;
} knTestSuiteReport;

typedef struct {
	const char* currentTestName;
	uint32_t assertsPassed, assertsFailed;
} knTestUnitReport;

KN_TEST_API knTestSuiteReport suiteReport;
KN_TEST_API knTestUnitReport unitReport;

KN_TEST_API void knTest_UnitInit(knTestUnitReport* u, const char* name) {
	if (!u) abort();
	u->currentTestName = name;
	u->assertsPassed = 0;
	u->assertsFailed = 0;
}

KN_TEST_API void knTest_UnitStart(knTestUnitReport* u, const char* name) {
	if (!u) abort();
	u->currentTestName = name;
	u->assertsPassed = 0;
	u->assertsFailed = 0;
	printf("...%s\n", name);
}

KN_TEST_API void knTest_UnitAssertFailed(knTestUnitReport* u) {
	assert(u);
	++u->assertsFailed;
}

KN_TEST_API bool knTest_UnitSucceeded(knTestUnitReport* u) {
	if (!u) abort();
	return u->assertsFailed == 0;
}

KN_TEST_API void knTest_SuiteInit(knTestSuiteReport* r) {
	if (!r) abort();
	r->testsPassed = 0;
	r->testsFailed = 0;
	r->assertsPassed = 0;
	r->assertsFailed = 0;
}

KN_TEST_API void knTest_SuiteStart(knTestSuiteReport* r, const char* name) {
	printf("Test suite: %s...", name);
}

KN_TEST_API void knTest_SuitePrint(knTestSuiteReport* r) {
	if (!r) abort();
	printf("Tests:\n\tPassed: %5" PRIu32 " Failed: %5" PRIu32 "\n",
		r->testsPassed, r->testsFailed);
	printf("Asserts:\n\tPassed: %5" PRIu32 " Failed: %5" PRIu32 "\n",
		r->assertsPassed, r->assertsFailed);
}

KN_TEST_API void knTest_SuiteShutdown(knTestSuiteReport* r) {
	if (!r) abort();
	knTest_SuitePrint(&suiteReport);
}

KN_TEST_API void knTest_SuiteAddUnit(knTestSuiteReport* r, knTestUnitReport* u) {
	if (!r) abort();
	if (!u) abort();
	assert(u->currentTestName);
	r->assertsPassed += u->assertsPassed;
	r->assertsFailed += u->assertsFailed;

	if (knTest_UnitSucceeded(u)) {
		++r->testsPassed;
	}
	else {
		++r->testsFailed;
	}
}

KN_TEST_API void knTest_CleanUpPreviousTest(knTestSuiteReport* r, knTestUnitReport* u) {
	if (!r) abort();
	if (!u) abort();
	if (u->currentTestName) {
		knTest_SuiteAddUnit(r, u);
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
	knTest_CleanUpPreviousTest(&suiteReport, &unitReport); \
	knTest_SuiteShutdown(&suiteReport); \
	return suiteReport.testsFailed > 0 ? EXIT_FAILURE : EXIT_SUCCESS; }

#define KN_TEST_UNIT(name) \
	knTest_CleanUpPreviousTest(&suiteReport, &unitReport); \
	knTest_UnitStart(&unitReport, name); \
	for (uint32_t i = 0; i < 1; ++i)

#ifndef KN_TESTING
	#error "KN_TESTING must be defined for assertion checking."
#endif
#include <knell/kn-assertion-testing.h>

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
			printf("%s:%i  \"" #expr " is false\n", \
				__FILE__, __LINE__); \
			break; \
		} \
	}

#define KN_TEST_ASSERT_FALSE(expr) { \
		if (!!(expr)) { \
			knTest_UnitAssertFailed(&unitReport); \
			printf("%s:%i  \"" #expr " is true\n", \
				__FILE__, __LINE__); \
			break; \
		} \
	}
