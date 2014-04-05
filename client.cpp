#include <unistd.h>
#include <pthread.h>
#include "client.h"
#include "limits.h"

int g_fdclient;
char g_name[MAXNAME];
char g_server[20];
char g_port[10];
struct sockaddr_in g_remaddrclient;

void *ReceiveThreadWorkerClient (void *);

int DoClientWork(char* name, char* port){
	char *server;
	server = strsep(&port, ":");

	//setup for global variables
	strcpy(g_name, name);
	strcpy(g_server, server);
	strcpy(g_port, port);

	struct sockaddr_in myaddr, g_remaddrclient;
	socklen_t slen = sizeof(g_remaddrclient);
	if ((g_fdclient=socket(AF_INET, SOCK_DGRAM, 0))==-1)
		printf("socket created\n");

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

	//receive_thread
	pthread_t pid_receive_thread;
	pthread_create (&pid_receive_thread, NULL, &ReceiveThreadWorkerClient, NULL);

	//send_thread
	char send_data[BUFSIZE];
	char msg_buffer[MSGSIZE];
	
	//register client info with sequencer
	//once sequencer gets reg msg, it will send user list
	//out-protocol: reg:clientname
	sprintf(send_data, "reg:%s", name);
	sendto(g_fdclient, send_data, strlen(send_data), 0, (struct sockaddr *)&g_remaddrclient, slen);

	while(fgets(msg_buffer, sizeof(msg_buffer), stdin) != NULL){
		//out-protocol: msg:MessageToSendToSequencer
		strcpy(send_data, "msg:");
		strcat(send_data, msg_buffer);
		if (sendto(g_fdclient, send_data, strlen(send_data), 0, (struct sockaddr *)&g_remaddrclient, slen)==-1) {
			perror("sendto");
			exit(1);
		}
	}
	pthread_cancel(pid_receive_thread);
	pthread_join(pid_receive_thread, NULL);
	close(g_fdclient);
	return 0;
}

void* ReceiveThreadWorkerClient (void *p){
	int recvlen;
	socklen_t slen = sizeof(g_remaddrclient);
	char recv_data[BUFSIZE];
	while(1){
		//clear buffer
		memset(&recv_data[0], 0, sizeof(recv_data));
		//get a msg
		recvlen = recvfrom(g_fdclient, recv_data, BUFSIZE, 0, (struct sockaddr *)&g_remaddrclient, &slen);
		if (recvlen >= 0) {
			recv_data[recvlen] = 0;
			//parse & do operation with msg
			ClientController(recv_data);
		}
	}
	pthread_exit (NULL);
}

//controller for received msg
void ClientController(char* recv_data){
	char *cmd;
	cmd = strsep(&recv_data, ":");
	//response to register req, it should get user name
	//in-protocol: reg:clientip:clientport:userlist
	if (strcmp(cmd, "reg") == 0){
		char myip[20];
		char myport[10];
		cmd = strsep(&recv_data, ":");
		strcpy(myip, cmd);
		cmd = strsep(&recv_data, ":");
		strcpy(myport, cmd);
		printf("%s joining a new chat on %s:%s, listening on %s:%s\n", g_name, g_server, g_port, myip, myport);
		printf("Succeeded, current users:\n");
		printf("%s", recv_data);
	}
	//in-protocol: msg:MessageToThisClient
	else if (strcmp(cmd, "msg") == 0) {
		printf("%s", recv_data);		
	}
	return;
}