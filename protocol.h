//
// Created by Sean Hsu on 9/20/17.
//

#ifndef LISO_SERVER_PROTOCOL_H
#define LISO_SERVER_PROTOCOL_H

#include "parse.h"

typedef enum
{
    OK = 200,
    BAD_REQUEST =400,
    NOT_FOUND = 404,
    LENGTH_REQUIRED = 411,
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    SERVICE_UNAVAILABLE = 503,
    HTTP_VERSION_NOT_SUPPORTED = 505
} Http_status;

//HTTP Request Header
typedef struct
{
    char http_version[50];
    char http_status[50];
    char http_reason[50];
    Http_header *headers;
    int header_count;
    char* body;
} Response;

int send_error(int sock_fd, Http_status status);

int select_method(int sock_fd, Request* request);

int check_http_version(const char* ver_string);

void free_response(Response* response);

#endif //LISO_SERVER_PROTOCOL_H
