#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
int DoSequencerWork(char*);
void SequencerController(char*, sockaddr_in);
void MultiCast(char*);
void ShowListWithLeader(char*);