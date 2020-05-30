#include "hayes_parser.h"
#include "string.h"

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
                    res->tag.inf = -1;
                    res->tag.sup = -1;
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
                if( ch ==' ') break;
                range *new_range = malloc(sizeof(range));
                new_range->inf = cursor;
                list_append(&res->resp, new_range);
                fsm = 4;
                break;
            }
            case 4:
                if (ch == ',') {
                    range *last = list_data(
                        list_nth_entry(res->resp, list_length(res->resp)));
                    last->sup = cursor;
                    fsm = 3;
                }
        }
        cursor++;
    }
    range *last = list_data(list_nth_entry(res->resp, list_length(res->resp)));
    last->sup = cursor;
}

