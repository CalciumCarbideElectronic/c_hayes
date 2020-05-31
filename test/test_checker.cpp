#include <gtest/gtest.h>

extern "C" {
#include "../src/hayes_checker.h"
}

TEST(test_checker, basic) {
    ASSERT_EQ(gDefaultChecker.is_ok("OK"), 0);
    ASSERT_EQ(gDefaultChecker.is_ok("OK\r\n"), 1);

    ASSERT_EQ(gDefaultChecker.is_empty("\r\n"), 1);
    ASSERT_EQ(gDefaultChecker.is_empty(""), 1);
    ASSERT_EQ(gDefaultChecker.is_empty(" "), 0);

    ASSERT_EQ(gDefaultChecker.is_error("+CME ERROR: 12\r\n"), 1);

    ASSERT_EQ(gDefaultChecker.is_response("whatever"), 1);
    ASSERT_EQ(gDefaultChecker.is_response("+CGATT: 1\r\n"), 1);

    ASSERT_EQ(gDefaultChecker.is_stdresp("+CGATT: 1\r\n"), 1);
    ASSERT_EQ(gDefaultChecker.is_stdresp("nothing"), 0);
}
