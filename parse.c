#include "parse.h"
#include "helper.h"

/**
* Given a char buffer returns the parsed request headers
*/
Request * parse(char *buffer, int size, int socketFd) {
    //Differant states in the state machine
    enum {
        STATE_START = 0, STATE_CR, STATE_CRLF, STATE_CRLFCR, STATE_CRLFCRLF
    };

    int i = 0, state;
    size_t offset = 0;
    char ch;
    char buf[8192];
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
        //TODO You will need to handle resizing this in parser.y
        request->headers = (Http_header *) malloc(sizeof(Http_header)*1);
        set_parsing_options(buf, i, request);

        if (yyparse() == SUCCESS) {
            int i = get_header_value(request->headers, request->header_count, "Accept");
            DBG_PRINT("Accept Index: %d", i);
            return request;
        }
        else {
            DBG_PRINT("yyparse() failed.");
            return NULL;
        }
    }
    //TODO implement reading message body
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
    free(request->headers);
    free(request);
}
