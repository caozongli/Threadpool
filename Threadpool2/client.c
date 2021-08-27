#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <bits/sockaddr.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/types.h>
#include "wrap.h"

#define SERVER_PORT 8000

void work(void* arg)
{

	struct sockaddr_in* serveraddr = (struct sockaddr_in*)arg;
	int connfd = Socket(AF_INET, SOCK_STREAM, 0);
	Connect(connfd, (struct sockaddr*)serveraddr, sizeof(*serveraddr));
}



int main(void* arg)
{
	int connfd, error_no;
	pthread_t tid[100000];
	struct sockaddr_in serveraddr;

	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, "192.168.125.128", &serveraddr.sin_addr.s_addr);
	serveraddr.sin_port = htons(SERVER_PORT);

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	int i = 0;
	while(1)
	{
		if((error_no = pthread_create(&tid[i++], &attr, work, &serveraddr)))
		{
			perror("pthread_create error...\n");
			exit(1);
		}
		usleep(10000);
	}

	return 0;
}