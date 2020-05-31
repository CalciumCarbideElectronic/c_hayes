#include "hayes_checker.h"

#include "compare-string.h"
#include "string.h"

hayes_checker gDefaultChecker = {.is_empty = chayes_is_empty,
                                 .is_ok = chayes_is_ok,
                                 .is_error = chayes_is_error,
                                 .is_response = chayes_is_response,
                                 .is_stdresp = chayes_is_stdresp};

uint8_t chayes_is_empty(const char *buf) {
    return strlen(buf) == 0 || string_equal((void *)buf, "\r\n");
}
uint8_t chayes_is_ok(const char *buf) {
    return string_nocase_equal((void *)"OK\r\n", (void *)buf);
}
uint8_t chayes_is_error(const char *buf) {
    return strstr(buf, "+CME ERROR:") != NULL;
}

uint8_t chayes_is_stdresp(const char *buf) {
    int fsm = 0;
    char ch;
    uint32_t cursor = 0;
    while (buf[cursor] != 0) {
        ch = buf[cursor];
        cursor++;
        switch (fsm) {
            case -1:
                return 0;
            case 0:
                fsm = (ch == '+' ? 1 : -1);
                break;
            case 1:
                fsm = (ch == ':' ? 2 : 1);
                break;
            case 2:
                return 1;
                break;
        }
    }
    return 0;
}

uint8_t chayes_is_response(const char *buf) {
    return !chayes_is_ok(buf) && !chayes_is_empty(buf) && !chayes_is_error(buf);
}

