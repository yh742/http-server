/******************************************************************************
* echo_server.c                                                               *
*                                                                             *
* Description: This file contains the C source code for an echo server.  The  *
*              server runs on a hard-coded port and simply write back anything*
*              sent to it by connected clients.  It does not support          *
*              concurrent clients.                                            *
*                                                                             *
* Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
*          Wolf Richter <wolf@cs.cmu.edu>                                     *
*                                                                             *
*******************************************************************************/

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "helper.h"
#include "parse.h"
#include "protocol.h"

//#define ECHO_PORT 9999
#define BUF_SIZE 8192

static int ECHO_PORT = 9999;

int close_socket(int sock) {
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

int read_socket(char* buf, int sock_fd){
    int readret, res;
    readret = recv(sock_fd, buf, BUF_SIZE, 0);
    if (readret == 0){
        // if the other side has hung up
        return 1;
    }
    else if (readret < 0){
        // if the error occurs
        return -1;
    }
    Request* request = parse(buf, BUF_SIZE, sock_fd);
    if (request == NULL) {
        DBG_PRINT("Malformed Request");
        // Format of header is not correct
        send_error(sock_fd, BAD_REQUEST);
        // Return 1
        // Should keep serving requests until client hangs up
        //close_socket(sock_fd);
        return 1;
    }
    // check if http version is 1.1
    if (check_http_version(request->http_version) == -1){
        send_error(sock_fd, HTTP_VERSION_NOT_SUPPORTED);
    }
    DBG_PRINT("select methods");
    res = select_method(sock_fd, request);
    //send_error(sock_fd, INTERNAL_SERVER_ERROR);
    // free up requests
    free_requests(request);
    return res;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        return EXIT_FAILURE;
    }
    ECHO_PORT = atoi(argv[1]);
    if (ECHO_PORT == 0){
        return EXIT_FAILURE;
    }
    int sock, client_sock, i, enable;
    ssize_t readret;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;
    char buf[BUF_SIZE];
    fd_set read_fd_set, act_fd_set;

    DBG_PRINT("----- Echo Server -----");
    /* all networked programs must create a socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){
        fprintf(stderr, "SetSockOpt Failed.\n");
        return EXIT_FAILURE;
    }

    DBG_PRINT("Checkpoint %d.", 1);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(ECHO_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    DBG_PRINT("Checkpoint %d.", 2);
    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)))
    {
        close_socket(sock);
        fprintf(stderr, "Failed binding socket.\n");
        return EXIT_FAILURE;
    }

    DBG_PRINT("Checkpoint %d.", 3);
    if (listen(sock, 3))
    {
        close_socket(sock);
        fprintf(stderr, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }

    DBG_PRINT("Checkpoint %d.", 4);
    FD_ZERO(&act_fd_set);
    FD_SET(sock, &act_fd_set);
    while (1) {
        // set read_fd_set to active file descriptor set
        read_fd_set = act_fd_set;
        //DBG_PRINT("Checkpoint %d.", 5);
        ERR_CHECK(select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL), "Select() failed.");

        //DBG_PRINT("Checkpoint %d.", 6);
        // check all file descriptor to see if any of them are set
        for (i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &read_fd_set)){
                //DBG_PRINT("Checkpoint %d.", 7);
                if (i == sock) {
                    // connection requested on original socket
                    cli_size = sizeof(cli_addr);
                    //DBG_PRINT("Checkpoint %d.", 8);
                    ERR_CHECK(client_sock = accept(i, (struct sockaddr *) &cli_addr, &cli_size),
                              "Cannot accept incoming client connection.");
                    DBG_PRINT("Connected from host: %s, port: %d.",
                              inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
                    FD_SET(client_sock, &act_fd_set);
                }
                else {
                    readret = read_socket(buf, i);
                    if (readret == 0) {
                        close_socket(i);
                        FD_CLR(i, &act_fd_set);
                    }
                    else if (readret < 0) {
                        goto error_exit;
                    }
                }
            }
        }
    }

error_exit:
    close_socket(sock);

    return EXIT_SUCCESS;
}
