#include "sock.h"

#include <cassert>
#include <cstring>

extern "C" {
#include <fcntl.h>
}

using namespace net01;

/* 
 * pass a null terminated host name or ip with a 
 * port number and get output *addr back with
 * sockaddr_in structure
 * 
 * throws UNKNOWN_HOST if gethostbyname returns null
 */
void sock::host_sin(const char *host, unsigned short port, struct sockaddr_in *sin) {
	struct hostent *hp;

	assert(sin != NULL);

	hp = gethostbyname(host); 
	if (hp == NULL) { 
		throw sock::UNKNOWN_HOST;
	}

	memset(sin, 0, sizeof(struct sockaddr_in)); 
	sin->sin_family = AF_INET; 
	memcpy(&sin->sin_addr, hp->h_addr, hp->h_length); 
	sin->sin_port = htons(port); 
}

int sock::tcp_cx(const struct sockaddr_in *host_addr) {
	int s;
	
	assert(host_addr != NULL);

	/* active open */ 
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		throw errno;
	} 
	if (connect(s, (struct sockaddr *)host_addr, sizeof(struct sockaddr_in)) < 0) { 
		close(s); 
		throw errno;
	} 
	
	return s; 
}

void sock::passive_sin(unsigned short port, struct sockaddr_in *sin, socklen_t *sin_len) {
	memset(sin, 0, *sin_len);
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = INADDR_ANY;
		
	sin->sin_port = htons(port);

}

/*
 * create a passive socket, bind it to sin,
 * set it to listen mode, then return socket fd.
 * sin/sin_len parameters are output parameters
 * as well as input parameters, where the output
 * value is the the address structure returned by bind.
 */
int sock::passive_tcp_sock(struct sockaddr_in *sin, socklen_t *sin_len, int qlen) {
	int s;
	
	assert(sin != NULL);
	assert(sin_len != NULL);
	
	if( (s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP) ) < 0 ) {
		throw errno;
	}
	
	if (bind(s, (struct sockaddr *)sin, *sin_len) < 0) {
		throw errno;
	}
	
	if(listen(s, qlen) < 0) {
		throw errno;
	}

	return s;
}

int sock::bound_udp_sock(struct sockaddr_in *sin, socklen_t *sin_len) {
	int s;

	assert(sin != NULL);
	assert(sin_len != NULL);

	if( (s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) ) < 0) {
		throw errno;
	}

	if(bind(s, (const sockaddr *)sin, *sin_len) < 0) {
		throw errno;
	}

	return s;
}

void sock::set_nonblocking(int socket)
{
	int opts;

	opts = fcntl(socket,F_GETFL);
	if (opts < 0) {
		throw errno;
	}

	opts = (opts | O_NONBLOCK);
	if (fcntl(socket,F_SETFL,opts) < 0) {
		throw errno;
	}

	return;
}
