#include "hayes_checker.h"
#include "string.h"
#include "compare-string.h"

uint8_t chayes_is_empty(const char * buf){
    return strlen(buf) == 0 ;
}
uint8_t chayes_is_ok(const char * buf){
    return string_nocase_equal((void *)"OK\r\n",(void *)buf);
}
uint8_t chayes_is_error(const char * buf){
    return strstr(buf,"+CME ERROR:")!=NULL;
}
uint8_t chayes_is_response(const char * buf){
    return  !chayes_is_ok(buf) &&
            !chayes_is_empty(buf) && 
            !chayes_is_error(buf);
}

