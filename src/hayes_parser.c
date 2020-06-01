#include "hayes_parser.h"

#include "stdlib.h"
#include "string.h"

hayes_parser *NewHayesParser(hayes_checker *checker) {
    hayes_parser *res = (hayes_parser *)malloc(sizeof(hayes_parser));
    if (checker)
        res->checker = checker;
    else
        res->checker = &gDefaultChecker;

    res->parse_resp = default_parse_result;
    res->parse_at_req = default_parse_at_req;
    return res;
}

void _free_malloced_list(ListEntry *list) {
    ListIterator iter;
    list_iterate(&list, &iter);
    while (list_iter_has_more(&iter)) {
        ListValue *next = list_iter_next(&iter);
        free((range *)next);
    }

    list_free(list);
}

void ParseResultFree(parser_result *res) {
    _free_malloced_list(res->resp);
    free(res);
}

parser_result *NewParseResult() {
    parser_result *res = malloc(sizeof(parser_result));
    return res;
}

void res_reset(parser_result *self) {
    _free_malloced_list(self->resp);
    self->resp = NULL;
}

void default_parse_at_req(hayes_parser *self, parser_result *res,
                          const char *buf) {
    res->type = HAYES_REQ;
    res->tag.inf = 0;
    res->tag.sup = 0;
    uint32_t cursor = 0;
    int fsm = 0;
    while (buf[cursor] != 0) {
        char ch = buf[cursor];
        switch (fsm) {
            case -1:
                return;
            case 0: {
                fsm = (ch == 'A' || ch == 'a') ? 1 : -1;
                break;
            }
            case 1: {
                fsm = (ch == 'T' || ch == 't') ? 2 : -1;
                break;
            }
            case 2: {
                fsm = (ch == '+') ? 3 : -1;
                break;
            }
            case 3: {
                res->tag.inf = cursor;
                fsm = 4;
                break;
            }
            case 4: {
                if (ch == ':' || ch == '?' || ch == '=') {
                    res->tag.sup = cursor;
                    return;
                }
                break;
            }
        }
        cursor++;
    }
    res->tag.sup = cursor;
}

void default_parse_result(hayes_parser *self, parser_result *res,
                          const char *buf) {
    if (self->checker->is_empty(buf)) {
        res->type = HAYES_RES_EMPTY;
        return;
    }
    if (self->checker->is_ok(buf)) {
        res->type = HAYES_RES_OK;
        return;
    }
    if (self->checker->is_error(buf)) {
        res->type = HAYES_RES_ERROR;
        return;
    }

    uint16_t fsm = 0;
    char ch;
    uint32_t cursor = 0;
    res->type = HAYES_RES_RESP;

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
                    res->type = HAYES_RES_STDRESP;
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
            case 4: {
                range *last =
                    list_nth_data(res->resp, list_length(res->resp) - 1);
                if (ch == ',' || ch == '\r') {
                    last->sup = cursor;
                    fsm = 3;
                }
                if (ch == '\r') fsm = 5;
                break;
            }
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
