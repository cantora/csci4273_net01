#ifndef LISTENER_H
#define LISTENER_H

#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h> 

/*
 * a passive socket listener
 */
namespace socket {

class listener {
	public:
		listener(int socket, int backlog);
		~listener();

		enum on_accept_status_t {OAS_OK, OAS_STOP};

		int listen(int n_accepts) const;
		virtual on_accept_status_t on_accept(int socket, struct sockaddr_in sin) = 0;

	private:
		const int m_socket;
		const int m_backlog;

}; /* listener */

}; /* socket */
#endif /* S_LISTENER_H */