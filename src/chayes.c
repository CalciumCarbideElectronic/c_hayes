#include "chayes.h"

#include "compare-string.h"
#include "hash-string.h"
#include "string.h"

control_ctx *NewControlCtx(syscall_shim shim, hayes_checker *checker) {
    static control_ctx singleton;
    static uint8_t init = 0;
    if (init == 0) {
        singleton.shim = shim;

        singleton.urc_hooks = hash_table_new(string_hash, string_equal);

        singleton.resp_queue = queue_new();

        singleton.parser = NewHayesParser();

        if (checker == NULL)
            singleton.checker = &gDefaultChecker;
        else
            singleton.checker = checker;

        init = 1;
    }
    return &singleton;
}

int send_timeout(control_ctx *self, const char *command, uint64_t timeout) {}

void feed(control_ctx *self, const char *buf) {
    static char tag_buf[30];
    if (self->checker->is_empty) {
        ok
    }
    if (self->checker->is_stdresp(buf)) {
        parser_result *res = new_parser_result();
        self->parser->parse_resp(res, buf);
        res_read_tag(res, tag_buf, buf);
        if (string_equal(tag_buf, self->inflight_tag)) {
            queue_push_head(self->resp_queue, (void *)buf);
        } else {
            urc_hook hook =
                hash_table_lookup(self->urc_hooks, (HashTableKey)tag_buf);
            if (hook != NULL) hook(buf);
        }
    }
}

void register_urc_hook(control_ctx *self, const char *urc, urc_hook hook) {
    hash_table_insert(self->urc_hooks, (void *)urc, (void *)hook);
}

void unregister_urc_hook(control_ctx *self, const char *urc) {
    hash_table_remove(self->urc_hooks, (void *)urc);
}

