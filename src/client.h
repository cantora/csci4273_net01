#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <string>

#include "msg.h"
#include "proto_chat.h"
#include "selectah.h"

extern "C" {
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
}

/*extern "C" {
#include <signal.h>
todo: handler sigpipe
}*/

namespace net01 {

class client : public selectah {
	public:
		client(struct sockaddr_in coord_sin, int udp_socket) : m_coord_sin(coord_sin), m_udp_socket(udp_socket) {
			m_timeout.tv_sec = 0;
			m_timeout.tv_usec = 1000;
			add_rfd(STDIN_FILENO);
			m_poll_interval = 1000; //ms
			m_msg_interval = 4000; //ms
			gettimeofday(&m_last_poll, NULL);
			gettimeofday(&m_last_msg_time, NULL);
		}

		~client() {
			if(is_open() ) {
				close_channel();
			}
			
			close(m_udp_socket);
		}

		void loop();
		
	protected:
		selectah::selectah_status_t process(int n);
		selectah::selectah_status_t on_rfds(const fd_set *rfd_set);

	private:

		int milli_since_last_msg();
		int milli_since_last_poll();
		void check_input();
		void close_channel();
		char prompt() const;
		void do_join();
		void do_snd_msg();
		void do_drop();
		void do_poll();

		void poll();
		void join(const std::string &host, unsigned short port);
		int send_msg(std::istream &istrm);
		void receive_responses();
		bool is_open() const { return (m_channel.get() != 0) && (m_channel->socket_ok); }
		void consume_msg();

		static const char SND_MSG_CMD = 'm';
		static const char JOIN_CMD = 'j';
		static const char DROP_CMD = 'd';
		static const char EXIT_CMD = 'e';
		static const char POLL_CMD = 'p';
		static const char FIND_CMD = 'f';
		static const char START_CMD = 's';
				
		class channel {
			public:
				channel(int sock) : socket(sock), last_msg_id(0), socket_ok(true) {}
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
				bool socket_ok;
		
		}; /* channel */

		std::auto_ptr<channel> m_channel;
		int m_poll_interval;
		int m_msg_interval;
		struct timeval m_last_msg_time;
		struct timeval m_last_poll;

		const struct sockaddr_in m_coord_sin;
		const int m_udp_socket;
		
}; /* client */

}; /* net01 */
#endif /* CLIENT_H */