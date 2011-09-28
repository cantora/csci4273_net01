#ifndef COORDINATOR_H
#define COORDINATOR_H

#include <map>
#include <cerrno>
#include <string>

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
		coordinator(int udp_socket, struct sockaddr_in sin);
		~coordinator();

		enum on_msg_status_t {OM_OK, OM_STOP};
		int loop();

	private:
		on_msg_status_t on_msg(const char *buf, int len, struct sockaddr_in &sin, socklen_t &sinlen);
		char start_session(const char *buf, int len);
		void term(const char *buf, int len);
		int find(const char *buf, int len, char *response, int &response_len);
	
		class session {
			public:
				int pid;
				unsigned short port;
				std::string token;
		};

		const int m_socket;
		const struct sockaddr_in m_sin;
		std::map<std::string, session> m_sessions;

}; /* coordinator */

}; /* net01 */
#endif /* COORDINATOR_H */