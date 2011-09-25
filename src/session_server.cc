#include "session_server.h"

using namespace std;
using namespace net01;

/*
 * read in requests/messages from clients.
 * does not reply in this function. replies are in process
 */
selectah::selectah_status_t session_server::consume(int socket) {
	assert(m_clients[socket].state == CLI_OK);
	
	/* if we arent currently reading in a message */
	if(m_clients[socket].in_msg_ostrm.get() == 0) {
		/* read in any non-stateful requests */
		queue_requests(socket);
	}

	/* at this point we may have a incoming msg 
	 * if so, read until no bytes left on socket
 	 * or read until we have read in 1 msg.
	 */
	if(m_clients[socket].requests.front() == proto_chat::MSG_SUBMIT) {
		return consume_msg(socket);
	}

	return SEL_OK;
}

selectah::selectah_status_t consume_msg(int socket) { 
	if(m_clients[socket].in_msg_ostrm.get() == 0) {
		m_clients[socket].in_msg = new msg();
		m_clients[socket].in_msg_ostrm = m_clients[socket].in_msg->ostrm();
	}
		
	proto_chat::recv_status_t status;
	int recd; status = proto_chat::recv_msg(socket, *m_clients[socket].in_msg_ostrm, recd);
		
	/* if we have the msg fully read in, copy it to the table */
	if(status == RCV_DONE) {
		string username;
		socket_to_username(socket, username);
		
		m_msg_table.push_back(username, *m_clients[socket].in_msg);
		m_clients[socket].in_msg.release();
		m_clients[socket].in_msg_ostrm.release();
	}
	else if(status == RCV_CLOSE) {
		m_clients[socket].state = CLI_CLOSE;
	}
	else if(status == RCV_MSG_MAX) {
		m_clients[socket].state = CLI_ERR_AND_CLOSE;
	}

	return SEL_OK;
}

selectah::selectah_status_t session_server::queue_requests(socket) {
	
}

void session_server::socket_to_username(int socket, string username) const {
	map<int, sock::sock_info_t>::const_iterator cli_sinfo = sock_table->find(socket);

	assert(cli_sinfo != sock_table->end() );
			
	username += inet_ntoa(cli_sinfo->sin.sin_addr);
	username += ":";
	username.append(ntohs(cli_sinfo->sin.sin_port) );

}