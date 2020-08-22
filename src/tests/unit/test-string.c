#include <calendon/test.h>

#include <calendon/cn.h>
#include <calendon/string.h>

CN_TEST_SUITE_BEGIN("String")
	CN_TEST_UNIT("TerminatedFitsIn") {
		CN_TEST_PRECONDITION(cnString_FitsWithNull(NULL, 0));
		CN_TEST_PRECONDITION(cnString_FitsWithNull("a", SIZE_MAX));

		CN_TEST_ASSERT_TRUE(cnString_FitsWithNull("abcdefghijklmnopqrstuvwxyz", 27));
		CN_TEST_ASSERT_TRUE(cnString_FitsWithNull("", 1));
	}

	CN_TEST_UNIT("StorageRequired") {
		size_t size;

		CN_TEST_PRECONDITION(cnString_StorageRequired(NULL, 10, &size));
		CN_TEST_PRECONDITION(cnString_StorageRequired("", 0, &size));
		CN_TEST_PRECONDITION(cnString_StorageRequired("a", SIZE_MAX, &size));

		CN_TEST_ASSERT_TRUE(cnString_StorageRequired("", 1, &size));
		CN_TEST_ASSERT_EQ_SIZE_T(size, 1);

		CN_TEST_ASSERT_FALSE(cnString_StorageRequired("a", 1, &size));
		CN_TEST_ASSERT_TRUE(cnString_StorageRequired("a", 2, &size));
		CN_TEST_ASSERT_EQ_SIZE_T(size, 2);

		CN_TEST_ASSERT_TRUE(cnString_StorageRequired("this is a test", 15, &size));
		CN_TEST_ASSERT_EQ_SIZE_T(size, 15);
	}

	CN_TEST_UNIT("NumCharacterBytes") {
		size_t size;

		CN_TEST_PRECONDITION(cnString_NumCharacterBytes(NULL, 10, &size));
		CN_TEST_PRECONDITION(cnString_NumCharacterBytes("a", SIZE_MAX, &size));

		CN_TEST_ASSERT_TRUE(cnString_NumCharacterBytes("", 0, &size));
		CN_TEST_ASSERT_EQ_SIZE_T(size, 0);

		CN_TEST_ASSERT_TRUE(cnString_NumCharacterBytes("", 10, &size));
		CN_TEST_ASSERT_EQ_SIZE_T(size, 0);

		CN_TEST_ASSERT_TRUE(cnString_NumCharacterBytes("12345", 10, &size));
		CN_TEST_ASSERT_EQ_SIZE_T(size, 5);

		CN_TEST_ASSERT_FALSE(cnString_NumCharacterBytes("123456", 4, &size));
	}

	CN_TEST_UNIT("Equal") {
		CN_TEST_PRECONDITION(cnString_Equal(NULL, "right", 256));
		CN_TEST_PRECONDITION(cnString_Equal("left", NULL, 256));
		CN_TEST_PRECONDITION(cnString_Equal("left", "right", 4));

		CN_TEST_ASSERT_TRUE(cnString_Equal("", "", 16));
		CN_TEST_ASSERT_TRUE(cnString_Equal("left", "left", 16));
		CN_TEST_ASSERT_FALSE(cnString_Equal("left", "right", 16));
	}

	CN_TEST_UNIT("HasPrefix") {
		const char* logEnable = "--log-enable";
		const char* tickLimit = "--tick-limit";
		const char* logPrefix = "--log-";
		const char* flagPrefix = "--";
		CN_TEST_PRECONDITION(cnString_HasPrefix(NULL, 64, flagPrefix));
		CN_TEST_PRECONDITION(cnString_HasPrefix(logEnable, SIZE_MAX, flagPrefix));
		CN_TEST_PRECONDITION(cnString_HasPrefix(logEnable, 64, NULL));

		// Check max bytes limits.  This should indicate that the caller has
		// inappropriately sized data.
		CN_TEST_PRECONDITION(cnString_HasPrefix(logEnable, 11, logPrefix));
		CN_TEST_PRECONDITION(cnString_HasPrefix(logEnable, 2, logPrefix));

		CN_TEST_ASSERT_TRUE(cnString_HasPrefix(logEnable, 64, ""));

		CN_TEST_ASSERT_TRUE(cnString_HasPrefix(logEnable, 64, flagPrefix));
		CN_TEST_ASSERT_TRUE(cnString_HasPrefix(logEnable, 64, logPrefix));

		CN_TEST_ASSERT_TRUE(cnString_HasPrefix(tickLimit, 64, flagPrefix));
		CN_TEST_ASSERT_FALSE(cnString_HasPrefix(tickLimit, 64, logPrefix));

		CN_TEST_ASSERT_FALSE(cnString_HasPrefix(logPrefix, 64, logEnable));
	}

	CN_TEST_UNIT("FirstIndexOfChar") {
		const char* systemVerbosity = "main:T";
		size_t index;
		CN_TEST_PRECONDITION(cnString_FirstIndexOfChar(systemVerbosity, 64, ':', NULL));
		CN_TEST_PRECONDITION(cnString_FirstIndexOfChar(NULL, 64, ':', &index));
		CN_TEST_PRECONDITION(cnString_FirstIndexOfChar(systemVerbosity, 5, ':', &index));
		CN_TEST_PRECONDITION(cnString_FirstIndexOfChar(systemVerbosity, SIZE_MAX, ':', &index));

		CN_TEST_ASSERT_TRUE(cnString_FirstIndexOfChar(systemVerbosity, 64, ':', &index));
		CN_TEST_ASSERT_EQ_SIZE_T(4, index);

		CN_TEST_ASSERT_FALSE(cnString_FirstIndexOfChar	("main", 64, ':', &index));
	}

CN_TEST_SUITE_END
