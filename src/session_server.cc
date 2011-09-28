#include "session_server.h"

#include <cassert>

extern "C" {
#include <arpa/inet.h>
}

using namespace std;
using namespace net01;


/*
 * read in requests/messages from clients.
 * does not reply in this function. replies are in process
 */
selectah::selectah_status_t session_server::consume(int socket) {
	string username;
	socket_to_username(socket, username);
	
	assert(m_clients[socket].state == client::CLI_OK);
		
	/* if we arent currently doing anything useful */
	if(m_clients[socket].request == proto_chat::NOOP) {
		/* read in requests until no data
		 * or until non-noop
		 */
		m_clients[socket].state = get_request(socket, m_clients[socket].request);
	}

	/* return if there is nothing to do or something wrong with client */
	if((m_clients[socket].request == proto_chat::NOOP) || (m_clients[socket].state != client::CLI_OK) ) {
		return SEL_OK;
	}
	
	client::state_t status;
	switch(m_clients[socket].request) {
		case proto_chat::MSG_START :
			/* we have an incoming msg 
			 * read until no bytes left on socket
	 		 * or read until we have read in 1 msg.
			 */
			consume_msg(socket);
			break;
	
		case proto_chat::REQ_GET_NEXT : 
			cout << "receive msg id for get_next from " << username << endl;
			m_clients[socket].state = receive_msg_id(socket);
			break;

		case proto_chat::REQ_GET_ALL  :
			/* if the current msg_id is valid, we are in the process of
 			 * sending messages to the client
			 */
			if(m_clients[socket].msg_id == client::NULL_MSG_ID) {
				/* this is a fresh request for all messages,
 				 * so read in the msg id argument
 				 */
				cout << "receive msg id for get_all from " << username << endl;
				m_clients[socket].state = receive_msg_id(socket);
			}
			break;

		default :
			/* do nothing */;
	} /* switch */

	return SEL_OK;
}

session_server::client::state_t session_server::receive_msg_id(int socket) {
	int recd = 0;
	proto_chat::recv_status_t status = proto_chat::RCV_CONT;
	string username;
	socket_to_username(socket, username);
	
	while(status == proto_chat::RCV_CONT) {
		status = proto_chat::recv_msg_id(socket, m_clients[socket].msg_id, recd);
	}

	if(status == proto_chat::RCV_CLOSE) {
		return client::CLI_CLOSE;
	}

	cout << "received message id " << m_clients[socket].msg_id << " from " << username << endl;
		
	return client::CLI_OK;
}

selectah::selectah_status_t session_server::process(int n) {
	
	map<int, client>::iterator cli, current;

	for(cli = m_clients.begin(); cli != m_clients.end(); /* nothing */) {
		current = cli++;
		respond_to_client(current->first);
		cout.flush();
	}

	//cout << ".";
	cout.flush();

	return SEL_OK;
}

selectah::selectah_status_t session_server::respond_to_client(int socket) {
	string username;
	socket_to_username(socket, username);
	
	if(m_clients[socket].state != client::CLI_OK) {
		switch(m_clients[socket].state) {
			case client::CLI_ERR : /* keep going... */
			case client::CLI_ERR_AND_CLOSE :
				/* we dont care about the status, we are about to close */
				cout << "reply with error to " << username << endl;

				proto_chat::send_opcode(socket, proto_chat::RPL_ERR);
				/* keep going... */
			case client::CLI_CLOSE :
				close_client(socket);
				break; 

			default : 
				assert(false); /* freak out! */
		}

		return SEL_OK;
	}

	switch(m_clients[socket].request){ 
		case proto_chat::MSG_START : 
			/* we have an incoming msg that is being
			 * read in consume() 
			 */
			break;

		case proto_chat::MSG_END : 
			assert(false); /* this shouldnt happen */
			break;

		case proto_chat::REQ_GET_NEXT : 
			handle_get_next(socket);
			break;

		case proto_chat::REQ_GET_ALL :
			handle_get_all(socket);
			break;

		case proto_chat::REQ_EXIT :
			close_client(socket);
			break;
	
		case proto_chat::NOOP :
			break;

		default:
			assert(false); /* !!! */
			
	} /* switch */	

	return SEL_OK;	
}

void session_server::close_client(int socket) {
	string username;
	socket_to_username(socket, username);
	
	cout << "closed connection to " << username << endl;

	m_clients.erase(socket);
	close_socket(socket);
}

void session_server::handle_get_next(int socket) {
	if(m_clients[socket].msg_id >= (m_msg_table.size()) ) {
		if(proto_chat::send_opcode(socket, proto_chat::RPL_NO_NEW_MSG) == proto_chat::SND_CLOSE) {
			m_clients[socket].state = client::CLI_CLOSE;
		}
	}
	else {
		m_clients[socket].state = rpl_get_next(socket);
	}

	if(m_clients[socket].state == client::CLI_CLOSE) {
		close_client(socket);
	}
	else {
		m_clients[socket].request = proto_chat::NOOP;
		m_clients[socket].msg_id = client::NULL_MSG_ID;	
	}	
}

