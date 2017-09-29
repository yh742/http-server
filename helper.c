//
// Created by Sean Hsu on 9/11/17.
//
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include "helper.h"

//#define LOG_FNAME "dbg.txt"

char LOG_FNAME[4096];
static int access_count = 0;

void get_current_time(char (*time_str)[], size_t maxsize) {

    time_t t;
    struct tm* timeinfo;

    time(&t);
    timeinfo = gmtime(&t);
    strftime(*time_str, maxsize, "%a, %d %b %Y %X %Z", timeinfo);
}

int itoa(char* buf, int number){
    return sprintf(buf, "%d", number);
}

void dbg_log_print(char* fname, int lnum, char* fmt, ...) {

    FILE* log_fd;
    char time_str[255];
    char* pfmt;
    char* sparam;
    int iparam;
    va_list list;

    if (access_count == 0) {
        log_fd = fopen(LOG_FNAME, "w");
    }
    else {
        log_fd = fopen(LOG_FNAME, "a+");
    }

    get_current_time(&time_str, sizeof(time_str));
    fprintf(log_fd, "%s %s %d>", time_str, fname, lnum );

    va_start(list, fmt);
    for (pfmt = fmt; *pfmt != '\0'; pfmt++) {
        if (*pfmt != '%') {
            fputc(*pfmt, log_fd);
        }
        else {
            switch (*(++pfmt)) {

                case 's':
                    sparam = va_arg(list, char *);
                    fprintf(log_fd, "%s", sparam);
                    break;
                case 'd':
                    iparam = va_arg(list, int);
                    fprintf(log_fd, "%d", iparam);
                    break;
                default:
                    fputc(*pfmt, log_fd);
            }
        }
    }
    va_end(list);

    fputc('\n', log_fd);
    access_count++;
    fclose(log_fd);
}


