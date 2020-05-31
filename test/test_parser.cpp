#include <gtest/gtest.h>

extern "C" {
#include "../src/hayes_parser.h"
}

TEST(test_parser,response_standard){
    hayes_parser * parser  = NewHayesParser();
    parser_result * res = new_parser_result();
    const char *response = "+CGATT: 0,\"1\",test, 3 ,4\r\n";
    parser->parse_resp(res,response);
    char buf[100];
    res_read_tag(res,buf,response);
    ASSERT_STREQ("CGATT",buf);

    ASSERT_EQ(res_read_nth(res,buf,0,response),0);
    ASSERT_STREQ("0",buf);

    ASSERT_EQ(res_read_nth(res,buf,1,response),0);
    ASSERT_STREQ("1",buf);

    ASSERT_EQ(res_read_nth(res,buf,2,response),0);
    ASSERT_STREQ("test",buf);

    ASSERT_EQ(res_read_nth(res,buf,3,response),0);
    ASSERT_STREQ("3",buf);

    ASSERT_EQ(res_read_nth(res,buf,4,response),0);
    ASSERT_STREQ("4",buf);

    ASSERT_EQ(res_read_nth(res,buf,5,response),-1);
    free_parser_result(res);
}


TEST(test_parser,response_plain){
    hayes_parser * parser  = NewHayesParser();
    parser_result * res = new_parser_result();
    const char *response = " whatever";
    parser->parse_resp(res,response);
    char buf[100];
    ASSERT_NE( res_read_tag(res,buf,response),0);

    ASSERT_EQ(res_read_nth(res,buf,0,response),0);
    ASSERT_STREQ("whatever",buf);
}

