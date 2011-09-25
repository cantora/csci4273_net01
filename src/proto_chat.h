#ifndef PROTO_CHAT_H
#define PROTO_CHAT_H

#include "msg.h"

namespace net01 {

namespace proto_chat {
	
	
	
	static const char REQ_JOIN = 0x90;
	static const char REQ_GET_NEXT = 0x92;
	static const char REQ_GET_ALL = 0x93;
	static const char REQ_EXIT = 0x94;

	static const char MSG_SUBMIT = 0x91;
	static const char MSG_SUBMIT_END = 0x0d;

	static const char RPL_ERR = 0xe0;  /* general server error */
	static const char RPL_ERR_MSG_TOO_LONG = 0xe1;  /* message too long */

	static const int MAX_MSG_LEN = 510; /* hopefully will work out to 512 bytes w/ op codes */

	static const char MIN_ASCII_BYTE = 0x20;
	static const char MAX_ASCII_BYTE = 0x7e;
	
	static const char BAD_ASCII_BYTE = '?'; 
	
	enum send_status_t { SND_DONE, SND_CLOSE }
	enum recv_status_t { RCV_DONE, RCV_CONT, RCV_MSG_MAX, RCV_CLOSE }

	send_status_t send_msg(int socket, std::auto_ptr<std::istream> &msg_istrm, int &sent);
	recv_status_t recv_msg(int socket, std::auto_ptr<std::ostream> &msg_ostrm, int &received);

	char next_opcode(int socket, int &received);

	int gsub_bad_ascii(char *buf, int size);
	int sub_bad_ascii(char &c);

}; /* proto_chat */

}; /* net01 */
#endif /* PROTO_CHAT_H */

/*

const char op_join = 0x//"join";
	const char[] op_submit = "submit";
	const char[] op_get_next = "getnext";
	const char[] op_get_all = "getall";
	const char[] op_exit = "exit";
*/