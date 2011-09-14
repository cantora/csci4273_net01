#define BOOST_TEST_MODULE "net01:socket"
#include <boost/test/unit_test.hpp>

#include <stdio.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h> 

#define SERVER_PORT 5432 
#define MAX_PENDING 5 
#define MAX_LINE 256 

BOOST_AUTO_TEST_CASE(tcplisten)
{ 
	struct sockaddr_in sin; 
	char buf[MAX_LINE]; 
	socklen_t len; 
	int s, new_s; 

	/* build address data structure */ 
	bzero((char *)&sin, sizeof(sin)); 
	sin.sin_family = AF_INET; 
	sin.sin_addr.s_addr = INADDR_ANY; 
	sin.sin_port = htons(SERVER_PORT); 

	/* setup passive open */ 
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) { 
		perror("socket"); 
		BOOST_REQUIRE(false); 
	} 

	if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) { 
		perror("bind"); 
		BOOST_REQUIRE(false); 
	} 

	listen(s, MAX_PENDING); 
	/* wait for connection, then receive and print text */ 

	if ((new_s = accept(s, (struct sockaddr *)&sin, &len)) < 0) { 
		perror("accept"); 
		BOOST_REQUIRE(false); 
	}
	fprintf(stderr, "accepted...\n"); 
	if(len = recv(new_s, buf, sizeof(buf), 0)) 
		fputs(buf, stdout); 

	fprintf(stderr, "close...\n");
	close(new_s); 
	close(s);
}

BOOST_AUTO_TEST_CASE(tcpsend)
{ 
	FILE *fp;
	struct hostent *hp;
	struct sockaddr_in sin;
	char *host = "localhost";
	char buf[MAX_LINE];
	int s;
	int len;
	 
	/* translate host name into peer’s IP address */ 
	hp = gethostbyname(host); 
	if (!hp) { 
		fprintf(stderr, "unknown host: %s\n", host); 
		BOOST_REQUIRE(false);
	}

	/* build address data structure */ 
	bzero((char *)&sin, sizeof(sin)); 
	sin.sin_family = AF_INET; 
	bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length); 
	sin.sin_port = htons(SERVER_PORT); 
	/* active open */ 
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) { 
		perror("socket"); 
		BOOST_REQUIRE(false); 
	} 
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) { 
		perror("connect"); 
		close(s); 
		BOOST_REQUIRE(false); 
	} 
	
	while (fgets(buf, sizeof(buf), stdin)) { 
		buf[MAX_LINE-1] = '\0'; 
		len = strlen(buf) + 1; 
		send(s, buf, len, 0); 
	} 
} 


