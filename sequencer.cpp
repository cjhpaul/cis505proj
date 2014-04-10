#include <unistd.h>
#include "sequencer.h"
#include "limits.h"
#include "addrlist.h"
#include "getip.h"
#include <ifaddrs.h>
#include <pthread.h>
#define PORT 12346 //hardcoded for now

int g_fd;
void *ReceiveThreadWorker (void *);
struct anode* g_alist = NULL; //keeps all clients info
char g_leaderinfo[MAXNAME + 20]; //keeps leader/sequencer info

int DoSequencerWork(char* name){
	struct sockaddr_in myaddr;
	if ((g_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		return 0;
	}

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(PORT);
	if (bind(g_fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}

	char ip[20];
	GetIP(ip);
	//update leader info and print current users
	sprintf(g_leaderinfo, "%s %s:%d", name, ip, PORT);
	printf("%s started a new chat, listening on %s:%d\n", name, ip, PORT);
	printf("Succeeded, current users:\n");
	char userlist[BUFSIZE];
	ShowListWithLeader(userlist);
	printf("%s", userlist);
	
	printf("Waiting for others to join...\n");

	//receive_thread
	pthread_t pid_receive_thread;
	pthread_create (&pid_receive_thread, NULL, &ReceiveThreadWorker, NULL);

	//send_thread
	char msg[MSGSIZE];
	char send_data[BUFSIZE];
	while(fgets(msg, sizeof(msg), stdin) != NULL){
		sprintf(send_data, "msg:%s:: %s", name, msg);
		MultiCast(send_data);
    }
    pthread_cancel(pid_receive_thread);
    pthread_join(pid_receive_thread, NULL);
	close(g_fd);
	return 0;
}

void* ReceiveThreadWorker (void *p){
	int recvlen;
	struct sockaddr_in addr;
	socklen_t slen = sizeof(addr);
	char recv_data[BUFSIZE]; //todo: max length should be adjusted
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

//controller for received msg
void SequencerController(char* recv_data, sockaddr_in addr){
	char *cmd;
	cmd = strsep(&recv_data, ":");
	//when a client joins
	//register ip, port, name etc
	//in-protocol: reg:name
	if (strcmp(cmd, "reg") == 0) {
		char listbuffer[BUFSIZE];
		char buffer[BUFSIZE];
		//first notice all other client EXCEPT the one that just joined
		//NOTICE Alice joined on 192.168.5.81:1923
		//out-protocol: msg:send_data
		sprintf(buffer, "msg:NOTICE %s joined on %s:%d\n", recv_data, inet_ntoa(addr.sin_addr), addr.sin_port);
		MultiCast(buffer);
		memset((char *)&buffer, 0, sizeof(buffer));

		//add addr to address list		
		Push(&g_alist, addr, recv_data);
		// Push(&g_alist, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), recv_data);
		
		//todo: send upd:name1:ip1:port1:name2:ip2:port2:...:end
		GetUpdateList(buffer);
		MultiCast(buffer);

		//out-protocol: reg:clientip:clientport:userlist
		ShowListWithLeader(listbuffer);
		sprintf(buffer, "reg:%s:%d:%s", inet_ntoa(addr.sin_addr), addr.sin_port, listbuffer);
		//send a list of users to the client
		sendto(g_fd, buffer, strlen(buffer), 0, 
			(struct sockaddr *)&addr, sizeof(addr));
	}
	//msg
	//in-protocol: msg:message
	else if (strcmp(cmd, "msg") == 0) {
		//get the name
		char name[MAXNAME];
		GetNameByAddr(g_alist, addr, name);
		//form msg to send
		//out-protocol: msg:name:: MessageToMulticast
		char send_data[BUFSIZE];
		sprintf(send_data, "msg:%s:: %s", name, recv_data);
		//multicast that msg to everyone that is connected
		MultiCast(send_data);
	} else {
		printf("nothing\n");
	}
	return;
}

void MultiCast(char* msg){
	struct anode* current = g_alist;
	//multicasting for all the registered clients
	while (current != NULL) {
		//do multicast
		sendto(g_fd, msg, 
			strlen(msg), 
			0, 
			(struct sockaddr *)&(current->addr), 
			sizeof(current->addr));
		current = current->next;
	}
	//at last, sequencer gets the msg as well
	char *cmd;
	cmd = strsep(&msg, ":");
	if (strcmp(cmd, "msg") == 0)
		printf("%s", msg);
	return;
}
void ShowListWithLeader(char* buffer){	
	char alistbuffer[BUFSIZE];
	ShowList(g_alist, alistbuffer);
	sprintf(buffer, "%s (Leader)\n%s", g_leaderinfo, alistbuffer);
	return;	
}

void GetUpdateList(char* buffer){
	char line[MAXNAME + 20];
	strcpy(buffer, "upd:");
	struct anode* current = g_alist;
	while (current != NULL) {
		sprintf(line, "%s:%s:%d:", 
			current->name, 
			inet_ntoa(current->addr.sin_addr), 
			current->addr.sin_port);
		strcat(buffer, line);
		current = current->next;
	}
	strcat(buffer, "end");
	return;
}