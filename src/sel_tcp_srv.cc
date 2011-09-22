#include "sel_tcp_srv.h"

using namespace net01;

/*
 * pass the main listen socket to
 * receive connection requrests on.
 * it should be open, bound, and
 * set to listen mode.
 */ 
sel_tcp_srv::sel_tcp_srv(int socket) : m_listen_sock(socket) {
	add_rfd(m_listen_sock);
	
}

sel_tcp_srv::~sel_tcp_srv() {
	std::set<int>::iterator it;
	
	for(it = m_rfds.begin(); it != m_rfds.end(); it++) {
		/* dont close m_listen_sock b.c. sel_tcp_srv didnt open it */
		if(*it != m_listen_sock) {
			/* dont need to call close_socket
 			 * here b.c. m_rfds and m_sock_table
 			 * will get destroyed anyway
			 */
			close(*it);

		}

	}
}

void sel_tcp_srv::close_socket(int socket) {
	int removed = 0;

	removed = remove_rfd(socket);
	assert(removed == 1);

	removed = m_sock_table.erase(socket);
	assert(removed == 1);

	if( close(socket) != 0) {
		throw errno;	
	}
}


/*
 * accept a connection on m_listen_sock
 * add the address of client to m_sock_table
 * and add the new socket to m_rfds
 */
selectah::selectah_status_t sel_tcp_srv::accept_cx() {
	int new_s;
	struct sockaddr_in sin; 
	socklen_t len = sizeof(sin);
	
	if ( (new_s = accept(m_listen_sock, (struct sockaddr *)&sin, &len) ) < 0) { 
		throw errno;
	}

	//m_sock_table[new_s] = new sock::sock_info_t;
	memcpy(&m_sock_table[new_s].sin, &sin, len);

	add_rfd(new_s);	

	return SEL_OK;
}

/*
 * check which fds are ready for reading.
 * if m_listen_sock has data, accept cx's from it.
 * if anything else has data, consume that input.
 */
selectah::selectah_status_t sel_tcp_srv::on_rfds(const fd_set *rfd_set) {
	std::set<int>::const_iterator it;
	fd_set set;
	selectah_status_t status = SEL_OK;

	FD_COPY(rfd_set, &set);

	for(it = m_rfds.begin(); it != m_rfds.end();) {
		/* copy it and then increment.
		 * we do this in case the element at 
 		 * it gets removed before the loop ends
		 */
		std::set<int>::const_iterator itcp = it++;
		if(FD_ISSET(*itcp, &set) != 0) {
			if(*itcp == m_listen_sock) {
				accept_cx();
			}
			else {
				if( (status = consume(*itcp) ) != SEL_OK) {
					return status;
				}
			}
		}
	}
		
	return SEL_OK;
}