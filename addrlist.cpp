#include "addrlist.h"
#include <sys/socket.h>
#include <ifaddrs.h>

void Push(struct anode** headRef, sockaddr_in addr, char *name) {
	if (CountList(*headRef) >= MAXCLIENT)
		return;
	struct anode* newNode = new anode;
	memcpy(&(newNode->addr), &addr, sizeof(newNode->addr));
	strcpy(newNode->name, name);
	newNode->next = *headRef;
	*headRef = newNode;
	return;	
}

void Push(struct anode** headRef, char* ip, short port, char *name) {
	printf("*debug*: %s:%d:%s\n", ip, port, name);
	if (CountList(*headRef) >= MAXCLIENT)
		return;
	struct anode* newNode = new anode;
	struct sockaddr_in testAddr;
	memset((char *)&testAddr, 0, sizeof(testAddr));
	testAddr.sin_family = AF_INET;
	inet_aton(ip, &testAddr.sin_addr);
	testAddr.sin_port = htons(port);
	memcpy(&(newNode->addr), &testAddr, sizeof(newNode->addr));
	strcpy(newNode->name, name);
	newNode->next = *headRef;
	*headRef = newNode;
	return;	
}

void GetNameByAddr(struct anode* head, sockaddr_in addr, char* name){
	struct anode* current = head;
	char addrip[20], curip[20];
	strcpy(addrip, inet_ntoa(addr.sin_addr));
	while (current != NULL) {
		strcpy(curip, inet_ntoa(current->addr.sin_addr));
		if (addr.sin_port == current->addr.sin_port &&
			strcmp(addrip, curip) == 0) {
			strcpy(name, current->name);
		}
		current = current->next;
	}
	return;
}

void DeleteList(struct anode** headRef){
	struct anode* current = *headRef;
	struct anode* next;
	while(current != NULL){
		next = current->next;
		delete(current);
		current = next;
	}
	*headRef = NULL;
	return;
}
void ShowList(struct anode* head, char* buffer){
	char line[MAXNAME + 20];
	// char buffer[BUFSIZE];
	strcpy(buffer, "");
	struct anode* current = head;
	while (current != NULL) {
		sprintf(line, "%s %s:%d\n", 
			current->name, 
			inet_ntoa(current->addr.sin_addr), 
			current->addr.sin_port);
		strcat(buffer, line);
		current = current->next;
	}
	// printf("%s", buffer);
	return;
}
int CountList(struct anode* head){
	struct anode* current = head;
	int count = 0;
	while (current != NULL) {
		count++;
		current = current->next;
	}
	return count;
}

void addrtest() {
	struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    char *addr;
    getifaddrs (&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr->sa_family==AF_INET) {
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            addr = inet_ntoa(sa->sin_addr);
            if (strcmp(addr, "127.0.0.1") != 0 && !(addr[0]=='1' && addr[1]=='9' && addr[2]=='2')){
                printf("Interface: %s\tAddress: %s\n", ifa->ifa_name, addr);
                break;
            }
        }
    }
    freeifaddrs(ifap);
    
    //use *sa
    struct anode* alist = NULL;
    char name[MAXNAME];
    char buffer[BUFSIZE];
    strcpy(name, "blah");
    Push(&alist, *sa, name);
    ShowList(alist, buffer);
    printf("%s", buffer);
    printf("count: %d\n", CountList(alist));
    DeleteList(&alist);
    ShowList(alist, buffer);
    printf("%s", buffer);
	return;
}