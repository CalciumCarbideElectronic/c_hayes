#include <gtest/gtest.h>

extern "C" {
#include "../src/hayes_parser.h"
}

TEST(test_parser, response_standard) {
    hayes_parser *parser = NewHayesParser(NULL);
    parser_result *res = NewParseResult();
    const char *response = "+CGATT: 0,\"1\",test, 3 ,4\r\n";
    parser->parse_resp(parser,res, response);
    char buf[100];
    res_read_tag(res, buf, response);
    ASSERT_STREQ("CGATT", buf);

    ASSERT_EQ(res_read_nth(res, buf, 0, response), 0);
    ASSERT_STREQ("0", buf);

    ASSERT_EQ(res_read_nth(res, buf, 1, response), 0);
    ASSERT_STREQ("1", buf);

    ASSERT_EQ(res_read_nth(res, buf, 2, response), 0);
    ASSERT_STREQ("test", buf);

    ASSERT_EQ(res_read_nth(res, buf, 3, response), 0);
    ASSERT_STREQ("3", buf);

    ASSERT_EQ(res_read_nth(res, buf, 4, response), 0);
    ASSERT_STREQ("4", buf);

    ASSERT_EQ(res_read_nth(res, buf, 5, response), -1);
    ParseResultFree(res);
}

TEST(test_parser, response_plain) {
    hayes_parser *parser = NewHayesParser(NULL);
    parser_result *res = NewParseResult();
    const char *response = " whatever";
    parser->parse_resp(parser, res, response);
    char buf[100];
    ASSERT_NE(res_read_tag(res, buf, response), 0);

    ASSERT_EQ(res_read_nth(res, buf, 0, response), 0);
    ASSERT_STREQ("whatever", buf);
}

TEST(test_parser, response_type) {
    const char *stdresp0 = "+CGATT: 0,\"1\",test, 3 ,4\r\n";
    const char *ok0 = "OK\r\n";
    const char *empty0 = "\r\n";
    const char *error0 = "+CME ERROR: 1";
    const char *plain0 = "plain";

    hayes_parser *parser = NewHayesParser(NULL);
    parser_result *res = NewParseResult();

    parser->parse_resp(parser, res, stdresp0);
    ASSERT_EQ(res->type, HAYES_RES_STDRESP);
    res_reset(res);

    parser->parse_resp(parser, res, ok0);
    ASSERT_EQ(res->type, HAYES_RES_OK);
    res_reset(res);

    parser->parse_resp(parser, res, empty0);
    ASSERT_EQ(res->type, HAYES_RES_EMPTY);
    res_reset(res);

    parser->parse_resp(parser, res, error0);
    ASSERT_EQ(res->type, HAYES_RES_ERROR);
    res_reset(res);

    parser->parse_resp(parser, res, plain0);
    ASSERT_EQ(res->type, HAYES_RES_RESP);
    res_reset(res);
}
