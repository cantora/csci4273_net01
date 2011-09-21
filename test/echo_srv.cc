#define BOOST_TEST_MODULE "net01:test_sess_srv"
#include <boost/test/unit_test.hpp>

#include <cerrno>
#include <iostream>

#include "sel_tcp_srv.h"

extern "C" {
#include <arpa/inet.h>
}

using namespace net01;
using namespace std;

class echo_srv : public sel_tcp_srv {
	public:
		echo_srv(int socket) : sel_tcp_srv(socket) {}

	private: 
		virtual selectah::selectah_status_t consume(int socket) {
			size_t bytes = 0;
	
			bytes = echo(socket);
	
			if(bytes < 1) {
				close_socket(socket);
				cout << "closed socket " << socket << ", " << rfd_set_size() << " sockets left" << endl;
			}
	
			return SEL_OK;
		}
	
		size_t echo(int socket)  {
			char buf[32];
			int bytes,sent;
			const sock::sock_info_t *sinfo;

			BOOST_REQUIRE(sock_table()->find(socket) != sock_table()->end() );

			sinfo = &(*sock_table()->find(socket)).second;

			BOOST_REQUIRE(sinfo != NULL);

			if( (bytes = recvfrom(socket, buf, sizeof(buf), 0, NULL, NULL) ) < 0) {
				throw errno;
			}

			if(bytes > 0) {
				sent = send(socket, buf, bytes, 0);
				cout << "echoed " << sent << " bytes to " << inet_ntoa(sinfo->sin.sin_addr) << ':' << ntohs(sinfo->sin.sin_port) << endl;
				BOOST_REQUIRE_EQUAL(bytes, sent);
			}
			
			return bytes;
		}
};

BOOST_AUTO_TEST_CASE(construct) {
	echo_srv es(34);

	BOOST_REQUIRE_EQUAL(1, es.rfd_set_size() );
}

void bbbreak() {}

BOOST_AUTO_TEST_CASE(echo_stuff) {
	echo_srv *es;
	struct sockaddr_in sin;
	socklen_t sin_len = sizeof(sin);
	unsigned short port = 4444;
	int s;
	
	bbbreak();

	sock::passive_sin(port, &sin, &sin_len);

	s = sock::passive_tcp_sock(&sin, &sin_len, 10);

	cout << "addr: " << inet_ntoa(sin.sin_addr) << endl;
	cout << "port: " << ntohs(sin.sin_port) << endl;

	es = new echo_srv(s);

	fflush(stdout);

	es->select_fds(0);

	close(s);		
}


