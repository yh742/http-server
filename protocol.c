#include <sys/socket.h>
#include <sys/sendfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "protocol.h"
#include "helper.h"
#include "http_methods.h"

static const char NEW_LINE[] = "\\r\\n";

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
    response->header_count = 6;
    response->headers = (Http_header*)malloc(response->header_count*sizeof(Http_header));
    strcpy(response->headers[0].header_name, SERVER);
    strcpy(response->headers[0].header_value, SERVER_NAME);
    strcpy(response->headers[1].header_name, DATE);
    get_current_time(&response->headers[1].header_value, sizeof(response->headers[1].header_value));
    strcpy(response->headers[2].header_name, CONNECTION);
    strcpy(response->headers[3].header_name, CONTENT_TYPE);
    strcpy(response->headers[4].header_name, CONTENT_LEN);
    strcpy(response->headers[5].header_name, CONTENT_EN);
    strcpy(response->headers[5].header_value, "gzip");

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
    DBG_PRINT("Response Header:%s", buffer);
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
    strcpy(response->headers[get_idx(CONNECTION)].header_value, CLOSE);
    strcpy(response->headers[get_idx(CONTENT_TYPE)].header_value, get_mime_type(".html"));
    char buf[50];
    itoa(buf, strlen(response->body));
    strcpy(response->headers[get_idx(CONTENT_LEN)].header_value, buf);
}

static void format_error_body(Response* response, char* body, Http_status status){
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
    response->body = (char*)body;
}

int send_error(int sock_fd, Http_status status){
    Response response;
    char buf[50];
    char body[8192];
    memset(buf, '\0', sizeof(buf));
    memset(body, '\0', sizeof(body));
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
    int length = atoi(response.headers[4].header_value);
    if (send(sock_fd, response.body, length, 0) < 1){
        DBG_ERROR("Invalid status code detected.");
        return -3;
    }
    free_response(&response);
    return 1;
}

int check_http_version(const char* ver_string){
    if (strncmp(ver_string, HTTP_VER, strlen(HTTP_VER) + 1) != 0){
        return -1;
    }
    return 0;
}

void dbg_print_response(Response* response){
    int index;
    DBG_PRINT("Http Method %s\n",response->http_status);
    DBG_PRINT("Http Version %s\n",response->http_version);
    DBG_PRINT("Http Uri %s\n",response->http_reason);
    DBG_PRINT("response Header\n");
    for(index = 0;index < response->header_count;index++){
        DBG_PRINT("Header Name: %s, Header Value: %s\n",response->headers[index].header_name,response->headers[index].header_value);
    }
}

int select_method(int sock_fd, Request* request){
    int res;
    Response resp;
    Response* response = &resp;
    response->body = NULL;
    DBG_PRINT("Setup general header");
    setup_general_header(response, "200", get_http_status(OK));
    response->header_count = 8;
    response->headers = realloc(response->headers, response->header_count* sizeof(Http_header));
    strcpy(response->headers[get_idx(CONNECTION)].header_value, KEEP_ALIVE);
    strcpy(response->headers[6].header_name, KEEP_ALIVE);
    // dummy value
    strcpy(response->headers[get_idx(KEEP_ALIVE)].header_value, "timeout=5, max=1000");
    strcpy(response->headers[7].header_name, LAST_MOD);

    // check which method this is
    DBG_PRINT("Finish setting up general header");
    if (!strncmp(request->http_method, "GET", 3)){
        res = do_get(request, response);
    }
    else if (!strncmp(request->http_method, "HEAD", 4)){
        res = do_head(request, response);
    }
    else if (!strncmp(request->http_method, "POST", 4)){
        res = do_post(request, response);
    }
    else{
        send_error(sock_fd, NOT_IMPLEMENTED);
        return 1;
    }

    // something failed when accessing methods
    if (res == 0){
        dbg_print_response(response);
        send_response_header(sock_fd, response);
        if (response-> body != NULL){
            sendfile(sock_fd, fileno(response->body), NULL, atoi(response->headers[get_idx(CONTENT_LEN)].header_value));
        }
    }
    else if (res == -1){
        send_error(sock_fd, NOT_FOUND);
    }
    else if (res == -2){
        send_error(sock_fd, INTERNAL_SERVER_ERROR);
    }
    free_response(response);
    return 1;
}

void free_response(Response* response) {
    free(response->headers);
    //free(response);
}