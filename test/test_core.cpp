
#include <gtest/gtest.h>
#include "urc_util.hpp"

extern "C" {

#include "../src/chayes.h"
#include "test_shim.h"
}
#include <chrono>
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
    struct syscall_shim shim = {.send = send_sync_urc};
    control_ctx *ctx = NewControlCtx(shim, NULL);
    std::thread th_send_ok(
        [&](control_ctx *hctx) {
            wait_for_urc_semaphore();
            feed(hctx, "OK\r\n");
        },
        ctx);
    parser_result *res = send_timeout(ctx, "AT+CGATT\r\n", 200);
    th_send_ok.join();

    ASSERT_EQ(res == NULL, false);
}

TEST(test_core, test_stdres) {
    char resbuf[30];
    struct syscall_shim shim = {.send = send_sync_urc};
    control_ctx *ctx = NewControlCtx(shim, NULL);

    std::thread th_send_ok(
        [&](control_ctx *hctx) {
            // wait for  send_timeout
            wait_for_urc_semaphore();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            feed(hctx, "+CGATT: 1,\"t\"\r\n");
            feed(hctx, "OK\r\n");
        },
        ctx);
    parser_result *res = send_timeout(ctx, "AT+CGATT\r\n", 1400);
    th_send_ok.join();

    ASSERT_EQ(res->type, HAYES_RES_STDRESP);
    ASSERT_TRUE(res != NULL);
    res_read_nth(res, resbuf, 0);
    ASSERT_STREQ(resbuf, "1");
    res_read_nth(res, resbuf, 1);
    ASSERT_STREQ(resbuf, "t");
}

TEST(test_core, test_urc_handle) {
    char resbuf[30];
    struct syscall_shim shim = {.send = send_sync_urc};
    control_ctx *ctx = NewControlCtx(shim, NULL);

    URCHelper fake_handler(std::string("FAKEURC"));
    auto hook = [](const char *buf) { URCHelper::hook(buf, "FAKEURC"); };

    register_urc_hook(ctx, "FAKEURC", hook);

    std::thread th_send_ok(
        [&](control_ctx *hctx) {
            wait_for_urc_semaphore();
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
    URCHelper::reset();
}

TEST(test_core, test_plain_handle) {
    struct syscall_shim shim = {.send = send_sync_urc};
    control_ctx *ctx = NewControlCtx(shim, NULL);

    URCHelper fake_handler(std::string("plain"));
    auto hook = [](const char *buf) { URCHelper::hook(buf, "plain"); };

    register_urc_hook(ctx, "plain", hook);

    std::thread th_send_ok(
        [&](control_ctx *hctx) {
            wait_for_urc_semaphore();
            feed(hctx, "+FAKEURC: foo,\"bar\"\r\n");
            feed(hctx, "OK\r\n");
            feed(hctx, "PUBREC\r\n");
            feed(hctx, "PUBCOMP\r\n");
        },
        ctx);

    parser_result *res = send_timeout(ctx, "AT\r\n", 200);
    th_send_ok.join();

    ASSERT_EQ(URCHelper::hit_times("plain"), 2);
    URCHelper::reset();
}
