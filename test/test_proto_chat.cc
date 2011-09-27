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
#include <unistd.h>
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

BOOST_AUTO_TEST_CASE(ascii) {
	char buf[] = "\x12\xf0\xf4hello\x01\x05\xf9\x7f\x82";
	string cmp("???hello?????");

	/* pass sizeof(buf)-1 to avoid killing the null terminator */
	proto_chat::gsub_bad_ascii(buf, sizeof(buf)-1 );

	cout << buf << endl;

	string str(buf);
	BOOST_REQUIRE_EQUAL(str, cmp);
}

BOOST_AUTO_TEST_CASE(send_msg) {
	string msg_str("hi nc! im a super cool chat_proto msg!");
	msg m(msg_str);

	struct sockaddr_in sin;
	unsigned short port = SERVER_PORT;
	int s,sent;
	proto_chat::send_status_t status;

	sock::host_sin("localhost", port, &sin);

	string str;
	nc_cmd(str);

	cout << "nc cmd: " << str << endl;
	system(str.c_str());
	sleep(1);
	
	s = sock::tcp_cx(&sin);

	auto_ptr<istream> istrm = m.istrm();

	status = proto_chat::send_msg(s, *istrm, sent);

	BOOST_REQUIRE_EQUAL(status, proto_chat::SND_DONE);	
	BOOST_REQUIRE_EQUAL(sent, msg_str.size()+2);
	close(s);		
}

BOOST_AUTO_TEST_CASE(recv_msg) {
	string msg_str("im a super cool chat proto msg!\ncheckout this illegal character \xf3.");
	msg m(msg_str);
	int pid;
	unsigned short port;
	srand(time(NULL));

	cout << "send message:" << endl << m.str() << endl;

	port = (rand() % 3000) + 4000;
	pid = fork();
	BOOST_REQUIRE(pid >= 0);

	if(pid == 0) {
		/* client setup */
		struct sockaddr_in sin;
		int s,sent;
		proto_chat::send_status_t status;
		
		try{
		sock::host_sin("localhost", port, &sin);

		sleep(2);
		s = sock::tcp_cx(&sin);
	
		auto_ptr<istream> istrm = m.istrm();
	
		status = proto_chat::send_msg(s, *istrm, sent);
	
		BOOST_REQUIRE_EQUAL(status, proto_chat::SND_DONE);	
		BOOST_REQUIRE_EQUAL(sent, msg_str.size()+2);
		close(s);
		}
		catch(int e) {
			cout << strerror(e) << endl;
			close(s);
			exit(1);
		}
		exit(0);
	}
	else {
		/* host setup */
		msg from_child;
		struct sockaddr_in hsin;
		int hsocket, new_s;
		socklen_t hsin_len = sizeof(hsin);
		sock::passive_sin(port, &hsin, &hsin_len); 	
		
		cout << "create passive sock" << endl;

		try {

		hsocket = sock::passive_tcp_sock(&hsin, &hsin_len, 10);
		
		auto_ptr<std::ostream> ostrm = from_child.ostrm();
		int recd = 0;
		proto_chat::recv_status_t status = proto_chat::RCV_CONT;;

		cout << "wait for accept" << endl;
		cout.flush();

		BOOST_REQUIRE( (new_s = accept(hsocket, (struct sockaddr *)&hsin, &hsin_len) ) >= 0);
		
		cout << "accepted cx" << endl;

		cout << "set non blocking" << endl;
		
		sock::set_nonblocking(new_s);

		while(status == proto_chat::RCV_CONT) {
			cout << '.';
			status = proto_chat::recv_msg(new_s, *ostrm, recd);
			sleep(0.1);
		}

		cout << endl << "got message from child: " << endl << from_child.str() << endl;
		
		close(new_s);
		close(hsocket);

		BOOST_REQUIRE(status == proto_chat::RCV_DONE);
		BOOST_REQUIRE(recd == m.size());
		BOOST_REQUIRE_EQUAL(from_child.size(), m.size());		

		} /*try*/
		catch(int e) {
			cout << strerror(e) << endl;
			close(new_s);
			close(hsocket);

			BOOST_REQUIRE(false);
		}
	}

	
}

