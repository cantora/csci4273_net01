#ifndef SESSION_SERVER_H
#define SESSION_SERVER_H

#include <map>

#include "sel_tcp_srv.h"
#include "proto_chat.h"
#include "sock.h"
#include "msg.h"

namespace net01 {

class session_server : public sel_tcp_srv {
	public:
		session_server(std::string &name, socket);
		~session_server();

	protected:
		virtual selectah::selectah_status_t consume(int socket);	
	
	private:
		selectah::selectah_status_t consume_msg(int socket);

		void socket_to_username(int socket, std::string &username) const;
		selectah::selectah_status_t queue_requests(int socket);

		class client {
			public:
				client() : state(CLI_OK) {}

				enum state_t { CLI_OK, CLI_ERR, CLI_ERR_AND_CLOSE, CLI_CLOSE };
				std::queue<char> requests;
				std::auto_ptr<msg> in_msg;
				std::auto_ptr<std::ostream> in_msg_ostrm;
				state_t state;
				
		};

		class chat_msg {
			public:
				chat_msg(std::string &username, msg &message) : user(username), m(message) {}

				std::string user;
				msg m;
				
		};

		std::map<int, client> m_clients;
		std::vector<const chat_msg> m_msg_table;
}; /* session_server */

}; /* net01 */

#endif /* SESSION_SERVER_H */
