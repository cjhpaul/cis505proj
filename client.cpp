#include <unistd.h>
#include <pthread.h>
#include "client.h"
#include "limits.h"

int g_fdclient;
struct sockaddr_in g_remaddrclient;
void *ReceiveThreadWorkerClient (void *);

int DoClientWork(char* name, char* port){
	char *server;
	server = strsep(&port, ":");

	struct sockaddr_in myaddr, g_remaddrclient;
	socklen_t slen = sizeof(g_remaddrclient);
	if ((g_fdclient=socket(AF_INET, SOCK_DGRAM, 0))==-1)
		printf("socket created\n");

	//todo: get proper ip and port
	printf("%s joining a new chat on %s:%s, listening on someip:port\n", name, server, port);

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(0);
	if (bind(g_fdclient, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}
	memset((char *) &g_remaddrclient, 0, sizeof(g_remaddrclient));
	g_remaddrclient.sin_family = AF_INET;
	g_remaddrclient.sin_port = htons(atoi(port));
	if (inet_aton(server, &g_remaddrclient.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

	printf("Succeeded, current users:\n");
	//todo: show user

	//receive_thread
	pthread_t pid_receive_thread;
	pthread_create (&pid_receive_thread, NULL, &ReceiveThreadWorkerClient, NULL);

	//send_thread
	char send_data[BUFSIZE];
	char msg_buffer[MSGSIZE];
	
	//register client info with sequencer
	strcpy(send_data, "reg:");
	strcat(send_data, name);
	sendto(g_fdclient, send_data, strlen(send_data), 0, (struct sockaddr *)&g_remaddrclient, slen);

	while(fgets(msg_buffer, sizeof(msg_buffer), stdin) != NULL){
		strcpy(send_data, "msg:");
		strcat(send_data, msg_buffer);
		if (sendto(g_fdclient, send_data, strlen(send_data), 0, (struct sockaddr *)&g_remaddrclient, slen)==-1) {
			perror("sendto");
			exit(1);
		}
	}
	pthread_join(pid_receive_thread, NULL);
	close(g_fdclient);
	return 0;
}

void* ReceiveThreadWorkerClient (void *p){
	int recvlen;
	socklen_t slen = sizeof(g_remaddrclient);
	char recv_data[BUFSIZE];
	while(1){
		recvlen = recvfrom(g_fdclient, recv_data, BUFSIZE, 0, (struct sockaddr *)&g_remaddrclient, &slen);
		if (recvlen >= 0) {
			recv_data[recvlen] = 0;	/* expect a printable string - terminate it */
			printf("%s", recv_data);
		}
	}
	pthread_exit (NULL);
}