#include "getip.h"

// Example
// int main ()
// {
//     char ip[20];
//     GetIP(ip);
//     printf("ip: %s\n", ip);
//     return 0;
// }

void GetIP(char* ip){
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    getifaddrs (&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr->sa_family==AF_INET){// && strcmp(ifa->ifa_name, "lo0") != 0) {
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            strcpy(ip, inet_ntoa(sa->sin_addr));
            break;
        }
    }
    freeifaddrs(ifap);
    return;
}