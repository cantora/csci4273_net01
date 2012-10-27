#include <iostream>

#include <cstdio>
#include <cstring>
#include <cstdlib>

extern "C" {
#include <unistd.h>
}
#include "coordinator.h"
#include "sock.h"

using namespace std;
using namespace net01;

void start_coord(struct sockaddr_in &sin, int coord_socket) {
	coordinator coord(coord_socket, sin);

	cout << "start coordinator on port " << ntohs(sin.sin_port) << endl;
	
	try {
		coord.loop();
	}
	catch(int e) {
		cout << "error in coordinator: " << strerror(e) << endl;
	}
}

int main(int argc, char *argv[]) {
	struct sockaddr_in coord_sin;
	int s;
	int port;
	socklen_t slen = sizeof(coord_sin);
	
	try {
	if( (argc < 2) || (argc > 2) ) {
		cout << "usage: " << argv[0] << " PORT" << endl;
		return 0;
	}
	else {
		port = strtol(argv[1], NULL, 10);
		sock::passive_sin(port, &coord_sin, &slen);
	}
	
	} /* try */
	catch(int e) {
		cout << strerror(e) << endl;
		return 1;
	}

	try {	
		s = sock::bound_udp_sock(&coord_sin, &slen);
	} 
	catch (int e) {
		cout << "error binding udp socket: " << strerror(e) << endl;
		return 1;
	}

	start_coord(coord_sin, s);

	cout << "exiting..." << endl;

	return 0;
}

