#include <unistd.h>
#include <pthread.h>
#include "addrlist.h"
#include "client.h"
#include "limits.h"
#include "sequencer.h"
#include "common.h"
#include "mesgqueue.h"

void *ReceiveThreadWorkerClient (void *);
void *FgetsThreadClient (void *);

int DoClientWork(char* name, char* port){
	char *server;
	server = strsep(&port, ":");
	//setup for global variables
	strcpy(g_name, name);
	strcpy(g_server, server);
	g_port = atoi(port);

	isEOF = 0;
	isLeaderChanged = 0;
	livecountForSequencer = 0;
	g_seqSend = 0;
	g_seqRecv = 0;

	socklen_t slen = sizeof(g_remaddrclient);
	if ((g_fdclient=socket(AF_INET, SOCK_DGRAM, 0))==-1)
		printf("socket created\n");

	//my addr
	memset((char *)&g_myaddr, 0, sizeof(g_myaddr));
	g_myaddr.sin_family = AF_INET;
	g_myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	g_myaddr.sin_port = htons(0);
	if (bind(g_fdclient, (struct sockaddr *)&g_myaddr, sizeof(g_myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}
	//remote addr
	memset((char *) &g_remaddrclient, 0, sizeof(g_remaddrclient));
	g_remaddrclient.sin_family = AF_INET;
	g_remaddrclient.sin_port = htons(atoi(port));
	if (inet_aton(server, &g_remaddrclient.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

	//send_thread
	char send_data[BUFSIZE];
	
	//fork thread
	pthread_create (&g_pid_receive_thread_client, NULL, &ReceiveThreadWorkerClient, NULL);
	pthread_create (&g_fgets_thread_client, NULL, &FgetsThreadClient, NULL);

	//register client info with sequencer
	//once sequencer gets rec msg, it will send user list
	//out-protocol: rec:seq:clientname
	sprintf(send_data, "0:rec:%s", name);
	char send_data_chksum[BUFSIZE];
	sprintf(send_data_chksum, "%d:%s", chash(send_data), send_data);
	sendto(g_fdclient, send_data_chksum, strlen(send_data_chksum), 0, (struct sockaddr *)&g_remaddrclient, slen);

	while (!isEOF){
		livecountForSequencer++;
		if (livecountForSequencer >= AUDIT_TIME) {
			int myport;
			char leaderName[MAXNAME];
			if ((myport = LeaderElection(name, leaderName)) > 0) {
				close(g_fdclient);
				pthread_cancel(g_pid_receive_thread_client);				
				pthread_cancel(g_fgets_thread_client);
				DeleteNode(&g_alist, name);
				DoSequencerWork(name, myport);
				pthread_join(g_pid_receive_thread_client, NULL);
				pthread_join(g_fgets_thread_client, NULL);
				return 0;
			}
			else if (myport == 0) { //im a client
				//waiting for a response from a new leader
				if (livecountForSequencer >= AUDIT_TIME + AUDIT_TIME) {
					//enough waiting, i'll remove the new leader
					//note: this happens when a (supposely) newly selected leader happens to crash/leave
					// before it does all the necessary sequencer job
					//it will remove the newly elected/non-responsive leader then go another leader election
					DeleteNode(&g_alist, leaderName);
				}
				GetAddrByName(g_alist, g_remaddrclient, leaderName);
			}
			else if (myport == -1) {
				fprintf(stderr, "timeout\n");
				break;
			}
		}
		sleep(HEARTBEAT_TIME);
	}
	close(g_fdclient);
	pthread_cancel(g_pid_receive_thread_client);
    pthread_cancel(g_fgets_thread_client);
    pthread_join(g_pid_receive_thread_client, NULL);
    pthread_join(g_fgets_thread_client, NULL);
	return 0;
}

void* ReceiveThreadWorkerClient (void *p){
	int recvlen;
	struct sockaddr_in recvaddr;
	socklen_t slen = sizeof(recvaddr);
	char recv_data[BUFSIZE];
	char recv_data2[BUFSIZE];
	while(1){
		//clear buffer
		memset(&recv_data[0], 0, sizeof(recv_data));
		//get a msg
		recvlen = recvfrom(g_fdclient, recv_data, BUFSIZE, 0, (struct sockaddr *)&recvaddr, &slen);
		if (recvlen >= 0) {
			recv_data[recvlen] = 0;
			if (CheckSum(recv_data, recv_data2)) {
				//parse & do operation with msg
				DoClientMessageQueueOperation(recv_data2, recvaddr);
			}
		}
	}
	pthread_exit(NULL);
}

void *FgetsThreadClient (void *) {
	//send_thread
	char send_data[BUFSIZE];
	char msg_buffer[MSGSIZE];

	while(fgets(msg_buffer, sizeof(msg_buffer), stdin) != NULL){
		//update the leader/sequencer info
		if (isLeaderChanged) {
			isLeaderChanged = 0;
			memset((char *) &g_remaddrclient, 0, sizeof(g_remaddrclient));
			g_remaddrclient.sin_family = AF_INET;
			g_remaddrclient.sin_port = htons(g_port);
			if (inet_aton(g_server, &g_remaddrclient.sin_addr)==0) {
				fprintf(stderr, "inet_aton() failed\n");
				exit(1);
			}
		}
		//out-protocol: msg:MessageToSendToSequencer
		sprintf(send_data, "%d:msg:%s", ++g_seqSend, msg_buffer);

		char send_data_chksum[BUFSIZE];
		sprintf(send_data_chksum, "%d:%s", chash(send_data), send_data);
		EnqueueMessageQueue(&g_SendQueue, send_data_chksum, g_seqSend, g_remaddrclient);

		if (sendto(g_fdclient, send_data_chksum, strlen(send_data_chksum), 0, (struct sockaddr *)&g_remaddrclient, sizeof(g_remaddrclient))==-1) {
			perror("sendto");
			exit(1);
		}
	}
	isEOF = 1;
	close(g_fdclient);
	pthread_cancel(g_pid_receive_thread_client);
	pthread_join(g_pid_receive_thread_client, NULL);
	pthread_exit(NULL);
}