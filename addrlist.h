#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "limits.h"

//todo: duplicate names can be buggy.
struct anode{
	sockaddr_in addr;
	char name[MAXNAME];
	int livecount;
	int seqSend; //seq# 0 or less than 0 can be ignored.
	int seqRecv;
	struct anode *next;
};

extern struct anode* g_alist; //keeps all clients info

//linked list functions (ref: http://cslibrary.stanford.edu/103/)
void Push(struct anode** headRef, sockaddr_in addr, char *name);
void Push(struct anode** headRef, char* ip, short port, char *name);
void GetNameByAddr(struct anode* head, sockaddr_in addr, char* name);
void GetAddrByName(struct anode* head, sockaddr_in addr, char* name);
void DeleteList(struct anode** headRef);
void ShowList(struct anode* head, char* buffer);
int CountList(struct anode* head);
void IncrementLiveCount(struct anode* head);
void ZeroizeLiveCount(struct anode* head, char* name);
void DeleteNode(struct anode** headRef, char* name);
int DeleteNodeByLiveCount(struct anode** headRef, int livecountlimit, char* name);

int GetSeqSendByAddr(struct anode* head, sockaddr_in addr);
int GetSeqRecvByAddr(struct anode* head, sockaddr_in addr);
void SetSeqSendByAddr(struct anode* head, sockaddr_in addr, int seq);
void SetSeqRecvByAddr(struct anode* head, sockaddr_in addr, int seq);

void addrtest();