#ifndef COORDINATOR_H
#define COORDINATOR_H

#include <cerrno>

extern "C" {
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h> 
#include <arpa/inet.h>
}

/*
 * a passive socket coordinator
 */
namespace net01 {

class coordinator {
	public:
		coordinator(int udp_socket);
		~coordinator();

		enum on_msg_status_t {OM_OK, OM_STOP};
		int loop();

	private:
		on_msg_status_t on_msg(const char *buf, int len, struct sockaddr_in &sin, socklen_t &sinlen);

		const int m_socket;

}; /* coordinator */

}; /* net01 */
#endif /* COORDINATOR_H */