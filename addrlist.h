#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "limits.h"
struct anode{
	sockaddr_in addr;
	char name[MAXNAME];
	struct anode *next;
};
//linked list functions (ref: http://cslibrary.stanford.edu/103/)
void Push(struct anode** headRef, sockaddr_in addr, char *name);
void GetNameByAddr(struct anode* head, sockaddr_in addr, char* name);
void DeleteList(struct anode** headRef);
void ShowList(struct anode* head);
int CountList(struct anode* head);

void addrtest();