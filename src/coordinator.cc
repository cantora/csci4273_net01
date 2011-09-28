#include "coordinator.h"

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
	int i;

	cout << "received packet from " << inet_ntoa(sin.sin_addr) << ":" << ntohs(sin.sin_port) << endl;
	cout << "msg: ";
	for(i = 0; i < len; i++) {
		cout << buf[i];
	}
	cout << endl;

	return OM_OK;
}