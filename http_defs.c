//
// Created by Sean Hsu on 9/28/17.
//
#include <string.h>

char* get_mime_type(char* ext) {
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
        return "application/octet-stream";
    }
}