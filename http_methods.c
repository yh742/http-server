//
// Created by Sean Hsu on 9/26/17.
//
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include "helper.h"
#include "http_methods.h"
#include "http_defs.h"

static const char WWW_PATH[] = "./WWW/";
static const char IDX_PATH[] = "index.html";

// return -1 for not exist
// return -2 for internal failures
int do_get(const Request* req, Response* response){
    size_t length = strlen(WWW_PATH) + strlen(req->http_uri);
    FILE* file;
    struct stat sb;

    char* path = malloc(length);
    memset(path, '\0', strlen(path));
    memcpy(path, WWW_PATH, strlen(WWW_PATH));
    memcpy(path + strlen(WWW_PATH), req->http_uri, strlen(req->http_uri));

    if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)){
        // check if it is a folder -> return index.html
        path = realloc(path, length + strlen(IDX_PATH));
        path = memcpy(path + length, IDX_PATH, strlen(IDX_PATH));
        DBG_PRINT("Added index to path: %s", path);
    }

    // check if file exists
    if (access(path, F_OK) == -1){
        free(path);
        DBG_PRINT("Path does not exist.");
        return -1;
    }

    // assign file to response body
    file = fopen(path, "r");
    if (file == NULL){
        free(path);
        DBG_PRINT("Resource errors.");
        return -2;
    }
    response->body = file;

    // get last modified stamp from the file
    stat(path, &sb);
    char date[50];
    strftime(date, strlen(date), "%a, %d %b %Y %X %Z", gmtime(&(sb.st_ctime)));
    strcpy(response->headers[get_idx(LAST_MOD)].header_value, date);

    // get file length
    itoa(response->headers[get_idx(CONTENT_LEN)].header_value, sb.st_size);

    // content type
    strcpy(response->headers[get_idx(CONTENT_TYPE)].header_value, get_mime_type(strrchr(path, '.')));

    return 1;
}

int do_head(const Request* req, Response* response){
    return 1;
}

int do_post(const Request* req, Response* response){
    return 1;
}