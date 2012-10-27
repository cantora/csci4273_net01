#include "proto_chat.h"

#include <cerrno>
#include <iostream>
#include <cassert>

extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
}

using namespace std;
using namespace net01;

/*
 * send a msg on socket using the istream pointer from msg.istrm().
 * stops reading from msg_istrm when it has sent max_msg_len bytes
 * return the number of bytes sent
 */
proto_chat::send_status_t proto_chat::send_msg(int socket, istream &msg_istrm, uint32_t msg_id, int &sent) {
	char buf[32];
	int bytes, read;
	int prefix_amt = 0;
	send_status_t status = SND_DONE;
	
	sent = 0;
	assert(msg_istrm.good());

	msg_istrm.readsome(buf, sizeof(buf) );
	
	read = msg_istrm.gcount();

	assert(read > 0 );

	if( (bytes = send(socket, &MSG_START, 1, 0)) != 1) {
		if(bytes == 0) {
			return SND_CLOSE;
		}	
		assert(false);
	}
	sent += bytes;
	prefix_amt += bytes;

	if( send_msg_id(socket, msg_id) == SND_CLOSE) {
		return SND_CLOSE;
	} 
	sent += 4;
	prefix_amt += 4;

	while(read > 0) {
		//cout << "sent,read: " << sent << ", " << read << endl;

		/* truncate amount to send if we will
		 * exceed max message len
		 */
		if( ((sent - prefix_amt) + read) > MAX_MSG_LEN) { 
			//cout << endl << "sent: " << sent << endl << "read: " << read << endl;
			read = (MAX_MSG_LEN - (sent - prefix_amt) );
			//cout << "new read: " << read << endl;
			assert(read >= 0);
	
			if(read <= 0) { /* we hit the maximum allowed size */
				status = SND_MSG_MAX;
				break;
			}
		}		

		gsub_bad_ascii(buf, read);

		if( (bytes = send(socket, buf, read, 0)) < 0) {
			switch(errno) {
			case EPIPE : 
				return SND_CLOSE;
				break;
			default : 
				throw errno;
			}
		}
		
		if(bytes == 0) {
			assert(read != 0);
			status = SND_CLOSE;
			break;
		}

		assert(bytes == read);
		sent += bytes;
		//cout << "sent: " << sent << endl;
		if( (sent - prefix_amt) >= MAX_MSG_LEN) { /* truncate message if its too long */
			//cout << endl << sent << endl;
			assert((sent - prefix_amt) == MAX_MSG_LEN);
			status = SND_MSG_MAX;
			break;
		}

		if(!msg_istrm.good()) { /* dont bother reading if error flags are set */
			status = SND_DONE;
			break;
		}

		msg_istrm.readsome(buf, sizeof(buf) );
		read = msg_istrm.gcount();
	}

	if( (bytes = send(socket, &MSG_END, 1, 0)) != 1) {
		if(bytes == 0) {
			return SND_CLOSE;
		}
		assert(false);
	}
	sent += bytes;

	return status;
}

/*
 * successive calls to this function with the same msg_ostrm object will 
 * continually read in a message from socket until MSG_END is
 * encountered. each time this returns it will fill &received with the
 * initial value of received + amount read in. 
 * each successive call should be passed the same received variable
 * so it can track how many bytes it has read in. 
 * this function will return a completed status when it reads
 * MAX_MSG_LEN amount of bytes. 
 * the return value informs whether the message is completed or not.
 */
