#include "hayes_checker.h"
#include "list.h"
#include "stdint.h"
#include "stdlib.h"

#ifndef HAYES_PARSET_H
#define HAYES_PARSET_H

typedef struct range {
    uint32_t inf;
    uint32_t sup;
} range;

enum result_type {
    HAYES_RES_EMPTY,
    HAYES_RES_OK,
    HAYES_RES_ERROR,
    HAYES_RES_STDRESP,
    HAYES_RES_RESP
};

typedef struct parser_result {
    enum result_type type;
    range tag;
    ListEntry *resp;
} parser_result;

typedef struct hayes_parser {
    hayes_checker *checker;
    void (*parse_resp)(struct hayes_parser *self, parser_result *resp,
                       const char *buf);

} hayes_parser;

hayes_parser *NewHayesParser(hayes_checker *checker);
parser_result *NewParseResult();

void res_reset(parser_result *self);
void ParseResultFree(parser_result *res);
void default_parse_result(hayes_parser *self, parser_result *resp,
                          const char *buf);

int read_range(char *dst, range _ran, const char *buf);
int res_read_nth(parser_result *res, char *dst, uint32_t n, const char *buf);

int res_read_tag(parser_result *res, char *dst, const char *buf);
#endif