void session_server::handle_get_all(int socket) {
	string username;
	socket_to_username(socket, username);
	
	if(m_clients[socket].msg_id >= (m_msg_table.size()) ) {
		/* dont send an op code response because we 
 		 * have been incrementing automatically
		 */
		cout << "finished sending all messages to " << username << endl;

		m_clients[socket].request = proto_chat::NOOP;
		m_clients[socket].msg_id = client::NULL_MSG_ID;	
		return;
	}

	/* we have not incremented past the last message yet,
	 * so send the next message and increment if the 
	 * cx is not closed
	 */
	m_clients[socket].state = rpl_get_next(socket);
	
	if(m_clients[socket].state == client::CLI_CLOSE) {
		close_client(socket);
	}
	else {
		m_clients[socket].msg_id++;
	}
}

session_server::client::state_t session_server::rpl_get_next(int socket) {
	proto_chat::send_status_t status;
	int sent = 0;
	auto_ptr<istream> istrm;

	string username;
	socket_to_username(socket, username);
	
	/* first check that msg id sent is in a valid range */
	assert(m_clients[socket].msg_id < (m_msg_table.size()) );
	assert(m_clients[socket].msg_id >= 0);

	/* if its in the right range, send back msg at index msg_id + 1,
	 * but really, since message ids start at 1, we actually reply with
 	 * the id at msg_id.
	 * e.g. they have msg 0 (none at all) so we give them 1 (index 0) 
	 * and tell them we just gave them msg 1
	 */
	istrm = m_msg_table.at(m_clients[socket].msg_id).m.istrm();

	status = proto_chat::send_msg(socket, *istrm, m_clients[socket].msg_id + 1, sent);
	if(status == proto_chat::SND_CLOSE) {
		return client::CLI_CLOSE;
	}
	else if(status == proto_chat::SND_MSG_MAX) {
		assert(false); /* this shouldnt happen */
	}

	assert(status == proto_chat::SND_DONE);

	cout << "sent msg " << m_clients[socket].msg_id + 1 << " to " << username << endl;
	return client::CLI_OK;
}

selectah::selectah_status_t session_server::consume_msg(int socket) { 
	string username;
	socket_to_username(socket, username);
			
	if(m_clients[socket].in_msg_ostrm.get() == 0) {
		cout << "start receiving new message from  " << username << endl;
		m_clients[socket].reset_in_msg();
	}
	else {
		cout << "continue reading message (recd " << m_clients[socket].in_msg_recd << ")" << endl;
	}
	
	proto_chat::recv_status_t status;
	uint32_t dummy;
	status = proto_chat::recv_msg(socket, *(m_clients[socket].in_msg_ostrm), dummy, m_clients[socket].in_msg_recd);
		
	/* if we have the msg fully read in, copy it to the table */
	if(status == proto_chat::RCV_DONE) {
		m_msg_table.push_back(chat_msg(username, m_clients[socket].in_msg));
		const string str = m_msg_table.back().m.str();

		cout << "received (" << m_clients[socket].in_msg_recd << ") message from " << username << ": " << str << " (" << str.size() << ") " << endl;
		
		m_clients[socket].in_msg_ostrm.release();
		m_clients[socket].state = client::CLI_OK;
		m_clients[socket].request = proto_chat::NOOP;		
	}
	else if(status == proto_chat::RCV_CLOSE) {
		m_clients[socket].state = client::CLI_CLOSE;
		m_clients[socket].request = proto_chat::REQ_EXIT;
	}
	else if(status == proto_chat::RCV_MSG_MAX) {
		cout << "message too big from " << username << endl;

		m_clients[socket].state = client::CLI_ERR_AND_CLOSE;
		m_clients[socket].request = proto_chat::REQ_EXIT;
	}
	else {
		assert(status == proto_chat::RCV_CONT);
		cout << "read " << m_clients[socket].in_msg_recd << " bytes from message" << endl;
		cout << "message so far: \"" << m_clients[socket].in_msg.str() << '"' << endl;
	}

	return SEL_OK;
}

session_server::client::state_t session_server::get_request(int socket, char &opcode) {
	//char code = proto_chat::NOOP;
	proto_chat::recv_status_t status;

	while(opcode == proto_chat::NOOP) {
		if( (status = proto_chat::recv_opcode(socket, opcode)) != proto_chat::RCV_DONE) {
			if(status == proto_chat::RCV_CLOSE) {
				return client::CLI_CLOSE;
			}
			else if(status == proto_chat::RCV_ERR) {
				return client::CLI_ERR;
			}
			else if(status == proto_chat::RCV_CONT) {
				/* there are no new requests, so use a noop as placeholder */
				opcode = proto_chat::NOOP;
				break;
			}
			else { /* freak out! */
				assert(false);
			}
		}
	}

	return client::CLI_OK;	
}

void session_server::socket_to_username(int socket, string &username) const {
	map<int, sock::sock_info_t>::const_iterator cli_sinfo = sock_table()->find(socket);

	assert(cli_sinfo != sock_table()->end() );
	
	std::ostringstream strm;		
	strm << inet_ntoa(cli_sinfo->second.sin.sin_addr) << ":";
	
	unsigned short port = ntohs(cli_sinfo->second.sin.sin_port);

	strm << port;

	username = strm.str();
}