#define BOOST_TEST_MODULE "net01:chat_coord_test"
#include <boost/test/unit_test.hpp>

#include <cerrno>

#include "chat_coordinator.h"

using namespace net01;

BOOST_AUTO_TEST_CASE(bad_socket)
{
	int r;
	int caught = 0;
	chat_coordinator coord(r, 1);

	srand ( time(NULL) );
	r = rand() % 500 + 1;	

	try {	
		coord.listen_and_accept(0);
	}
	catch(int e) {
		printf("caught: %s (%d)\n", strerror(e), e );
		BOOST_REQUIRE_EQUAL(EBADF, e);
		caught = 1;
	}

	BOOST_REQUIRE_EQUAL(caught, 1);	
	
}

BOOST_AUTO_TEST_CASE(accept_one) {
	struct sockaddr_in sin; 
	socklen_t len; 
	int s;
	chat_coordinator *coord;
	int accepted = 0;

	/* build address data structure */ 
	bzero((char *)&sin, sizeof(sin)); 
	sin.sin_family = AF_INET; 
	sin.sin_addr.s_addr = INADDR_ANY; 
	sin.sin_port = htons(5432); 

	/* setup passive open */ 
	if ((s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) { 
		perror("socket"); 
		BOOST_REQUIRE(false); 
	} 

	if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) { 
		perror("bind"); 
		BOOST_REQUIRE(false); 
	} 

	printf("socket bound to port %d...\n", ntohs(sin.sin_port));
	fflush(stdout);

	coord = new chat_coordinator(s, 20);
		
	accepted = coord->listen_and_accept(1);
	
	printf("accepted %d cx's\n", accepted);

	BOOST_REQUIRE_EQUAL(1, accepted);

	printf("socket bound to port %d...\n", ntohs(sin.sin_port));
	fflush(stdout);

	coord = new chat_coordinator(s, 20);
		
	accepted = coord->listen_and_accept(1);
	
	printf("accepted %d cx's\n", accepted);

	BOOST_REQUIRE_EQUAL(1, accepted);
}