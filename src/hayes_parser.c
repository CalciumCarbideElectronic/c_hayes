#include "hayes_parser.h"

#include "stdlib.h"
#include "string.h"

hayes_parser *NewHayesParser() {
    hayes_parser *res = (hayes_parser *)malloc(sizeof(hayes_parser));

    res->parse_resp = default_parse_result;
    return res;
}

void free_parser_result(parser_result *res) {
    ListIterator *iter;
    list_iterate(&res->resp, iter);
    while (list_iter_has_more(iter)) {
        ListValue *next = list_iter_next(iter);
        free((range *)next);
    }
    list_free(res->resp);
}

parser_result *new_parser_result() {
    parser_result *res = malloc(sizeof(parser_result));
    return res;
}

void default_parse_result(parser_result *res, const char *buf) {
    uint16_t fsm = 0;
    char ch;
    uint32_t cursor = 0;

    while (buf[cursor] != '\0') {
        ch = buf[cursor];
        switch (fsm) {
            case 0:
                if (ch == '+') {
                    fsm = 1;
                    break;
                } else {
                    res->tag.inf = 0;
                    res->tag.sup = 0;
                    range *new_range = malloc(sizeof(range));
                    new_range->inf = cursor;
                    list_append(&res->resp, new_range);
                    fsm = 4;
                }
            case 1:
                res->tag.inf = cursor;
                fsm = 2;
                break;
            case 2:
                if (ch == ':') {
                    res->tag.sup = cursor;
                    fsm = 3;
                }
                break;
            case 3: {
                if (ch == ' ') break;
                range *new_range = malloc(sizeof(range));
                new_range->inf = cursor;
                list_append(&res->resp, new_range);
                fsm = 4;
                break;
            }
            case 4:
                if (ch == ',' || ch == '\r') {
                    uint16_t len = list_length(res->resp);
                    range *last = list_data(list_nth_entry(res->resp, len - 1));
                    last->sup = cursor;
                    fsm = 3;
                }
                if (ch == '\r') fsm = 5;
        }
        cursor++;
    }
    uint16_t len = list_length(res->resp);

    range *last = list_data(list_nth_entry(res->resp, len - 1));
    last->sup = cursor;
}

int read_range(char *dst, range _ran, const char *buf) {
    if (_ran.inf == _ran.sup) return -1;

    while (buf[_ran.inf] == ' ' || buf[_ran.inf] == '"') _ran.inf++;
    while (buf[_ran.sup - 1] == ' ' || buf[_ran.sup - 1] == '"' ||
           buf[_ran.sup - 1] == '\r' || buf[_ran.sup - 1] == '\n')
        _ran.sup--;
    memcpy(dst, buf + _ran.inf, _ran.sup - _ran.inf);
    dst[_ran.sup - _ran.inf] = '\0';
    return 0;
}

int res_read_nth(parser_result *res, char *dst, uint32_t n, const char *buf) {
    if (n >= list_length(res->resp)) return -1;

    range *data = (range *)list_nth_data(res->resp, n);
    return read_range(dst, *data, buf);
}

int res_read_tag(parser_result *res, char *dst, const char *buf) {
    return read_range(dst, res->tag, buf);
}
