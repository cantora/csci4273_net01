#include "s_listener.h"

listener::listener(int socket, int backlog) : m_socket(socket), m_backlog(backlog) {}

listener::~listener() {
	close(m_socket);
}

/*
 * n_accepts: accept n sockets, then return
 * 		n_accepts < 1 => infinite accepts
 *
 * return value: the number of connections accepted.
 */
int listener::listen(int n_accepts) const {
	struct sockaddr_in sin;
	socklen_t len;
	int new_sock, i;
	on_accept_status_t status;

	/* wait for connection, then receive and print text */ 
	if( listen(m_socket, m_backlog) != 0) {
		throw errno;
	} 
	
	for(i = 0; (n_accepts < 1) || (i < n_accepts); i++) {
		if ( (new_sock = accept(m_socket, (struct sockaddr *)&sin, &len)) < 0 ) { 
			throw errno;
		}
	
		status = on_accept(new_sock, sin);
	
		if(status == OAS_STOP) {
			break;
		}
	}	

	return i + 1;
}