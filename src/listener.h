#ifndef LISTENER_H
#define LISTENER_H

#include <cerrno>

extern "C" {
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h> 
}

/*
 * a passive socket listener
 */
namespace net01 {

class listener {
	public:
		listener(int socket, int backlog);
		~listener();

		enum on_accept_status_t {OAS_OK, OAS_STOP};

		int listen_and_accept(int n_accepts);
		
	private:
		virtual on_accept_status_t on_accept(int socket, struct sockaddr_in sin) = 0;

		const int m_socket;
		const int m_backlog;

}; /* listener */

}; /* net01 */
#endif /* LISTENER_H */