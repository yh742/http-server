//
// Created by Sean Hsu on 9/11/17.
//

#ifndef LISO_SERVER_DBG_LOGGER_H
#define LISO_SERVER_DBG_LOGGER_H

#include <errno.h>


#define STR_ERROR() \
    (errno == 0 ? "None" : strerror(errno))
#define ERR_CHECK(A, M, ...) \
    if (!(A)) {DBG_ERROR(M, ##__VA_ARGS__); errno = 0; goto error_exit;}
#define DBG_ERROR(M, ...) \
    dbg_log_print(__FILE__, __LINE__, "[ERROR]: %s. " M, STR_ERROR(), ##__VA_ARGS__)
#define DBG_PRINT(...) \
    dbg_log_print(__FILE__, __LINE__, ##__VA_ARGS__)

// gets current time stamp
void get_current_time(char (*time_str)[], size_t maxsize);

// converts integer to alpha
int itoa(char* buf, int number);

// prints to debug file
void dbg_log_print(char* fname, int lnum, char* fmt, ...);

#endif //LISO_SERVER_DBG_LOGGER_H
