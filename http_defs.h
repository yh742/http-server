//
// Created by Sean Hsu on 9/28/17.
//

#ifndef LISO_SERVER_HTTP_DEFINITIONS_H
#define LISO_SERVER_HTTP_DEFINITIONS_H

#define get_idx(X) get_header_value(response->headers, response->header_count, X)

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

// For connection header file
static const char HTTP_VER[] = "HTTP/1.1";
static const char SERVER_NAME[] = "Liso/1.0";
// -----------------------------------------
static const char KEEP_ALIVE[] = "Keep-Alive";
static const char CLOSE[] = "Close";
static const char CONNECTION[] = "Connection";
static const char CONTENT_TYPE[] = "Content-Type";
static const char CONTENT_LEN[] = "Content-Length";
static const char CONTENT_EN[] = "Content-Encoding";
static const char LAST_MOD[] = "Last-Modified";
static const char SERVER[] = "Server";
static const char DATE[] = "Date";

char* get_mime_type(char* ext);

#endif //LISO_SERVER_HTTP_DEFINITIONS_H
