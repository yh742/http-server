CC = gcc
CFLAGS = -I. -DYACCDEBUG
DEPS = dbg_logger.h parse.h y.tab.h
OBJ = echo_server.o dbg_logger.o y.tab.o lex.yy.o parse.o
FLAGS = -g -Wall

default: all

all: echo_server echo_client

lex.yy.c: lexer.l
	flex $^

y.tab.c: parser.y
	yacc -d $^

%.o: %.c $(DEPS)
	$(CC) $(FLAGS) -c -o $@ $< $(CFLAGS)

echo_server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

echo_client: echo_client.o
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm *.o echo_server echo_client
