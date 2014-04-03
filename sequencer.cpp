#include <unistd.h>
#include "sequencer.h"
#include "limits.h"
#include <ifaddrs.h>
#include <pthread.h>
#define PORT 12345

int g_fd;
struct sockaddr_in g_remaddr;
void *ReceiveThreadWorker (void *);

int DoSequencerWork(char* name){
	struct sockaddr_in myaddr;	/* our address */
	// struct sockaddr_in remaddr;	/* remote address */
	
	// struct sockaddr_in* list_adder;
	// list_adder = (sockaddr_in*)malloc(sizeof(sockaddr_in)*MAXCLIENT);

	socklen_t slen = sizeof(g_remaddr);
	// char buf[BUFSIZE];
	// char outbuf[BUFSIZE];
	if ((g_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		return 0;
	}
	
	//todo: get proper ip and port
	char ip[BUFSIZE];
	strcpy(ip, "123:123:123:123");
	printf("%s started a new chat, listening on %s:%d\n", name, ip, PORT);

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(PORT);
	if (bind(g_fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}

	printf("Succeeded, current users:\n");
	//todo: show user
	printf("Waiting for others to join...\n");

	//receive_thread
	pthread_t pid_receive_thread;
	pthread_create (&pid_receive_thread, NULL, &ReceiveThreadWorker, NULL);

	//send_thread
	char send_data[BUFSIZE];
	char send_data_with_name[BUFSIZE + MAXNAME];
    while(scanf("%s", send_data) != EOF){ //toto: need to gracefully exit upon EOF
    	strcpy(send_data_with_name, name);
		strcat(send_data_with_name, ":: ");
		strcat(send_data_with_name, send_data);
		if (sendto(g_fd, send_data_with_name, strlen(send_data_with_name), 0, (struct sockaddr *)&g_remaddr, slen)==-1) {
			perror("sendto");
			exit(1);
		}
    }
    pthread_join(pid_receive_thread, NULL);
	close(g_fd);
	return 0;
}

void* ReceiveThreadWorker (void *p){
	int recvlen;
	socklen_t slen = sizeof(g_remaddr);
	char recv_data[BUFSIZE]; //todo: max length should be adjusted
	while(1){
		recvlen = recvfrom(g_fd, recv_data, BUFSIZE, 0, (struct sockaddr *)&g_remaddr, &slen);
		if (recvlen >= 0) {
			recv_data[recvlen] = 0;	/* expect a printable string - terminate it */
			printf("%s\n", recv_data);
		}
	}
	pthread_exit (NULL);
}