CC = gcc
CFLAGS = -I. -DYACCDEBUG
DEPS = helper.h parse.h y.tab.h protocol.h
OBJ = echo_server.o helper.o y.tab.o lex.yy.o parse.o protocol.o
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
