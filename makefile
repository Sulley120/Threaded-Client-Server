CC = gcc

client: num_client.c
	$(CC) num_client.c -o client

server: num_server.o
	$(CC) -pthread -g num_server.o -o server

num_server.o: num_server.c server.h
	$(CC) num_server.c server.h -c

test: client server
	./server &
	./client -v -s 7 /tmp/larocques8
	./client -v -r /tmp/larocques8