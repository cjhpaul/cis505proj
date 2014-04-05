#include <unistd.h>
#include "sequencer.h"
#include "limits.h"
#include "addrlist.h"
#include <ifaddrs.h>
#include <pthread.h>
#define PORT 12346

int g_fd;
struct sockaddr_in g_remaddr;
void *ReceiveThreadWorker (void *);
struct anode* g_alist = NULL;

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
	//todo: get proper ip and port
	printf("%s started a new chat, listening on %s:%d\n", name, inet_ntoa(myaddr.sin_addr), PORT);

	printf("Succeeded, current users:\n");
	//todo: show user
	printf("Waiting for others to join...\n");

	//receive_thread
	pthread_t pid_receive_thread;
	pthread_create (&pid_receive_thread, NULL, &ReceiveThreadWorker, NULL);

	//send_thread
	char msg[MSGSIZE];
	char send_data[BUFSIZE];
	//todo: need to gracefully exit upon EOF
	while(fgets(msg, sizeof(msg), stdin) != NULL){
		sprintf(send_data, "%s:: %s", name, msg);
		MultiCast(send_data);
    }
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
		//parse & do operation with msg
		SequencerController(recv_data, addr);
	}
	pthread_exit (NULL);
}

void SequencerController(char* recv_data, sockaddr_in addr){
	char *cmd;
	cmd = strsep(&recv_data, ":");
	//register ip, port, name etc
	if (strcmp(cmd, "reg") == 0) {
		Push(&g_alist, addr, recv_data);
		//debug
	    ShowList(g_alist);
	    //end of debug
	}//msg 
	else if (strcmp(cmd, "msg") == 0) {
		//get the name
		char name[MAXNAME];
		GetNameByAddr(g_alist, addr, name);
		//form msg to send
		char send_data[BUFSIZE];
		sprintf(send_data, "%s:: %s", name, recv_data);
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
	printf("%s", msg);
	return;
}