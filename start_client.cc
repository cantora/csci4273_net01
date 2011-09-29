#include <iostream>

#include "client.h"
#include "sock.h"
#include <cstdio>

using namespace std;
using namespace net01;

void start_client(struct sockaddr_in &sin, int coord_socket) {
	client cl(sin, coord_socket);
	
	cout << "commands:" << endl;
	cout << "\tj->join: join a chat session. this will prompt for a port." << endl;
	cout << "\tm->msg: submit a message to the chat session. this will prompt to enter a message" << endl;
	cout << "\tp->poll: poll for recent messages from the chat session. no arguments needed" << endl;
	cout << "\td->drop: close connection with chat session, if any is open. no arguments." << endl;
	cout << "\tf->find: ask coordinator for the port of a chat session. prompts for a session name." << endl;
	cout << "\ts->start: ask coordinator to start a chat session. prompts for a session name" << endl;
	cout << "\te->exit: exit this program." << endl;

	cout << "when prompted with the following prompt:" << endl;
	cout << "j->join, m->msg, p->poll, d->drop, f->find, s->start, e->exit: " << endl;
	cout << "type the letter representing the comand you want and press enter." << endl;
	cout << "the command will prompt you for arguments if necessary." << endl << endl;

	cout << "starting up..." << endl;
	try {
		cl.loop();
	}
	catch(int e) {
		cout << "error in client: " << strerror(e) << endl;
	}
}

int main(int argc, char *argv[]) {
	struct sockaddr_in coord_sin;
	int s;
	int port;
	socklen_t slen = sizeof(coord_sin);
	
	try {
	if(argc < 2) {
		cout << "usage: " << argv[0] << " PORT [HOST]" << endl;
		return 0;
	}
	else if(argc == 2) {
		port = strtol(argv[1], NULL, 10);
		sock::host_sin("localhost", port, &coord_sin);
	}
	else {
		port = strtol(argv[1], NULL, 10);
		sock::host_sin(argv[2], port, &coord_sin);
	}
	} /* try */
	catch(int e) {
		cout << strerror(e) << endl;
		return 1;
	}
	
	if( (s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) ) < 0) {
		perror("socket: ");
		return 1;
	}

	start_client(coord_sin, s);

	cout << "exiting..." << endl;

	return 0;
}

