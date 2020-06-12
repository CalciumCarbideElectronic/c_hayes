#include "stdint.h"
#ifndef CHAYES_CHECKER_H
#define CHAYES_CHECKER_H

typedef uint8_t(*checkfunc)(const char *);

typedef struct hayes_checker{
    checkfunc is_empty;
    checkfunc is_ok;
    checkfunc is_error;
    checkfunc is_response;
    checkfunc is_stdresp;
}hayes_checker;

uint8_t chayes_is_empty(const char * buf);
uint8_t chayes_is_ok(const char * buf);
uint8_t chayes_is_error(const char * buf);
uint8_t chayes_is_response(const char * buf);
uint8_t chayes_is_stdresp(const char *buf);
void init_default_checker();

extern  hayes_checker gDefaultChecker;






#endif
