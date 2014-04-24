#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sequencer.h"
#include "client.h"
#include "addrlist.h"
#include "limits.h"

int main(int argc, char **argv)
{
#if defined(TEST)
	addrtest();
	return 0;
#endif
	int result;
	//name length check
	if (strlen(argv[1]) >= MAXNAME)
	{
		printf("Name exceeds %d characters\n", MAXNAME);
		return 0;
	}
	//sequencer
	if (argc == 2){
		result = DoSequencerWork(argv[1], -1);
		printf("Successfully exited\n");
	}
	//client
	else if (argc == 3){
		result = DoClientWork(argv[1], argv[2]);
		printf("Successfully exited\n");
	}
	else {
		printf("Usage: dchat name, or dchat name ip:port\n");
		return 0;
	}
	return 0;
}