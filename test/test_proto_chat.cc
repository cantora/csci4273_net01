#define BOOST_TEST_MODULE "net01:test_sess_srv"
#include <boost/test/unit_test.hpp>

#include <cerrno>
#include <iostream>
#include <cstdlib>
#include <ctime>

#include "proto_chat.h"
#include "sock.h"

extern "C" {
#include <arpa/inet.h>
}

using namespace net01;
using namespace std;

#define SERVER_PORT 4444

void nc_cmd(string &str) {
	char buf2[512];

	str.clear();
	sprintf(buf2, "nc -lp %d 2>&1 > /dev/null &", SERVER_PORT);
	str += buf2;
}

BOOST_AUTO_TEST_CASE(send_msg) {
	string msg_str("hi nc! im a super cool chat_proto msg!");
	msg m(msg_str);

	struct sockaddr_in sin;
	unsigned short port = SERVER_PORT;
	int s,sent;
	
	sock::host_sin("localhost", port, &sin);

	string str;
	nc_cmd(str);

	cout << "nc cmd: " << str << endl;
	system(str.c_str());
	sleep(1);
	
	s = sock::tcp_cx(&sin);

	auto_ptr<istream> istrm = m.istrm();

	sent = proto_chat::send_msg(s, istrm);
	
	BOOST_REQUIRE_EQUAL(sent, msg_str.size()+2);
	close(s);		
}


