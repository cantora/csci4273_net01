#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <string>

#include "msg.h"
#include "proto_chat.h"
#include "selectah.h"

namespace net01 {

class client : public selectah {
	public:
		client() {
			m_timeout.tv_sec = 0;
			m_timeout.tv_usec = 100000;
			add_rfd(0);
		}

		~client() {
			if(is_open() ) {
				close_channel();
			}
		}

		void loop();
		
	protected:
		selectah::selectah_status_t process(int n);
		selectah::selectah_status_t on_rfds(const fd_set *rfd_set);

	private:
		void check_input();
		void close_channel();
		char prompt() const;
		void do_join();
		void do_snd_msg();
		void do_drop();

		void join(const std::string &host, unsigned short port);
		void send_msg(std::istream &istrm);
		bool is_open() const { return (m_channel.get() != 0); }
		void consume_msg();

		static const char SND_MSG_CMD = 'm';
		static const char JOIN_CMD = 'j';
		static const char DROP_CMD = 'd';
		static const char EXIT_CMD = 'e';
				
		class channel {
			public:
				channel(int sock) : socket(sock), last_msg_id(0) {}
				~channel() { close(socket); }

				void reset_in_msg() {
					incoming.clear();
					in_msg_ostrm.release();
					in_msg_recd = 0;
				}

				uint32_t last_msg_id;
				int socket;
				msg incoming;
				std::auto_ptr<std::ostream> in_msg_ostrm;
				int in_msg_recd;

		
		}; /* channel */

		std::auto_ptr<channel> m_channel;
		
}; /* client */

}; /* net01 */
#endif /* CLIENT_H */