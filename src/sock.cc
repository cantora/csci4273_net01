#include "sock.h"

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