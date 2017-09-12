################################################################################
# Makefile                                                                     #
#                                                                              #
# Description: This file contains the make rules for Recitation 1.             #
#                                                                              #
# Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                          #
#          Wolf Richter <wolf@cs.cmu.edu>                                      #
#                                                                              #
################################################################################

default: echo_server echo_client

echo_server:
	@gcc echo_server.c dbg_logger.c -o echo_server -Wall -Werror

echo_client:
	@gcc echo_client.c dbg_logger.c -o echo_client -Wall -Werror

clean:
	@rm echo_server echo_client
