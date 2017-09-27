//
// Created by Sean Hsu on 9/20/17.
//

#ifndef LISO_SERVER_PARSE_H
#define LISO_SERVER_PARSE_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define SUCCESS 0

//Header field
typedef struct
{
    char header_name[4096];
    char header_value[4096];
} Http_header;

//HTTP Request Header
typedef struct
{
    char http_version[50];
    char http_method[50];
    char http_uri[4096];
    Http_header* headers;
    int header_count;
    char* body;
} Request;

Request* parse(char *buffer, int size, int sock_fd);

void free_requests(Request* req);

// returns index where header is found
int get_header_value(const Http_header* start_hdr, int count, const char* name);
#endif //LISO_SERVER_PARSE_H
