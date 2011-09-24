#include "proto_chat.h"

#include <cerrno>

extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
}

using namespace std;
using namespace net01;

/*
 * send a msg on socket using the istream pointer from msg.istrm().
 * stops reading from msg_istrm when it has sent max_msg_len bytes
 * return the number of bytes sent
 */
int proto_chat::send_msg(int socket, auto_ptr<istream> &msg_istrm, int &sent) {
#error "fix this to return send status and fill &sent"
	char buf[32];
	int sent, read, total_sent;
	
	total_sent = 0;
	assert(msg_istrm->good());

	msg_istrm->readsome(buf, sizeof(buf) );
	
	read = msg_istrm->gcount();

	if(read <= 0 ) {
		return 0;
	}

	if( (sent = send(socket, &REQ_SUBMIT, 1, 0)) != 1) {
		throw errno;
	}
	total_sent += sent;

	while(read > 0) {

		if((total_sent - 1) + read > MAX_MSG_LEN) {    /* truncate amount to send if we will
			read = (MAX_MSG_LEN - (total_sent - 1) );   * exceed max message len
			assert(read >= 0);						    */
	
			if(read <= 0) {
				break;
			}
		}		

		gsub_bad_ascii(buf, read);

		if( (sent = send(socket, buf, read, 0)) < 0) {
			throw errno;
		}
		
		assert(sent == read);
		total_sent += sent;

		if(total_sent - 1 >= MAX_MSG_LEN) { /* truncate message if its too long */
			assert((total_sent - 1) == MAX_MSG_LEN);
			break;
		}

		if(!msg_istrm->good()) { /* dont bother reading if error flags are set */
			break;
		}

		msg_istrm->readsome(buf, sizeof(buf) );
		read = msg_istrm->gcount();
	}

	if( (sent = send(socket, &REQ_SUBMIT_END, 1, 0)) != 1) {
		throw errno;
	}
	total_sent += sent;

	return total_sent;
}

/*
 * successive calls to this function with the same msg_ostrm object will 
 * continually read in a message from socket until MSG_SUBMIT_END is
 * encountered. each time this returns it will fill &received with the
 * amount read in. 
 * this function will return a completed status when it reads
 * MAX_MSG_LEN amount of bytes. 
 * the return value informs whether the message is completed or not.
 */
proto_chat::recv_status_t proto_chat::recv_msg(int socket, std::auto_ptr<std::ostream> &msg_ostrm, int &received) {
	char buf[64];
	int read;
	
	received = 0;
	assert(msg_ostrm->good());

	while(1) {
		if( (read = recvfrom(socket, buf, sizeof(buf), 0, NULL, NULL)) < 0) {
			if(errno == EAGAIN) {
				break;
			}

			throw errno;
		}
		
		if(read == 0) {
			return RCV_CLOSE;
		}
		
		if(received + read > MAX_MSG_LEN) {
			read = MAX_MSG_LEN - received;
			assert(read >= 0);
		}

		for(int i = 0; i < read; i++) {
			if(buf[i] == MSG_SUBMIT_END) {
				return RCV_DONE;
			}

			*ostrm << sub_bad_ascii(buf[i]); 
			received++;
		}
		
		if(received >= MAX_MSG_LEN) {
			assert(received == MAX_MSG_LEN);
			return RCV_MSG_MAX;
		}
	}

	return RCV_CONT;
}

int proto_chat::gsub_bad_ascii(char *buf, int size) {
	int count;

	assert(buf != NULL);

	for(int i = 0; i < size; i++) {
		count += sub_bad_ascii(buf[i]);
	}
		
	return count;
}

int proto_chat::sub_bad_ascii(char &c) {
	if( (c < MIN_ASCII_BYTE) || (c > MAX_ASCII_BYTE) ) {
		c = BAD_ASCII_BYTE;
		return 1;
	}
	
	return 0;
}