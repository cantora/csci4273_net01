#define BOOST_TEST_MODULE "net01:socket"
#include <boost/test/unit_test.hpp>

extern "C" {
#include <stdio.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h> 
#include <arpa/inet.h>
}

#include "sock.h"
#include <iostream>

using namespace std;
using namespace net01;

BOOST_AUTO_TEST_CASE(host_sin) {
	const char* hosts[] = {
		"127.0.0.1",
		"google.com",
		"localhost",
		"khalija.com"

	};

	const char *bad_host = "qwerqwerqasdfadf";
	bool caught = false;

	struct sockaddr_in sin;
	
	for(int i = 0; i < sizeof(hosts)/4; i++) {

		cout << hosts[i] << ": ";
		net01::sock::host_sin(hosts[i], 80, &sin);
		cout << inet_ntoa(sin.sin_addr) << endl;
	}

	try {
		sock::host_sin(bad_host, 80, &sin);		
	}
	catch(sock::sock_err e) {
		cout << "caught " << e << " for host " << bad_host << endl;
		BOOST_REQUIRE_EQUAL(sock::UNKNOWN_HOST, e);
		caught = true;		
	}

	BOOST_REQUIRE_EQUAL(caught, true);
}

BOOST_AUTO_TEST_CASE(tcpsend)
{ 
	char buf[1024];
	const char *host = "localhost";
	const char i_say[] = "hey hey hey\n";
	const char u_say[] = "go go go\n";

	std::string nc_cmd;

	nc_cmd.append("echo '");
	//nc_cmd.append(u_say, 0, sizeof(u_say) - 2);
	nc_cmd.append("' | nc -l -p 6543 &");

	int s;
	int len;
 	struct sockaddr_in sin;
	
	sock::host_sin(host, 6543, &sin);

	cout << "nc_cmd: " << nc_cmd << endl;

	//system(nc_cmd.c_str());
	sleep(1);
	
	s = sock::tcp_cx(&sin);

	len = strlen(buf) + 1; 

	if( send(s, buf, len, 0) < 0) {
		perror("send");
		BOOST_REQUIRE(false);
	}

	if( recvfrom(s, buf, 1023, 0, NULL, 0) < 0) {
		perror("recvfrom");
		BOOST_REQUIRE(false);
	}

	cout << "got response from nc (size = " << strlen(buf) << "):" << endl;
	for(int i = 0; i < strlen(buf); i++) { printf("0x%02x ", buf[i]); }; cout << endl;
	cout << "expected (size = " << sizeof(u_say) << "): " << endl;
	for(int i = 0; i < strlen(u_say); i++) { printf("0x%02x ", u_say[i]); }; cout << endl;

	BOOST_REQUIRE_EQUAL(0, memcmp(u_say, buf, sizeof(u_say) ) );

	close(s);
} 
