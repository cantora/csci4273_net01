#ifndef SEL_TCP_SRV_H
#define SEL_TCP_SRV_H

#include <cstdio>
#include <map>

#include "selectah.h"
#include "sock.h"

namespace net01 {

class sel_tcp_srv : public selectah {
	public:
		sel_tcp_srv(int socket);
		~sel_tcp_srv();

	protected:

		void close_socket(int socket);

		virtual selectah::selectah_status_t accept_cx();
		virtual selectah::selectah_status_t consume(int socket) = 0;

		virtual selectah::selectah_status_t on_rfds(const fd_set *rfd_set);
		
		const std::map<int, sock::sock_info_t> *sock_table() const { return &m_sock_table; };

	private:

		const int m_listen_sock;
		std::map<int, sock::sock_info_t> m_sock_table;

}; /* sel_tcp_srv */

}; /* net01 */

#endif /* SEL_TCP_SRV_H */