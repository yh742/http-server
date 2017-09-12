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
#include "dbg_logger.h"


#define ECHO_PORT 9999
#define BUF_SIZE 4096

int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    int sock, client_sock, i;
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
        DBG_PRINT("Checkpoint %d.", 5);
        ERR_CHECK(select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL), "Select() failed.");

        DBG_PRINT("Checkpoint %d.", 6);
        // check all file descriptor to see if any of them are set
        for (i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &read_fd_set)){
                DBG_PRINT("Checkpoint %d.", 7);
                if (i == sock) {
                    // connection requested on original socket
                    cli_size = sizeof(cli_addr);
                    DBG_PRINT("Checkpoint %d.", 8);
                    ERR_CHECK(client_sock = accept(i, (struct sockaddr *) &cli_addr, &cli_size),
                              "Cannot accept incoming client connection.");
                    DBG_PRINT("Connected from host: %s, port: %d.",
                              inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
                    FD_SET(client_sock, &act_fd_set);
                }
                else {
                    // existing connection requests
                    if ((readret = recv(i, buf, BUF_SIZE, 0)) >= 1)
                    {
                        int sentret =  send(i, buf, readret, 0);
                        DBG_PRINT("Checkpoint %d. %d vs %d", 8, readret, sentret);
                        ERR_CHECK(sentret == readret, "Error sending to client %d vs. %d.", readret, sentret);
                        memset(buf, 0, BUF_SIZE);
                    }
                }
            }
        }
    }


    /* finally, loop waiting for input and then write it back */
    /*
    while (1)
    {
        cli_size = sizeof(cli_addr);
        if ((client_sock = accept(sock, (struct sockaddr *) &cli_addr,
                                  &cli_size)) == -1)
        {
            close(sock);
            fprintf(stderr, "Error accepting connection.\n");
            return EXIT_FAILURE;
        }

        readret = 0;

        while((readret = recv(client_sock, buf, BUF_SIZE, 0)) >= 1)
        {
            if (send(client_sock, buf, readret, 0) != readret)
            {
                close_socket(client_sock);
                close_socket(sock);
                fprintf(stderr, "Error sending to client.\n");
                return EXIT_FAILURE;
            }
            memset(buf, 0, BUF_SIZE);
        }

        if (readret == -1)
        {
            close_socket(client_sock);
            close_socket(sock);
            fprintf(stderr, "Error reading from client socket.\n");
            return EXIT_FAILURE;
        }

        if (close_socket(client_sock))
        {
            close_socket(sock);
            fprintf(stderr, "Error closing client socket.\n");
            return EXIT_FAILURE;
        }
    }
    */

error_exit:
    close_socket(sock);

    return EXIT_SUCCESS;
}
