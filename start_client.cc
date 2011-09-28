#include <iostream>

#include "client.h"
#include "sock.h"
#include <cstdio>

using namespace std;
using namespace net01;

void start_client(struct sockaddr_in &sin, int coord_socket) {
	client cl(sin, coord_socket);
	
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

