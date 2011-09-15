#include "sess_srv.h"

using namespace net01;

sess_srv::sess_srv(int socket) {
	add_rfd(socket);
	
}

sess_srv::~sess_srv() {}

selectah::selectah_status_t sess_srv::on_rfds(const fd_set *rfd_set) {
	return SEL_OK;
}