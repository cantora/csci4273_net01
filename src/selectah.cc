#include "selectah.h"

#include <iostream>

using namespace std;
using namespace net01;

selectah::selectah() {

	m_timeout.tv_sec = 60;
	m_timeout.tv_usec = 0;
}

selectah::~selectah() {}

/*
 * add fd to the set of fds to track
 * using select system call.
 * dont pass a non-open fd
 */
void selectah::add_rfd(int fd) {
	m_rfds.insert(fd);
}


int selectah::remove_rfd(int fd) {
	return m_rfds.erase(fd);
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
		status = process(i);

		if(status == SEL_STOP) {
			return i;
		}

		rfd_set(&rfds);
		timeout = m_timeout;
		
		//cout << "select fds" << endl; cout.flush();
		n_fds = select(high_rfd()+1, &rfds, (fd_set *) 0, (fd_set *) 0, &timeout);
		//cout << "done selecting fds" << endl; cout.flush();
		

		if(n_fds < 0) {
			throw errno;
		}
		else if(n_fds == 0) {
			continue;
		}
		
		/* if we are here we have fds to look at */
		status = on_rfds(&rfds);

		if(status == SEL_STOP) {
			return i;
		}
	}

	return i;
}


/*
 * turn our std::set container into a
 * fd_set data type required to pass
 * to the  select system call
 */
void selectah::rfd_set(fd_set *result) const {
	std::set<int>::iterator it;

	FD_ZERO(result);

	for(it = m_rfds.begin(); it != m_rfds.end(); it++) {
		FD_SET(*it, result);
	}
}

/*
 * return the fd which is the 
 * highest numbered fd we 
 * are tracking
 */
int selectah::high_rfd() const {
	if(m_rfds.size() < 1) {
		return 0;
	}

	return *std::max_element(m_rfds.begin(), m_rfds.end() );
}

