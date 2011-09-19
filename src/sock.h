#ifndef SOCK_H
#define SOCK_H

#include <cerrno>

#include <string>

extern "C" {
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h> 
}

/*
 * sock
 */
namespace net01 {

namespace sock {
	
	void host_sin(const char *host, unsigned short port, struct sockaddr_in *sin);
	int tcp_cx(const struct sockaddr_in *host_addr);

	void passive_sin(unsigned short port, struct sockaddr_in *sin, socklen_t *sin_len);
	int passive_tcp_sock(struct sockaddr_in *sin, socklen_t *sin_len, int qlen);

	enum sock_err {UNKNOWN_HOST};

	class sock_info_t {
		public:
			struct sockaddr_in sin;
	};


}; /* sock */

}; /* net01 */
#endif /* SOCK_H */