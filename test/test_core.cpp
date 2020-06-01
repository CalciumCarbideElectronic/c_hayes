
#include <gtest/gtest.h>

extern "C" {

#include "../src/chayes.h"
#include "test_shim.h"
}
#include <thread>

TEST(test_chayes, test_construct) {
    struct syscall_shim shim = {.send = send_stdio};
    control_ctx *ctx = NewControlCtx(shim, NULL);
    //to ensure queue init
    ASSERT_NE(ctx->resp_q,-1);
}

TEST(test_chayes, test_send_timeout) {
    struct syscall_shim shim = {.send = send_stdio};
    control_ctx *ctx = NewControlCtx(shim, NULL);
    parser_result *res = send_timeout(ctx, "Hi", 20);
    ASSERT_EQ(res == NULL, true);
}

TEST(test_core, test_ok) {
    struct syscall_shim shim = {.send = send_stdio};
    control_ctx *ctx = NewControlCtx(shim, NULL);
    std::thread th_send_ok(
        [&](control_ctx *hctx) {
            printf("send OK\r\n");
            feed(hctx, "OK\r\n");
        },
        ctx);
    parser_result *res = send_timeout(ctx, "AT+CGATT\r\n", 200);
    th_send_ok.join();

    ASSERT_EQ(res == NULL, false);
}
