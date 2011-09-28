#include "coordinator.h"
#include "proto_coord.h"
#include "session_server.h"

#include <iostream>

using namespace std;
using namespace net01;

coordinator::coordinator(int udp_socket) : m_socket(udp_socket) {}

coordinator::~coordinator() {
	close(m_socket);
}

int coordinator::loop() {
	int i;
	struct sockaddr_in sin;
	socklen_t sinlen = sizeof(sin);
	char buf[512];
	int msg_len = 0;

	on_msg_status_t status;

	for(i = 0; ; i++) {
		if( (msg_len = recvfrom(m_socket, buf, sizeof(buf), 0, (sockaddr *)&sin, &sinlen)) < 0) {
			throw errno;
		}
	
		status = on_msg(buf, msg_len, sin, sinlen);
			
		msg_len = 0;
		if(status == OM_STOP) {
			break;
		}
	}	

	return i;
}

coordinator::on_msg_status_t coordinator::on_msg(const char *buf, int len, struct sockaddr_in &sin, socklen_t &sinlen) {
	int i,sent;

	cout << "received packet from " << inet_ntoa(sin.sin_addr) << ":" << ntohs(sin.sin_port) << endl;
	/*cout << "msg: ";
	for(i = 0; i < len; i++) {
		cout << buf[i];
	}
	cout << endl;*/

	assert(len > 1);

	switch(buf[0]) {
		case proto_coord::REQ_START:
			char reply = start_session(buf, len, sin, sinlen);
			if( (sent = sendto(m_socket, &reply, 1, 0, (const sockaddr *) &sin, sinlen ) ) < 0) {
				throw errno;
			}
			break;
		
		default:
			cout << "unknown request " << hex << (int) buf[0] << endl;
			break;
		
	}

	return OM_OK;
}

char coordinator::start_session(const char *buf, int len, struct sockaddr_in &sin, socklen_t &sinlen) {
	char name[9];
	int namelen,pid;
	string sess_name;
	session sess;
	struct sockaddr_in sess_sin;
	int socket;
	socklen_t sess_sin_len = sizeof(sess_sin);
	session_server *ss;
	
	proto_coord::recv_sess_name(buf, len, name, namelen);
	
	assert(namelen <= 8);

	name[namelen] = 0x00;
	sess_name = name;

	cout << "start session " << sess_name << endl;

	if(m_sessions.find(sess_name) != m_sessions.end() ) {
		cout << "session already exists" << endl;
		return proto_chat::RPL_ERR;
	}

	sock::passive_sin(0, &sess_sin, &sess_sin_len); 	
	socket = sock::passive_tcp_sock(&sess_sin, &sess_sin_len, 20);
	ss = new session_server(sess_name, socket);

	sess.port = ntohs(sess_sin.sin_port);

	sess.pid = fork();
	
	if(sess.pid < 0) {
		cout << "could not create session" << endl;
		delete ss;
		close(socket);

		return proto_chat::RPL_ERR;	
	}
	else if(sess.pid == 0) {
		cout << "created session server " << sess_name << " on port " << sess.port << endl;
	
		try {
			ss->select_fds(0);
		} /*try*/
		catch(int e) {
			cout << "exception from session server: " << strerror(e) << endl;
		}
	
		cout << "close session server " << sess_name << endl;
		delete ss;
		close(socket);
	}
	else {
		m_sessions[sess_name] = sess;
		cout << "created session process for " << sess_name << " at " << sess.pid << endl;
		return proto_coord::RPL_START;
	}
}