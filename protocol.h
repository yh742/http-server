//
// Created by Sean Hsu on 9/20/17.
//

#ifndef LISO_SERVER_PROTOCOL_H
#define LISO_SERVER_PROTOCOL_H

#include "http_defs.h"
#include "parse.h"

//HTTP Request Header
typedef struct
{
    char http_version[50];
    char http_status[50];
    char http_reason[50];
    Http_header *headers;
    int header_count;
    void* body;
} Response;

int send_error(int sock_fd, Http_status status);

int select_method(int sock_fd, Request* request);

int check_http_version(const char* ver_string);

void free_response(Response* response);

#endif //LISO_SERVER_PROTOCOL_H
