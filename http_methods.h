//
// Created by Sean Hsu on 9/26/17.
//

#ifndef LISO_SERVER_HTTP_METHODS_H
#define LISO_SERVER_HTTP_METHODS_H
#include "parse.h"
#include "protocol.h"

char* get_mime_type(char* ext);

int do_get(const Request* req, Response* response, int head);

int do_head(const Request* req, Response* response);

int do_post(const Request* req, Response* response);

#endif //LISO_SERVER_HTTP_METHODS_H
