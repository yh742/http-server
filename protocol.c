#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "protocol.h"
#include "helper.h"
#include "parse.h"

static const char NEW_LINE[] = "\\r\\n";
static const char HTTP_VER[] = "HTTP/1.1";
static const char SERVER_NAME[] = "Liso/1.0";
// For connection header file
static const char KEEP_ALIVE[] = "keep-alive";
static const char CLOSE[] = "close";

static char* get_mime_type(char* ext) {
    char* dot = strchr(ext, '.');
    if (dot == NULL) {
        return NULL;
    }
    else if (!strncmp(dot, ".html", 5)){
        return "text/html";
    }
    else if (!strncmp(dot, ".css", 4)){
        return "text/css";
    }
    else if (!strncmp(dot, ".png", 4)){
        return "image/png";
    }
    else if (!strncmp(dot, ".jpg", 4)){
        return "image/jpeg";
    }
    else if (!strncmp(dot, ".gif", 4)){
        return "image/gif";
    }
    else {
        return NULL;
    }
}

static char* get_http_status(Http_status status){
    // okay to return string literals, not on stack
    switch (status){
        case OK:
            return "OK";
        case BAD_REQUEST:
            return "Bad Request";
        case NOT_FOUND:
            return "Not Found";
        case LENGTH_REQUIRED:
            return "Length Required";
        case INTERNAL_SERVER_ERROR:
            return "Internal Server Error";
        case NOT_IMPLEMENTED:
            return  "Not Implemented";
        case SERVICE_UNAVAILABLE:
            return "Service Unavailable";
        case HTTP_VERSION_NOT_SUPPORTED:
            return "HTTP Version Not Supported";
        default:
            return "OK";
    }
}

static void setup_general_header(Response* response, const char* code, const char* reason){
    strcpy(response->http_version, HTTP_VER);
    strcpy(response->http_status, code);
    strcpy(response->http_reason, reason);
    response->header_count = 5;
    response->headers = (Http_header*)malloc(response->header_count*sizeof(Http_header));
    strcpy(response->headers[0].header_name, "Server");
    strcpy(response->headers[0].header_value, SERVER_NAME);
    strcpy(response->headers[1].header_name, "Date");
    get_current_time(&response->headers[1].header_value, sizeof(response->headers[1].header_value));
    strcpy(response->headers[2].header_name, "Connection");
    strcpy(response->headers[3].header_name, "Content-Type");
    strcpy(response->headers[4].header_name, "Content-Length");
}

// points to the next position
static char* copy_to_buffer(char* buffer, const char* str, int count, int skip, int line_ending){
    int i;
    char* ptr;
    for (i = 0, ptr = buffer; i < count; ptr++, i++){
        (*ptr) = str[i];
    }
    if (skip){
        (*ptr) = ' ';
        ptr++;
    }
    if (line_ending){
        (*ptr) = '\r';
        ptr++;
        (*ptr) = '\n';
        ptr++;
    }
    return ptr;
}

static int send_response_header(int sock_fd, Response* response){
    char buffer[8192];
    char* p_pos;
    int i, res = -1;
    // fill entire buffer with white spaces
    memset(buffer, '\0', sizeof(buffer));
    p_pos = copy_to_buffer(buffer, response->http_version, strlen(response->http_version), 1, 0);
    p_pos = copy_to_buffer(p_pos, response->http_status, strlen(response->http_status), 1, 0);
    p_pos = copy_to_buffer(p_pos, response->http_reason, strlen(response->http_reason), 0, 1);
    for (i = 0; i < response->header_count; i++){
        p_pos = copy_to_buffer(p_pos, response->headers[i].header_name, strlen(response->headers[i].header_name), 0, 0);
        p_pos = copy_to_buffer(p_pos, ": ", strlen(": "), 0, 0);
        p_pos = copy_to_buffer(p_pos, response->headers[i].header_value, strlen(response->headers[i].header_value), 0, 1);
    }
    (*p_pos) = '\r';
    p_pos++;
    (*p_pos) = '\n';
    DBG_PRINT("Response Header:\n%s", buffer);
    res = send(sock_fd, buffer, strlen(buffer), 0);
    DBG_PRINT("SEND RES: %d", res);
    return res;
}


static void format_error_response(Response* response, const char* code, const char* reason){
    /* In the following form:
     *
    HTTP/1.1 "Status" "Reason"
    Server: Liso/1.0
    Date: Thu, 21 Sep 2017 03:57:55 GMT
    Connection: close
    Content-Type: text/html
    */
    setup_general_header(response, code, reason);
    strcpy(response->headers[2].header_value, CLOSE);
    strcpy(response->headers[3].header_value, get_mime_type(".html"));
    char buf[50];
    itoa(buf, strlen(response->body));
    strcpy(response->headers[4].header_value, buf);
}

static void format_error_body(Response* response, char* body, Http_status status){
    memset(body, 0, strlen(body));
    sprintf(body,
            "<head>\n"
            "<title>Error response</title>\n"
            "</head>\n"
            "<body>\n"
            "<h1>Error response</h1>\n"
            "<p>Error code %d.\n"
            "<p>Message: %s.\n"
            "</body>\n",
            (int)status, get_http_status(status));
    DBG_PRINT("Error Response Body:\n%s", body);
    response->body = body;
}

int send_error(int sock_fd, Http_status status){
    Response response;
    char buf[50];
    char body[8192];
    memset(buf, '\0', sizeof(buf));
    // All status code is 3 letters
    if (itoa(buf, (int)status) != 3){
        DBG_ERROR("Invalid status code detected.");
        return -1;
    }
    format_error_body(&response, body, status);
    // Content-length needs body length first
    format_error_response(&response, buf, get_http_status(status));
    if (send_response_header(sock_fd, &response) < 1){
        DBG_ERROR("Invalid status code detected.");
        return -2;
    }
    if (send(sock_fd, response.body, strlen(response.body), 0) < 1){
        DBG_ERROR("Invalid status code detected.");
        return -3;
    }
    return 1;
}
