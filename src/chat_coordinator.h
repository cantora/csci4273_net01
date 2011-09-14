#ifndef CHAT_COORDINATOR_H
#define CHAT_COORDINATOR_H

#include <cstdio>
#include "listener.h" 
/*
 * 
 *	coordinator:
 *		-starts by printing port number on stdout
 *		-talks to clients via UDP
 *		-creates session servers
 *		-operations:
 *			Start NAME
 *			Find NAME
 *			Terminate NAME
 *		-Start: print error if  session with NAME is already open. otherwise, create a TCP socket and fork/exec a session server to manage the session
 *		-Find: check db of sessions for session NAME. if found, return the address of that session server.
 *		-Terminate: terminate session NAME. can only be invoked by the sesson server that owns NAME.
 *
 */

namespace net01 {

class chat_coordinator : public listener {
	public:
		chat_coordinator(int socket, int backlog);
		~chat_coordinator();

		
	private:
		virtual on_accept_status_t on_accept(int socket, struct sockaddr_in sin);

}; /* chat_coordinator */

}; /* net01 */

#endif /* CHAT_COORDINATOR_H */