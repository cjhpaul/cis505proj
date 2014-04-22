#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "limits.h"

struct mnode{
  char mesg[MAXNAME];
  int seqNum;
  struct mnode *next;
  sockaddr_in addr;
};

extern struct mnode* g_SendQueue;
extern struct mnode* g_RecvQueue;

void EnqueueMessageQueue(struct mnode** headRef, char* mesg, int seqNum, sockaddr_in addr);
struct mnode* PeekMessageQueue(struct mnode* headRef, int seqNum);
struct mnode* DequeueMessageQueue(struct mnode** headRef, int seqNum);
void Show(struct mnode* head);
void RemoveMessage(struct mnode** headRef, int seqNum);
void RemoveEntireMessage(struct mnode** headRef);
int Count(struct mnode* headRef);
int IsEmpty(struct mnode* head);
void mesgtest();
