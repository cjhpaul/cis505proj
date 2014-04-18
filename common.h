#include <unistd.h>
#include "sequencer.h"
#include "client.h"
#include "limits.h"
#include <ifaddrs.h>
#include <pthread.h>

void SequencerController(char* recv_data, sockaddr_in addr);
void MultiCast(char* msg);
void ShowListWithLeader(char* buffer);
void GetUpdateList(char* buffer);
void ClientController(char* recv_data, sockaddr_in recvaddr);
void UpdateClientList(char* recv_data);
int LeaderElection(char* name, char* leaderName);

extern int g_fd;
extern char g_leaderinfo[MAXNAME + 20]; //keeps leader/sequencer info
extern char g_leaderName[MAXNAME];
extern int g_fdclient;
extern char g_name[MAXNAME];
extern char g_server[20];
extern int g_port;
extern struct sockaddr_in g_remaddrclient, g_myaddr;
extern int isLeaderChanged;
extern int isEOF;
extern int livecountForSequencer;

extern pthread_t g_pid_receive_thread_client;
extern pthread_t g_keep_alive_thread_client;
extern pthread_t g_fgets_thread_client;