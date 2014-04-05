#include "addrlist.h"
#include <sys/socket.h>
#include <ifaddrs.h>

void Push(struct anode** headRef, sockaddr_in addr, char *name) {
	struct anode* newNode = new anode;
	memcpy(&(newNode->addr), &addr, sizeof(newNode->addr));
	strcpy(newNode->name, name);
	newNode->next = *headRef;
	*headRef = newNode;
	return;	
}

void GetNameByAddr(struct anode* head, sockaddr_in addr, char* name){
	struct anode* current = head;
	while (current != NULL) {
		if (memcmp(&(current->addr), &addr, sizeof(addr)) == 0)
			strcpy(name, current->name);
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
	char line[50];
	strcpy(buffer, "");
	struct anode* current = head;
	while (current != NULL) {
		sprintf(line, "%s %s:%d, ", 
			current->name, 
			inet_ntoa(current->addr.sin_addr), 
			current->addr.sin_port);
		strcat(buffer, line);
		current = current->next;
	}
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
    strcpy(name, "blah");
    Push(&alist, *sa, name);
    char buff[1024];
    ShowList(alist, buff);
    printf("out: %s\n", buff);
    printf("count: %d\n", CountList(alist));
    DeleteList(&alist);
    ShowList(alist, buff);
    printf("out2: %s\n", buff);
	return;
}