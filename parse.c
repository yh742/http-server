#include "parse.h"
#include "helper.h"
#include <sys/socket.h>

/**
* Given a char buffer returns the parsed request headers
*/

static void dbg_print_response(Request* request, int sock_fd){
    int index;
    DBG_PRINT("Request Socket %d\n", sock_fd);
    DBG_PRINT("Http Method %s\n",request->http_method);
    DBG_PRINT("Http Version %s\n",request->http_version);
    DBG_PRINT("Http Uri %s\n",request->http_uri);
    DBG_PRINT("Request Header\n");
    for(index = 0;index < request->header_count;index++) {
        DBG_PRINT("Header Name: %s, Header Value: %s\n", request->headers[index].header_name,
                  request->headers[index].header_value);
    }
}

Request * parse(char *buffer, int size, int sock_fd) {
    //Differant states in the state machine
    enum {
        STATE_START = 0, STATE_CR, STATE_CRLF, STATE_CRLFCR, STATE_CRLFCRLF
    };

    int i = 0, state, head_index, length;
    size_t offset = 0;
    char ch;
    char buf[8192];
    char* body;
    memset(buf, 0, 8192);

    state = STATE_START;
    while (state != STATE_CRLFCRLF) {
        char expected = 0;

        if (i == size)
            break;

        ch = buffer[i++];
        buf[offset++] = ch;

        switch (state) {
            case STATE_START:
            case STATE_CRLF:
                expected = '\r';
                break;
            case STATE_CR:
            case STATE_CRLFCR:
                expected = '\n';
                break;
            default:
                state = STATE_START;
                continue;
        }

        if (ch == expected)
            state++;
        else
            state = STATE_START;

    }

    //Valid End State
    if (state == STATE_CRLFCRLF) {
        Request *request = (Request *) malloc(sizeof(Request));
        request->header_count=0;
        request->headers = (Http_header *) malloc(sizeof(Http_header)*1);
        request->body = NULL;
        set_parsing_options(buf, i, request);

        if (yyparse() == SUCCESS) {
            dbg_print_response(&request, sock_fd);
            // parse the body if one exists
            if ((head_index = get_header_value(request->headers, request->header_count, "Content-Length")) != -1){

                length = atoi(request->headers[head_index].header_value);
                DBG_PRINT("Found Content Length: %d", length);
                if (length != 0){
                    char *body = malloc(length);

                    // offset + length is greater than buffer size
                    if (length > size - offset){
                        int read_size = size - offset;
                        if (read_size != 0){
                            // copy rest of the buffer
                            memcpy(body, buffer + offset, read_size);
                            // read rest fromt he socket
                            recv(sock_fd, body + read_size, length - read_size ,0);
                        }
                        else{
                            // read the whole thing from the socket, edge case
                            recv(sock_fd, body, length ,0);
                        }
                    }
                    else{
                        memcpy(body, buffer + offset, length);
                    }
                    DBG_PRINT("Content: %s", body);
                }

            }
            request->body = body;
            return request;
        }
        else {
            DBG_PRINT("yyparse() failed.");
            return NULL;
        }
    }
    return NULL;
}

int get_header_value(const Http_header* start_hdr, int count, const char* name){
    int index;
    for (index = 0; index < count; index++){
        if (!strcmp(start_hdr[index].header_name, name)){
            return index;
        }
    }
    return -1;
}

void free_requests(Request* request) {
    DBG_PRINT("Free up requests enter");
    if (request->body != NULL){
        free(request->body);
    }
    free(request->headers);
    free(request);
    DBG_PRINT("Free up requests exit");
}
