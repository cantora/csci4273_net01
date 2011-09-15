#ifndef SESS_SRV_H
#define SESS_SRV_H

#include <cstdio>
#include "selectah.h"

namespace net01 {

class sess_srv : public selectah {
	public:
		sess_srv(int socket);
		~sess_srv();

	protected:

		virtual selectah::selectah_status_t sess_srv::on_rfds(const fd_set *rfd_set);

	private:

}; /* sess_srv */

}; /* net01 */

#endif /* SESS_SRV_H */