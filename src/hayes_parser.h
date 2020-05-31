#include "stdint.h"
#include "list.h"
#include "stdlib.h"

#ifndef HAYES_PARSET_H
#define HAYES_PARSET_H 

typedef struct range{
    uint32_t inf;
    uint32_t sup;
}range;


typedef struct parser_result{
    uint8_t   available;
    range     tag; 
    ListEntry *resp;
}parser_result;



typedef struct hayes_parser{
    void (*parse_resp)(parser_result* resp,const char * buf);
}hayes_parser;

hayes_parser * NewHayesParser();

parser_result * new_parser_result();
void free_parser_result(parser_result *res);
void default_parse_result(parser_result *resp, const char * buf);

int read_range(char *dst,range _ran,const char *buf);
int res_read_nth(parser_result * res,char* dst,uint32_t n,const char * buf);

int res_read_tag(parser_result * res,char* dst,const char * buf);
#endif 
