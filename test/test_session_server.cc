#include <iostream>

#include "session_server.h"
#include "sock.h"

#include <ctime>

using namespace std;
using namespace net01;

int main(int argc, char *argv[]) {
	unsigned short port;
	struct sockaddr_in sin;
	int socket;
	socklen_t sin_len = sizeof(sin);
	session_server *ss;

	srand(time(NULL));
	
	if(argc == 1) {
		port = (rand() % 2000) + 3000;
	}
	else if(argc == 2) {
		port = strtol(argv[1], NULL, 10);
	}

	cout << "session server on port " << port << endl;
	
	sock::passive_sin(port, &sin, &sin_len); 	
	
	try {

		socket = sock::passive_tcp_sock(&sin, &sin_len, 10);
		cout << "created passive socket" << endl;
			
		cout << "start server" << endl;
	
		ss = new session_server("blahblah", socket);
		
		cout << "enter main loop" << endl;
		cout.flush();

		ss->select_fds(0);

	} /*try*/
	catch(int e) {
		cout << "exception from session server: " << strerror(e) << endl;
		
	}

	delete ss;	
	close(socket);
	cout << "closed socket, goodbye" << endl;
}