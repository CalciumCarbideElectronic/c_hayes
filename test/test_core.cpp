
#include <gtest/gtest.h>
#include "urc_util.hpp"

extern "C" {

#include "../src/chayes.h"
#include "test_shim.h"
}
#include <functional>
#include <thread>
#include "mqueue.h"

TEST(test_chayes, test_construct) {
    struct syscall_shim shim = {.send = send_null};
    control_ctx *ctx = NewControlCtx(shim, NULL);
    // to ensure queue init
    ASSERT_NE(ctx->resp_q, -1);
}

TEST(test_chayes, test_send_timeout) {
    struct syscall_shim shim = {.send = send_null};
    control_ctx *ctx = NewControlCtx(shim, NULL);
    parser_result *res = send_timeout(ctx, "Hi", 20);
    ASSERT_EQ(res == NULL, true);
    mq_unlink(CHAYES_QUEUE_NAME);
}

TEST(test_core, test_ok) {
    struct syscall_shim shim = {.send = send_null};
    control_ctx *ctx = NewControlCtx(shim, NULL);
    std::thread th_send_ok([&](control_ctx *hctx) { feed(hctx, "OK\r\n"); },
                           ctx);
    parser_result *res = send_timeout(ctx, "AT+CGATT\r\n", 200);
    th_send_ok.join();

    ASSERT_EQ(res == NULL, false);
}

TEST(test_core, test_stdres) {
    char resbuf[30];
    struct syscall_shim shim = {.send = send_null};
    control_ctx *ctx = NewControlCtx(shim, NULL);

    std::thread th_send_ok(
        [&](control_ctx *hctx) {
            feed(hctx, "+CGATT: 1,\"t\"\r\n");
            feed(hctx, "OK\r\n");
        },
        ctx);

    parser_result *res = send_timeout(ctx, "AT+CGATT\r\n", 200);
    th_send_ok.join();

    ASSERT_TRUE(res != NULL);
    res_read_nth(res, resbuf, 0);
    ASSERT_STREQ(resbuf, "1");
    res_read_nth(res, resbuf, 1);
    ASSERT_STREQ(resbuf, "t");
}

TEST(test_core, test_urc_handle) {
    char resbuf[30];
    struct syscall_shim shim = {.send = send_null};
    control_ctx *ctx = NewControlCtx(shim, NULL);
    bool urc_flag = false;

    URCHelper fake_handler(std::string("FAKEURC"));
    auto hook = [](const char *buf) { URCHelper::hook(buf, "FAKEURC"); };

    register_urc_hook(ctx, "FAKEURC", hook);

    std::thread th_send_ok(
        [&](control_ctx *hctx) {
            feed(hctx, "+CGATT: 1,\"t\"\r\n");
            feed(hctx, "+FAKEURC: foo,\"bar\"\r\n");
            feed(hctx, "OK\r\n");
        },
        ctx);

    parser_result *res = send_timeout(ctx, "AT+CGATT\r\n", 200);
    th_send_ok.join();

    ASSERT_TRUE(res != NULL);
    res_read_nth(res, resbuf, 0);
    ASSERT_STREQ(resbuf, "1");
    res_read_nth(res, resbuf, 1);
    ASSERT_STREQ(resbuf, "t");

    ASSERT_TRUE(URCHelper::hit_times("FAKEURC") > 0);
}

TEST(test_core, test_plain_handle) {
    char resbuf[30];
    struct syscall_shim shim = {.send = send_null};
    control_ctx *ctx = NewControlCtx(shim, NULL);
    bool urc_flag = false;

    URCHelper fake_handler(std::string("plain"));
    auto hook = [](const char *buf) { URCHelper::hook(buf, "plain"); };

    register_urc_hook(ctx, "plain", hook);

    std::thread th_send_ok(
        [&](control_ctx *hctx) {
            feed(hctx, "+FAKEURC: foo,\"bar\"\r\n");
            feed(hctx, "PUBREC\r\n");
            feed(hctx, "PUBCOMP\r\n");
        },
        ctx);

    parser_result *res = send_timeout(ctx, "AT\r\n", 200);
    th_send_ok.join();
    auto obj = URCHelper::get_obj("plain");
    printf("dd obj hit time: %ul\n", obj->mHitTimes);
    while (!obj->received_stack.empty()) {
        auto i = obj->received_stack.top();
        printf("\t\t recv: %s", i.c_str());
        obj->received_stack.pop();
    }

    ASSERT_EQ(URCHelper::hit_times("plain"), 2);
}