proto_chat::recv_status_t proto_chat::recv_msg(int socket, std::ostream &msg_ostrm, uint32_t &msg_id, int &received) {
	char buf[64];
	int read;
	bool next_char_is_msg_end = false;
	bool truncated = false;
	int prefix_size = sizeof(msg_id);
	
	//msg_id = 0;
	/* dont initialize received because it has the amount received the last time this funciton was called */
	//received = 0; 
	assert(msg_ostrm.good());

	if(received <= 0) {
		assert(received == 0);
		recv_status_t status = RCV_CONT;
		int msgid_recd = 0;

		while(status == RCV_CONT) {
			status = recv_msg_id(socket, msg_id, msgid_recd);
		}

		if(status == RCV_CLOSE) {
			return RCV_CLOSE;
		}
		assert(status == RCV_DONE);
		received += prefix_size;
	}

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
		
		if(received + read - prefix_size > MAX_MSG_LEN) {
			read = MAX_MSG_LEN + prefix_size - received;
			//cout << "truncated read: " << read << endl;
			
			assert(read >= 0);
			next_char_is_msg_end = (buf[read] == MSG_END);
			truncated = true;
		}

		for(int i = 0; i < read; i++) {
			if(buf[i] == MSG_END) {
				return RCV_DONE;
			}

			sub_bad_ascii(buf[i]);
			msg_ostrm << buf[i];
			received++;
		} /* for */
		
		//cout << "recd: " << received << endl;

		if(truncated) { //if( received >= MAX_MSG_LEN) {
			assert(received - prefix_size == MAX_MSG_LEN);
			
			if(next_char_is_msg_end) {
				return RCV_DONE;
			}
			else {
				return RCV_MSG_MAX;
			}
		}
	} /* while(1) */

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

bool proto_chat::test_byte_for_opcode(char code) {
	bool found = false;
	for(int i = 0; i < sizeof(proto_chat::op_codes); i++) {
		if(code == proto_chat::op_codes[i]) {
			found = true;
			break;
		}
	}
	
	return found;
}

proto_chat::recv_status_t proto_chat::recv_opcode(int socket, char &code) { 
	int read;
	code = 0;

	if( (read = recvfrom(socket, &code, 1, 0, NULL, NULL)) < 0) {
		if(errno == EAGAIN) {
			return RCV_CONT;
		}
		
		throw errno;
	}
	
	if(read == 0) {
		return RCV_CLOSE;
	}
	
	
	if(!test_byte_for_opcode(code)) {
		return RCV_ERR;
	}
	
	return RCV_DONE;
}

proto_chat::send_status_t proto_chat::send_opcode(int socket, char code) { 
	int sent;
	
	assert(test_byte_for_opcode(code) );
		
	if( (sent = send(socket, &code, 1, 0)) < 0) {
		switch(errno) {
		case EPIPE : 
			return SND_CLOSE;
			break;
		default : 
			throw errno;
		}
	}
	
	if(sent == 0) {
		return SND_CLOSE;
	}
	
	return SND_DONE;
}

proto_chat::send_status_t proto_chat::send_msg_id(int socket, uint32_t msg_id) {
	int sent;
	int nwo_msg_id = htonl(msg_id);
			
	if( (sent = send(socket, (char *) &nwo_msg_id, sizeof(nwo_msg_id), 0)) < 0) {
		switch(errno) {
		case EPIPE : 
			return SND_CLOSE;
			break;
		default : 
			throw errno;
		}
	}
	
	if(sent == 0) {
		return SND_CLOSE;
	}
	
	if(sent != sizeof(nwo_msg_id) ) {
		throw EIO; /* i expect to always be able to send 4 bytes */
	}

	return SND_DONE;		
}

proto_chat::recv_status_t proto_chat::recv_msg_id(int socket, uint32_t &msg_id, int &received) {
	int read;

	if( (read = recvfrom(socket, &msg_id+received, sizeof(msg_id), 0, NULL, NULL)) < 0) {
		if(errno == EAGAIN) {
			return RCV_CONT;
		}
		
		throw errno;
	}
	
	if(read == 0) {
		return RCV_CLOSE;
	}
	
	received += read;

	if(received < sizeof(msg_id) ) {
		return RCV_CONT;
	}

	assert(received == sizeof(msg_id) );

	msg_id = ntohl(msg_id);

	return RCV_DONE;	
}
