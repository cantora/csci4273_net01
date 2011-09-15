#include "selectah.h"

using namespace net01;

selectah::selectah() {

	m_timeout.tv_sec = 60;
	m_timeout.tv_usec = 0;
}

selectah::~selectah() {}

/*
 *
 */
int selectah::add_rfd(int fd) {
	m_rfds.insert(fd);
}

int selectah::rfd_set_size() const {
	return m_rfds.size();
}
/*
 * n: n iterations
 * 		n < 1 => infinite loop
 *
 * return value: the number of iterations
 */
int selectah::select_fds(int n) {
	int i, n_fds;
	fd_set rfds;
	struct timeval timeout;
	selectah_status_t status = SEL_OK;

	if(m_rfds.empty() )
		return 0;
 
	for(i = 0; (n < 1) || (i < n); i++) {
		rfd_set(&rfds);
		timeout = m_timeout;

		n_fds = select(high_rfd()+1, &rfds, (fd_set *) 0, (fd_set *) 0, &timeout);

		if(n_fds < 1) {
			throw errno;
		}
		else if(n_fds == 0) {
			continue;
		}
		
		/* if we are here we have fds to look at */
		status = on_rfds(&rfds);
	}

	return i;
}


void selectah::rfd_set(fd_set *result) const {
	std::set<int>::iterator it;

	FD_ZERO(result);

	for(it = m_rfds.begin(); it != m_rfds.end(); it++) {
		FD_SET(*it, result);
	}
}

int selectah::high_rfd() const {
	if(m_rfds.size() < 1) {
		return 0;
	}

	return *std::max_element(m_rfds.begin(), m_rfds.end() );
}

