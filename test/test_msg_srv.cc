#define BOOST_TEST_MODULE "net01:test_sess_srv"
#include <boost/test/unit_test.hpp>

#include <cerrno>
#include <iostream>
#include <cstdlib>
#include <ctime>

#include "sel_tcp_srv.h"
#include "msg.h"

extern "C" {
#include <arpa/inet.h>
}

using namespace net01;
using namespace std;

#define SERVER_PORT 4444
#define NUMBER_OF_MSGS 50
#define WINDOW_TIME 1
#define WAIT_TIME 10
#define MAX_SIZE 1024

vector<string> random_vec; 

class msg_srv : public sel_tcp_srv {
	public:
		msg_srv(int socket) : sel_tcp_srv(socket) {}

	private: 
		virtual selectah::selectah_status_t consume(int socket) {
			size_t bytes = 0;
	
			bytes = store(socket);
	
			if(bytes < 1) {
				close_socket(socket);
				cout << "closed socket " << socket << ", " << rfd_set_size() << " sockets left" << endl;
			}

			if(m_msgs.size() >= NUMBER_OF_MSGS) {
				check_integrity();				
				return SEL_STOP;
			}
	
			return SEL_OK;
		}

		void check_integrity() {
			vector<string>::const_iterator itr;
			
			for(itr = random_vec.begin(); itr != random_vec.end(); itr++) {
				//cout << (m_msgs[0] == *itr) << endl;
				//BOOST_REQUIRE(find(m_msgs.begin(), m_msgs.end(), *itr) != m_msgs.end());				
			}
			
		}
	
		size_t store(int socket)  {
			char buf[MAX_SIZE+1];
			int bytes;
			const sock::sock_info_t *sinfo;

			BOOST_REQUIRE(sock_table()->find(socket) != sock_table()->end() );

			sinfo = &(*sock_table()->find(socket)).second;

			BOOST_REQUIRE(sinfo != NULL);

			if( (bytes = recvfrom(socket, buf, sizeof(buf)-1, 0, NULL, NULL) ) < 0) {
				throw errno;
			}

			if(bytes > 0) {
				buf[bytes] = 0x00;
				m_msgs.push_back(buf);
				
				cout << "stored msg " << m_msgs.size() << ": " << buf << " from " << inet_ntoa(sinfo->sin.sin_addr) << ':' << ntohs(sinfo->sin.sin_port) << endl ;
			}
			
			return 0; /* close the cx */
		}

		vector<msg> m_msgs;
};

int random_data(char *buf, int max_amount) {
	int amt = (rand() % (max_amount-2)) + 1;
	int i = 0;	
	
	for(i = 0; i < amt; i++) {
		buf[i] = (rand() % (90-48) ) + 48;

		if(buf[i] == 0x27) buf[i] = 0x28;
	}
	
	buf[i] = 0x00;

	return amt;
}

void nc_cmd(string &str) {
	char buf[MAX_SIZE];
	char buf2[512];

	str.clear();
	sprintf(buf2, "sleep %d && echo '",((rand() % WINDOW_TIME) + WAIT_TIME) );

	str += buf2;

	random_data(buf, sizeof(buf) );

	random_vec.push_back(buf);

	str += buf;
	sprintf(buf2, "' | tr -d '\\n' | nc 127.0.0.1 %d 2>&1 > /dev/null &", SERVER_PORT);
	str += buf2;
}

BOOST_AUTO_TEST_CASE(recv_msgs) {
	msg_srv *es;
	struct sockaddr_in sin;
	socklen_t sin_len = sizeof(sin);
	unsigned short port = SERVER_PORT;
	int s;
	
	srand(time(NULL));

	sock::passive_sin(port, &sin, &sin_len);

	s = sock::passive_tcp_sock(&sin, &sin_len, 50);

	cout << "addr: " << inet_ntoa(sin.sin_addr) << endl;
	cout << "port: " << ntohs(sin.sin_port) << endl;

	es = new msg_srv(s);

	fflush(stdout);

	for(int i = 0; i < NUMBER_OF_MSGS; i++) {
		string str;
		nc_cmd(str);

		//cout << "nc cmd: " << str << endl;
		system(str.c_str());
		sleep(0.2);
	}

	es->select_fds(0);

	close(s);		
}


