#ifndef PROTO_COORD_H
#define PROTO_COORD_H

extern "C" {
#include <stdint.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

}

namespace net01 {

namespace proto_coord {
	
	static const char REQ_FIND = 0x91;
	static const char RPL_FIND = 0x92;
	static const char REQ_START = 0x93;
	static const char RPL_START = 0x94;
	static const char REQ_TERM = 0x95;
	
	static const char RPL_ERR = 0xe0;  /* general server error */

	static const char op_codes[] = {REQ_FIND, RPL_FIND, REQ_START, RPL_START, RPL_ERR};

	static const int MAX_SESS_NAME_LEN = 8;
	static const int TERM_TOKEN_LEN = 32;

	static const char MIN_ASCII_BYTE = 0x61;
	static const char MAX_ASCII_BYTE = 0x7a;

	enum send_status_t { SND_DONE, SND_CLOSE, SND_ERR };
	enum recv_status_t { RCV_DONE, RCV_CLOSE, RCV_ERR };

	send_status_t send_sess_op(int socket, const struct sockaddr_in *sin, socklen_t sinlen, const char *name, int namelen, char opcode);

	//send_status_t send_term(int socket, const struct sockaddr_in *sin, socklen_t sinlen, const char *token, int toklen);

	recv_status_t recv_sess_name(const char *msg, int msglen, char *name, int &namelen);
	recv_status_t recv_sess_port(const char *msg, int msglen, unsigned short &port);
	
	bool check_bad_ascii(char c);

	bool check_ascii_buf(const char *buf, int buflen);

}; /* proto_coord */

}; /* net01 */
#endif /* PROTO_COORD_H */
