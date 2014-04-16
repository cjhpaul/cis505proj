#include <unistd.h>
#include <pthread.h>
#include "addrlist.h"
#include "client.h"
#include "limits.h"
#include "sequencer.h"

int g_fdclient;
char g_name[MAXNAME];
char g_server[20];
int g_port;
struct sockaddr_in g_remaddrclient;
int isLeaderChanged;

void *ReceiveThreadWorkerClient (void *);

int DoClientWork(char* name, char* port){
	char *server;
	server = strsep(&port, ":");
	//setup for global variables
	strcpy(g_name, name);
	strcpy(g_server, server);
	g_port = atoi(port);

	isLeaderChanged = 0;

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
		//update the leader/sequencer info
		if (isLeaderChanged) {
			isLeaderChanged = 0;
			memset((char *) &g_remaddrclient, 0, sizeof(g_remaddrclient));
			g_remaddrclient.sin_family = AF_INET;
			g_remaddrclient.sin_port = htons(ntohs(g_port));
			if (inet_aton(g_server, &g_remaddrclient.sin_addr)==0) {
				fprintf(stderr, "inet_aton() failed\n");
				exit(1);
			}
		}
		//***debug
		if (strcmp(msg_buffer, "leader\n") == 0) {
			int myport;
			if ((myport = LeaderElection(name)) != 0) {
				printf("I'm a new leader!\n");
				pthread_cancel(pid_receive_thread);
				pthread_join(pid_receive_thread, NULL);
				close(g_fdclient);
				DeleteNode(&g_alist, name);
				DoSequencerWork(name, myport);
			}
			continue;
		}
		//***end of debug

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
	struct sockaddr_in recvaddr;
	socklen_t slen = sizeof(recvaddr);
	char recv_data[BUFSIZE];
	while(1){
		//clear buffer
		memset(&recv_data[0], 0, sizeof(recv_data));
		//get a msg
		recvlen = recvfrom(g_fdclient, recv_data, BUFSIZE, 0, (struct sockaddr *)&recvaddr, &slen);
		if (recvlen >= 0) {
			recv_data[recvlen] = 0;
			//parse & do operation with msg
			ClientController(recv_data, recvaddr);
		}
	}
	pthread_exit (NULL);
}

//controller for received msg
void ClientController(char* recv_data, sockaddr_in recvaddr){
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
		printf("%s joining a new chat on %s:%d, listening on %s:%s\n", g_name, g_server, g_port, myip, myport);
		printf("Succeeded, current users:\n");
		printf("%s", recv_data);
	}
	//in-protocol: msg:MessageToThisClient
	else if (strcmp(cmd, "msg") == 0) {
		printf("%s", recv_data);		
	}
	else if (strcmp(cmd, "upd") == 0) {
		UpdateClientList(recv_data);
	}
	else if (strcmp(cmd, "upl") == 0) {
		strcpy(g_name, recv_data);
		isLeaderChanged = 1;
		strcpy(g_server, inet_ntoa(recvaddr.sin_addr));
		g_port = recvaddr.sin_port;
	}
	else if (strcmp(cmd, "kpa") == 0) {
		if (strcmp(recv_data, "KEEP_ALIVE") == 0){
			char alive[20];
			strcpy(alive, "kpa:ALIVE");
			socklen_t slen = sizeof(recvaddr);
			if (sendto(g_fdclient, alive, strlen(alive), 0, (struct sockaddr *)&recvaddr, slen)==-1) {
				perror("sendto");
				exit(1);
			}
		}
	}
	return;
}

// name:ip:port:...:end
void UpdateClientList(char* recv_data){
	char *token;
	char name[MAXNAME], ip[20], port[10];
	//initialize the client list
	DeleteList(&g_alist);
	//add clients to the list
	while ((token = strsep(&recv_data, ":")) != NULL){
		if (strcmp(token, "end") == 0)
			break;
		strcpy(name, token);
		if ((token = strsep(&recv_data, ":")) == NULL){
			printf("error\n");
		}
		strcpy(ip, token);
		if ((token = strsep(&recv_data, ":")) == NULL){
			printf("error\n");
		}
		strcpy(port, token);
		Push(&g_alist, ip, ntohs(atoi(port)), name);
	}
}

//leader is whoever last joined in the chat
//return port number of the new leader
int LeaderElection(char* name) {
	struct anode* current = g_alist;
	while (current->next != NULL) {
		current = current->next;
	}
	if (strcmp(name, current->name) == 0) {
		return htons(current->addr.sin_port);
	}
	return 0;
}