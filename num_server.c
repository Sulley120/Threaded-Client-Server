#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>
#include <limits.h>
#include <time.h>
#include "server.h"

int status = 1;
int position = 1;
int sig_flag = 1;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void handle(int arg)
{
	if (arg == SIGINT)
		status = 0;
	if (arg == SIGHUP)
		sig_flag = 0;
}

int main(int argc, char *argv[])
{
	int server, client, check, i, size;
	pthread_t threads;
	struct Stack *stack = createStack(100);
	struct sockaddr_un server_addr;
	struct sockaddr_un client_addr;
	struct sigaction sig = {0};
	struct args *client_info = malloc(sizeof(struct args));

	sig.sa_handler = handle;
	sig.sa_flags = 0;
	sigemptyset(&sig.sa_mask);
	sigaction(SIGINT, &sig, NULL);
	//Get the server socket.
	server = socket(AF_UNIX, SOCK_STREAM, 0);
	if (server == -1) {
		printf("Server-Side Socket Error\n");
		return;
	}

	memset(&client_addr, 0, sizeof(struct sockaddr_un));
	memset(&server_addr, 0, sizeof(struct sockaddr_un));
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, "/tmp/larocques8");
	unlink("/tmp/larocques8");

	//binds the server to the socket.
	check = bind(server, (struct sockaddr *)&server_addr,
				 sizeof(server_addr));
	if (check == -1) {
		printf("Server Side Bind Error.\n");
		return;
	}
	//Server Listens for clients.
	check = listen(server, 10);
	if (check == -1) {
		printf("Server Side Listen Error.\n");
		return;
	}

	printf("Listening on /tmp/larocques8\n");
	//Keeps the server running.
	while (status) {
		//Obtain the client connection.
		client = accept(server, NULL, NULL);
		if (client == -1) {
			printf("\nServer Side Client-Accept Error.\n");
			break;
		}
		/* Creates a new Thread for each client
		 * connetion and sends them to peon function
		 */
		client_info->new = stack;
		client_info->connection = client;
		if (pthread_create(&threads, NULL,
				peon, (void *)client_info) != 0)
			printf("Could Not Create Thread\n");
		//Prints out array of numbers when SIGHUP is called.
		if (sig_flag == 0) {
			for (i = 0; i < 30; i++)
				printf("%lld ", client_info->array[i]);
			printf("\n");
		}
	}
	//frees all memory.
	printf("Exiting Gracefully\n");
	free(client_info);
	free(stack->array);
	free(stack);
	exit(0);
}

void *peon(void *args)
{

	long long temp;
	int client, check = 0;
	char buff[10];
	long long buff2[1];
	struct args *client_info;

	client_info = (struct args *)args;
	client = client_info->connection;
	memset(buff, 0, 10);
	pthread_detach(pthread_self());
	check = read(client, buff, 4);
	if (check == -1) {
		printf("Server Side STOR read Error.\n");
		return;
	}
	/*If the store command is selected it will read in
	 *the number sent by the client and write back OK
	 *letting the client know it will store the value.
	 */
	if (!strcmp(buff, "STOR")) {
		check = read(client, buff2, sizeof(long long));
		if (check == -1) {
			printf("Server Side Num Read Error.\n");
			return;
		}
		check = write(client, "OK", 2);
		if (check == -1) {
			printf("Server Side OK Error.\n");
			return;
		}
		//Lock around the push function for threads.
		temp = buff2[0];
		pthread_mutex_lock(&lock);
		push(client_info->new, temp);
		client_info->array[position] = temp;
		if (position == 100)
			position = 0;
		position++;
		pthread_mutex_unlock(&lock);
		close(client);
		return;
	}
	/*If the retrieve command is selected it will pop
	 *the number off the stack and write it back to the
	 *client. Thread Lock on the pop() stack function.
	 */
	if (!strcmp(buff, "RTRV")) {
		pthread_mutex_lock(&lock);
		temp = pop(client_info->new);
		client_info->array[position] = 0;
		if (position != 0)
			position--;
		pthread_mutex_unlock(&lock);
		buff2[0] = temp;

		check = write(client,
						buff2, sizeof(long long));
		if (check == -1) {
			printf("Server Side OK Error.\n");
			return;
		}
		close(client);
		return;
	}
}

/*
 * Implementation Taken From:
 * https://www.geeksforgeeks.org/stack-data-structure-introduction-program/
 */
// Stack is full when top is equal to the last index
int isFull(struct Stack *stack)
{
	return stack->top == stack->capacity - 1;
}

// Stack is empty when top is equal to -1
int isEmpty(struct Stack *stack)
{
	return stack->top == -1;
}

// Function to add an item to stack.  It increases top by 1
void push(struct Stack *stack, long long item)
{
	if (isFull(stack))
		return;
	stack->array[++stack->top] = item;
}

// Function to remove an item from stack.  It decreases top by 1
long long pop(struct Stack *stack)
{
	if (isEmpty(stack))
		return INT_MIN;
	return stack->array[stack->top--];
}

struct Stack *createStack(unsigned int capacity)
{
	struct Stack *stack = (struct Stack *) malloc(sizeof(struct Stack));

	stack->capacity = capacity;
	stack->top = -1;
	stack->array = (long long *) malloc(stack->capacity
					* sizeof(long long));
	return stack;
}
