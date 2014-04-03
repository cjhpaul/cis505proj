all:
	g++ -o dchat -Wall -lpthread sequencer.cpp client.cpp main.cpp
debug:
	g++ -o dchat -g sequencer.cpp main.cpp
clean:
	rm *.o
	rm dchat