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
#include <pwd.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
int vflag = 0, sflag = 0, rflag = 0;


void set_flags(int argc, char *argv[])
{
	int c;

	while ((c = getopt(argc, argv, "vsr")) != -1)
		switch (c) {
		case 'v':
			vflag = 1;
			break;
		case 's':
			sflag = 1;
			break;
		case 'r':
			rflag = 1;
			break;
		case '?':
			printf("error flag\n");
			break;
		}
}
int main(int argc, char *argv[])
{
	int client, server, check;
	char buff[256];
	long long buff2[1];
	long long buff3[1];



	struct sockaddr_un client_addr;

	//Deals with -v -r & -s flags.
	set_flags(argc, argv);
	if (sflag == rflag) {
		printf("Flag Error\n");
		return 0;
	}

	//sets the memory and details for the client address.
	memset(&client_addr, 0, sizeof(struct sockaddr_un));
	client_addr.sun_family = AF_UNIX;
	strcpy(client_addr.sun_path, argv[argc - 1]);
	//Creates Local Socket.
	client = socket(AF_UNIX, SOCK_STREAM, 0);

	//Tries to connect to the server socket.
	check = connect(client, (struct sockaddr *)&client_addr,
					sizeof(client_addr));
	if (check == -1) {
		printf("Client side connect failed\n");
		return;
	}
	if (sflag == 1) {
		buff2[0] = atol(argv[argc - 2]);

		write(client, "STOR", 4);
		write(client, buff2, sizeof(long long));
		read(client, buff, 256);
		if (vflag == 1)
			printf("%s\n", buff);
	} else {
		write(client, "RTRV", 4);
		check = read(client, buff3, sizeof(long long));
		if (check == -1) {
			printf("Server Side Num Read Error.\n");
			return;
		}
		if (vflag == 1)
			printf("%ld\n", buff3[0]);
		return buff3[0];
	}

	return 0;
}
