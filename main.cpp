#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sequencer.h"
#include "client.h"

int main(int argc, char **argv)
{
	int result;
	//todo: name length check
	//sequencer
	if (argc == 2){
		result = DoSequencerWork(argv[1]);
		printf("result: %d\n", result);
	}
	//client
	else if (argc == 3){
		result = DoClientWork(argv[1], argv[2]);
		printf("result: %d\n", result);
	}
	else {
		printf("Usage: dchat name, or dchat name ip:port\n");
		return 0;
	}
	return 0;
}