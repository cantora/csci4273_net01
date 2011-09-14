#include "chat_coordinator.h"

using namespace net01;

chat_coordinator::chat_coordinator(int socket, int backlog) : listener(socket, backlog) {}

chat_coordinator::~chat_coordinator() {}

chat_coordinator::on_accept_status_t chat_coordinator::on_accept(int socket, struct sockaddr_in sin) {
	printf("chat_coordinator.on_accept:\nsocket: %d\n", ntohs(sin.sin_port) ); //,sin.addr: %s\n"

	close(socket);
	return OAS_OK;
}
