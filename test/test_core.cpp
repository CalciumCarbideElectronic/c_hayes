
#include <gtest/gtest.h>

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

enum TEST_URC_IOCTL { IOCTL_WRITE, IOCTL_WORK, IOCTL_READ };

static bool test_urc_hook(const char *buf, TEST_URC_IOCTL ctl,
                          const char *cache_str) {
    static bool flag;
    static char cached_cmd[100];
    switch (ctl) {
        case IOCTL_WRITE: {
            strcpy(cached_cmd, cache_str);
            flag = false;
            return false;
        }
        case IOCTL_WORK: {
            if (strcmp(buf, cached_cmd) == 0) flag = true;
            return false;
        }
        case IOCTL_READ:
            return flag;
    }
}

TEST(test_core, test_urc_handle) {
    char resbuf[30];
    struct syscall_shim shim = {.send = send_null};
    control_ctx *ctx = NewControlCtx(shim, NULL);
    bool urc_flag = false;

    test_urc_hook(NULL, IOCTL_WRITE, "+FAKEURC: foo,\"bar\"\r\n");
    auto hook = [](const char *buf) { test_urc_hook(buf, IOCTL_WORK, NULL); };

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

    ASSERT_TRUE(test_urc_hook(NULL, IOCTL_READ, NULL));
}
