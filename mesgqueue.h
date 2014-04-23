#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "limits.h"

struct mnode{
  char mesg[BUFSIZE];
  int seqNum;
  struct mnode *next;
  sockaddr_in addr;
};

extern struct mnode* g_SendQueue;
extern struct mnode* g_RecvQueue;

void EnqueueMessageQueue(struct mnode** headRef, char* mesg, int seqNum, sockaddr_in addr);
struct mnode* PeekMessageQueue(struct mnode* headRef, int seqNum, sockaddr_in addr);
struct mnode* DequeueMessageQueue(struct mnode** headRef, int seqNum, sockaddr_in addr);
void Show(struct mnode* head, char* buffer);
void RemoveMessage(struct mnode** headRef, int seqNum, struct sockaddr_in addr);
void RemoveEntireMessage(struct mnode** headRef);
int MsgQCount(struct mnode* headRef);
int IsEmpty(struct mnode* head);
void mesgtest();
