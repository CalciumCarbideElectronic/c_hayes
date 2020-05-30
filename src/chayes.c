#include "chayes.h"

#include "compare-string.h"
#include "hash-string.h"
#include "string.h"

void ctx_init(control_ctx *self, func_send sender, func_delay osDelay,
              func_getline getline, hayes_checker *checker) {
    self->sender = sender;
    self->osDelay = osDelay;
    self->getline = getline;
    self->urc_hooks = hash_table_new(string_hash, string_equal);

    if (checker == NULL) checker = &gDefaultChecker;
}

int send_timeout(control_ctx *self, const char *command, uint64_t timeout) {
    static char line_buf[1024];
    self->sender(command, strlen(command));
    while (self->getline(line_buf, 1024, timeout)) {
        if (self->checker->is_ok(line_buf)) {
            return 1;
        }
        if (self->checker->is_error(line_buf)) {
            return 0;
        }
    }
    return 1;
}

void register_urc_hook(control_ctx *self, const char *urc, urc_hook hook) {
    hash_table_insert(self->urc_hooks, (void *)urc, (void *)hook);
}

void unregister_urc_hook(control_ctx *self, const char *urc) {
    hash_table_remove(self->urc_hooks, (void *)urc);
}