BOOST_AUTO_TEST_CASE(recv_long_msg) {
	string msg_str("im a super cool chat proto msg!\ncheckout this illegal character \xf3. damn thats cool\nnow im gonna go on and on with stuff to test the length of the possible things we can send over chat proto...The Ricky Gervais Show is a comedy audio show in the UK starring Ricky Gervais, Stephen Merchant, and Karl Pilkington, later adapted into an animated televised version debuting for HBO and Channel 4 in 2010. The show started in November 2001 on Xfm, and aired in weekly periods for months at a time throughout 2002, 2003, 2004, and mid-2005. In November 2005");
	msg m(msg_str);
	int pid;
	unsigned short port;
	srand(time(NULL));

	cout << "send message:" << endl << m.str() << endl;

	port = (rand() % 3000) + 4000;
	pid = fork();
	BOOST_REQUIRE(pid >= 0);

	if(pid == 0) {
		/* client setup */
		struct sockaddr_in sin;
		int s,sent;
		proto_chat::send_status_t status;
		
		try{
		sock::host_sin("localhost", port, &sin);

		sleep(1);
		s = sock::tcp_cx(&sin);
	
		auto_ptr<istream> istrm = m.istrm();
	
		status = proto_chat::send_msg(s, *istrm, sent);
	
		BOOST_REQUIRE_EQUAL(status, proto_chat::SND_MSG_MAX);
		BOOST_REQUIRE_EQUAL(sent, 512);
		close(s);
		}
		catch(int e) {
			cout << strerror(e) << endl;
			close(s);
			exit(1);
		}
		exit(0);
	}
	else {
		/* host setup */
		msg from_child;
		struct sockaddr_in hsin;
		int hsocket, new_s;
		socklen_t hsin_len = sizeof(hsin);
		sock::passive_sin(port, &hsin, &hsin_len); 	
		
		cout << "create passive sock" << endl;

		try {

		hsocket = sock::passive_tcp_sock(&hsin, &hsin_len, 10);
		
		auto_ptr<std::ostream> ostrm = from_child.ostrm();
		int recd = 0;
		proto_chat::recv_status_t status = proto_chat::RCV_CONT;;

		cout << "wait for accept" << endl;
		cout.flush();

		BOOST_REQUIRE( (new_s = accept(hsocket, (struct sockaddr *)&hsin, &hsin_len) ) >= 0);
		
		cout << "accepted cx" << endl;

		cout << "set non blocking" << endl;
		
		sock::set_nonblocking(new_s);

		char code = 0;
		status = proto_chat::RCV_CONT;
		while(status == proto_chat::RCV_CONT) {
			cout << ',';
			status = proto_chat::recv_opcode(new_s, code);
			sleep(0.1);
		}

		BOOST_REQUIRE_EQUAL(status, proto_chat::RCV_DONE);
		BOOST_REQUIRE_EQUAL(code, proto_chat::MSG_START);

		status = proto_chat::RCV_CONT;
		while(status == proto_chat::RCV_CONT) {
			cout << '.';
			status = proto_chat::recv_msg(new_s, *ostrm, recd);
			sleep(0.1);
		}

		cout << endl << "got message from child: " << endl << from_child.str() << endl;
		
		close(new_s);
		close(hsocket);

		BOOST_REQUIRE_EQUAL(status,  proto_chat::RCV_DONE);
		BOOST_REQUIRE_EQUAL(recd, proto_chat::MAX_MSG_LEN);
		
		} /*try*/
		catch(int e) {
			cout << strerror(e) << endl;
			close(new_s);
			close(hsocket);

			BOOST_REQUIRE(false);
		}
	}

	
}

BOOST_AUTO_TEST_CASE(send_recv_msg_id) {
	uint32_t msgid;
	int pid;
	unsigned short port;
	srand(time(NULL));
	
	msgid = rand() % 0xffffffff;
	cout << "send message id:" << msgid << endl;

	port = (rand() % 3000) + 4000;
	pid = fork();
	BOOST_REQUIRE(pid >= 0);

	if(pid == 0) {
		/* client setup */
		struct sockaddr_in sin;
		int s;
		proto_chat::send_status_t status;
		
		try{
		sock::host_sin("localhost", port, &sin);

		sleep(2);
		s = sock::tcp_cx(&sin);
	
		status = proto_chat::send_msg_id(s, msgid);
	
		BOOST_REQUIRE_EQUAL(status, proto_chat::SND_DONE);	
		close(s);
		}
		catch(int e) {
			cout << strerror(e) << endl;
			close(s);
			exit(1);
		}
		exit(0);
	}
	else {
		/* host setup */
		uint32_t from_child;
		struct sockaddr_in hsin;
		int hsocket, new_s;
		socklen_t hsin_len = sizeof(hsin);
		sock::passive_sin(port, &hsin, &hsin_len); 	
		
		cout << "create passive sock" << endl;

		try {

		hsocket = sock::passive_tcp_sock(&hsin, &hsin_len, 10);
		
		int recd = 0;
		proto_chat::recv_status_t status = proto_chat::RCV_CONT;

		cout << "wait for accept" << endl;
		cout.flush();

		BOOST_REQUIRE( (new_s = accept(hsocket, (struct sockaddr *)&hsin, &hsin_len) ) >= 0);
		
		cout << "accepted cx" << endl;

		cout << "set non blocking" << endl;
		
		sock::set_nonblocking(new_s);

		while(status == proto_chat::RCV_CONT) {
			cout << '.';
			status = proto_chat::recv_msg_id(new_s, from_child, recd);
			sleep(0.1);
		}

		cout << endl << "got msg id from child: " << from_child << endl;
		
		close(new_s);
		close(hsocket);

		BOOST_REQUIRE_EQUAL(from_child, msgid);

		} /*try*/
		catch(int e) {
			cout << strerror(e) << endl;
			close(new_s);
			close(hsocket);

			BOOST_REQUIRE(false);
		}
	}

	
}