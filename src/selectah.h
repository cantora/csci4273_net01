#ifndef SELECTAH_H
#define SELECTAH_H

#include <cerrno>
#include <cstring>
#include <set>
#include <algorithm>

extern "C" {
#include <sys/select.h>
}

/*
 * selectah
 */
namespace net01 {

class selectah {
	public:
		selectah();
		~selectah();

		enum selectah_status_t {SEL_OK, SEL_STOP};

		int add_rfd(int fd);
		int select_fds(int n);
		int rfd_set_size() const;

	protected:
		int high_rfd() const;
		void rfd_set(fd_set *result) const;
		
		virtual selectah_status_t on_rfds(const fd_set *rfd_set) = 0;

		/* file descriptors we want to wake up when readable */
		std::set<int> m_rfds;

		struct timeval m_timeout;
		
	private:

}; /* selectah */

}; /* net01 */
#endif /* SELECTAH_H */