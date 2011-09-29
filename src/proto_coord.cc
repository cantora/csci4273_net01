#include "proto_coord.h"

#include <cerrno>
#include <iostream>
#include <cassert>
#include <cstring>

using namespace std;
using namespace net01;

proto_coord::send_status_t proto_coord::send_sess_op(int socket, const struct sockaddr_in *sin, socklen_t sinlen, const char* name, int namelen, char opcode) {
	char buf[9];
	int sent;

	assert(namelen <= MAX_SESS_NAME_LEN);
	assert(namelen > 0);
	
	if(!check_ascii_buf(name, namelen) ) {
		return SND_ERR;
	}
	
	buf[0] = opcode;
	for(int i = 0; i < namelen; i++) {
		buf[1+i] = name[i];
	}

	if( (sent = sendto(socket, buf, 1+namelen, 0, (const sockaddr *) sin, sinlen ) ) < 0) {
		throw errno;
	}

	if(sent == 0) {
		return SND_CLOSE;
	}

	assert(sent == 1+namelen);

	return SND_DONE;
}

/*proto_coord::send_status_t proto_coord::send_term(int socket, const struct sockaddr_in *sin, socklen_t sinlen, const char *token, int toklen) {
	char buf[9];
	int sent;

	assert(namelen <= MAX_SESS_NAME_LEN);
	assert(namelen > 0);
	
	if(!check_ascii_buf(name, namelen) ) {
		return SND_ERR;
	}
	
	buf[0] = opcode;
	for(int i = 0; i < namelen; i++) {
		buf[1+i] = name[i];
	}

	if( (sent = sendto(socket, buf, 1+namelen, 0, (const sockaddr *) sin, sinlen ) ) < 0) {
		throw errno;
	}

	if(sent == 0) {
		return SND_CLOSE;
	}

	assert(sent == 1+namelen);

	return SND_DONE;
}*/

proto_coord::recv_status_t proto_coord::recv_sess_name(const char *msg, int msglen, char *name, int &namelen) {
	assert(msg != NULL);
	assert(name != NULL);
	assert(msglen > 1);
	
	if(msglen > proto_coord::MAX_SESS_NAME_LEN + 1) {
		msglen = proto_coord::MAX_SESS_NAME_LEN + 1;
	}

	namelen = msglen-1;
	memcpy(name, msg+1, namelen); 
	
	//for(namelen = 0; namelen+1 < msglen; namelen++) {
	//	name[namelen] = msg[namelen+1];
	//} 	
	
	assert(namelen <= MAX_SESS_NAME_LEN);

	return RCV_DONE;
}

proto_coord::recv_status_t proto_coord::recv_sess_port(const char *msg, int msglen, uint16_t  &port) {
	assert(msg != NULL);
	assert(msglen > 1);
	
	port = 0;
	if(msglen < 3) {
		return RCV_ERR;
	}

	memcpy((void *)&port, msg+1, sizeof(port));

	port = ntohs(port);

	return RCV_DONE;
}

bool proto_coord::check_ascii_buf(const char *buf, int buflen) {
	for(int i = 0; i < buflen; i++) {
		if(!check_bad_ascii(buf[i]) ) {
			return false;
		}
	}

	return true;
}

bool proto_coord::check_bad_ascii(char c) {
	return ( (c <= MAX_ASCII_BYTE) && (c >= MIN_ASCII_BYTE) );
}
#if 0
int proto_coord::gsub_bad_ascii(char *buf, int size) {
	int count;

	assert(buf != NULL);

	for(int i = 0; i < size; i++) {
		count += sub_bad_ascii(buf[i]);
	}
		
	return count;
}

int proto_coord::sub_bad_ascii(char &c) {
	if( (c < MIN_ASCII_BYTE) || (c > MAX_ASCII_BYTE) ) {
		c = BAD_ASCII_BYTE;
		return 1;
	}
	
	return 0;
}

bool proto_coord::test_byte_for_opcode(char code) {
	bool found = false;
	for(int i = 0; i < sizeof(proto_coord::op_codes); i++) {
		if(code == proto_coord::op_codes[i]) {
			found = true;
			break;
		}
	}
	
	return found;
}

#endif