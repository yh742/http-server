//
// Created by Sean Hsu on 9/26/17.
//
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include "helper.h"
#include "http_methods.h"
#include "http_defs.h"
#include "parse.h"

char WWW_PATH[4096] = "./www";
static const char IDX_PATH[] = "index.html";

// return -1 for not exist
// return -2 for internal failures
int do_get(const Request* req, Response* response, int head){
    size_t length = strlen(WWW_PATH) + strlen(req->http_uri);
    FILE* file;
    struct stat sb;

    char path[500];
    memset(path, '\0', sizeof(path));
    memcpy(path, WWW_PATH, strlen(WWW_PATH));
    memcpy(path + strlen(WWW_PATH), req->http_uri, strlen(req->http_uri));

    if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)){
        // check if it is a folder -> return index.html
        memcpy(path + length, IDX_PATH, strlen(IDX_PATH));
        DBG_PRINT("Added index to path: %s", path);
    }

    // check if file exists
    if (access(path, F_OK) == -1){
        DBG_PRINT("Path does not exist: %s", path);
        return -1;
    }

    // head has no body
    if (!head) {
        // assign file to response body
        file = fopen(path, "r");
        if (file == NULL) {
            DBG_PRINT("Resource errors.");
            return -2;
        }
        response->body = file;
    }

    // get last modified stamp from the file
    stat(path, &sb);
    char date[50];
    strftime(date, sizeof(date), "%a, %d %b %Y %X %Z", gmtime(&(sb.st_ctime)));
    strcpy(response->headers[get_idx(LAST_MOD)].header_value, date);

    // get file length
    itoa(response->headers[get_idx(CONTENT_LEN)].header_value, sb.st_size);

    // content type
    strcpy(response->headers[get_idx(CONTENT_TYPE)].header_value, get_mime_type(strrchr(path, '.')));

    DBG_PRINT("Finished setting up GET");
    return 1;
}

int do_head(const Request* req, Response* response){
    return do_get(req, response, 1);
}

int do_post(const Request* req, Response* response){
    if (get_header_value(req->headers, req->header_count, CONTENT_LEN) == -1){
        return -3;
    }
    itoa(response->headers[get_idx(CONTENT_LEN)].header_value, 0);
    return 1;
}
