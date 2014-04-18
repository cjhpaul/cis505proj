#include <unistd.h>
#include "sequencer.h"
#include "limits.h"
#include "addrlist.h"
#include "getip.h"
#include "common.h"
#include <ifaddrs.h>
#include <pthread.h>
// #define PORT 12346 //hardcoded for now
//todo: connect via other clients
//todo: add seq# to all message types

void *ReceiveThreadWorker (void *);
void *KeepAliveThread (void *);

int DoSequencerWork(char* name, int p){
	int PORT;
	if ((g_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		return 0;
	}
	PORT = p;
	if (p == -1) { //todo: clean up stdout output
		PORT = 12346;
	}

	//setup my address
	memset((char *)&g_myaddr, 0, sizeof(g_myaddr));
	g_myaddr.sin_family = AF_INET;
	g_myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	g_myaddr.sin_port = htons(PORT);
	if (bind(g_fd, (struct sockaddr *)&g_myaddr, sizeof(g_myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}

	//update leader info and print current users
	char ip[20];
	GetIP(ip);
	sprintf(g_leaderinfo, "%s %s:%d", name, ip, PORT);
	char userlist[BUFSIZE];
	ShowListWithLeader(userlist);

	//new leader has arrived. Time for all clients to update their client list
	if (p >= 0) {
		char clist[BUFSIZE];
		GetUpdateList(clist);
		MultiCast(clist);
	}
	else {
		printf("%s started a new chat, listening on %s:%d\n", name, ip, PORT);
		printf("Succeeded, current users:\n");
		printf("%s", userlist);
		printf("Waiting for others to join...\n");
	}

	//fork threads
	pthread_t pid_receive_thread, keep_alive_thread;
	pthread_create (&pid_receive_thread, NULL, &ReceiveThreadWorker, NULL);
	pthread_create (&keep_alive_thread, NULL, &KeepAliveThread, NULL);

	//send_thread
	char msg[MSGSIZE];
	char send_data[BUFSIZE];

	//if this is a new leader, multicast to claim "I'm a leader!"
	if (p >= 0) { //todo
		sprintf(send_data, "upl:%s", name);
		MultiCast(send_data);
	}

	//fgets loop
	while(fgets(msg, sizeof(msg), stdin) != NULL){
		sprintf(send_data, "msg:%s:: %s", name, msg);
		MultiCast(send_data);
    }

    //clean up
    close(g_fd);
    pthread_cancel(pid_receive_thread);
	pthread_cancel(keep_alive_thread);
	pthread_join(pid_receive_thread, NULL);
    pthread_join(keep_alive_thread, NULL);
	return 0;
}

void* ReceiveThreadWorker (void *p){
	int recvlen;
	struct sockaddr_in addr;
	socklen_t slen = sizeof(addr);
	char recv_data[BUFSIZE];
	while(1){
		//clear buffer
		memset(&recv_data[0], 0, sizeof(recv_data));
		//get a msg
		recvlen = recvfrom(g_fd, recv_data, BUFSIZE, 0, (struct sockaddr *)&addr, &slen);
		if (recvlen >= 0) {
			recv_data[recvlen] = 0;
			//parse & do operation with msg
			SequencerController(recv_data, addr);
		}
	}
	pthread_exit (NULL);
}

void* KeepAliveThread (void *p){
	char buffer[BUFSIZE];
	char deleted_name[MAXNAME];
	while (1){
		IncrementLiveCount(g_alist);
		sprintf(buffer, "kpa:KEEP_ALIVE");
		MultiCast(buffer);
		if (DeleteNodeByLiveCount(&g_alist, AUDIT_TIME, deleted_name)) {
			sprintf(buffer, "msg:NOTICE %s left the chat or crashed\n", deleted_name);
			MultiCast(buffer);
			//someone just left the chat, time to update the client list
			GetUpdateList(buffer);
			MultiCast(buffer);
		}
		sleep(HEARTBEAT_TIME);
	}
	pthread_exit(NULL);
}