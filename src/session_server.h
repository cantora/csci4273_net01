#ifndef SESSION_SERVER_H
#define SESSION_SERVER_H

#include <map>
#include <queue>
//#include <iostream>

#include "sel_tcp_srv.h"
#include "proto_chat.h"
#include "sock.h"
#include "msg.h"

namespace net01 {

class session_server : public sel_tcp_srv {
	public:
		session_server(const std::string &name, int socket) : sel_tcp_srv(socket), m_name(name) {}

	protected:
		virtual selectah::selectah_status_t consume(int socket);	
		virtual selectah_status_t process(int n);
	
	private:

		class client {
			public:
				client() : state(CLI_OK), in_msg_recd(0), request(proto_chat::NOOP), msg_id(NULL_MSG_ID) {}
				client(const client &other) 
				 : request(other.request), in_msg(other.in_msg), state(CLI_OK),
				 in_msg_recd(0), msg_id(other.msg_id) {

					assert(other.in_msg_ostrm.get() == 0);
					assert(other.state == CLI_OK);

				}
				
				void reset_in_msg() {
					in_msg.clear();
					in_msg_recd = 0;
					in_msg_ostrm = in_msg.ostrm();
				}

				enum state_t { CLI_OK, CLI_ERR, CLI_ERR_AND_CLOSE, CLI_CLOSE};

				//std::queue<char> requests;
				char request;
				state_t state;
				std::auto_ptr<std::ostream> in_msg_ostrm;
				int in_msg_recd;
				msg in_msg;
				uint32_t msg_id;
					
				static const uint32_t NULL_MSG_ID = 0xffffffff;

			private:
				void operator= (const client &other);
				
					
		};

		class chat_msg {
			public:
				chat_msg(const std::string &username, const msg &message) : user(username), m(message) {}

				std::string user;
				msg m;
				
		};

		std::map<int, client> m_clients;
		std::vector<chat_msg> m_msg_table;
		std::string m_name;


		selectah::selectah_status_t consume_msg(int socket);

		void socket_to_username(int socket, std::string &username) const;
		client::state_t get_request(int socket, char &opcode);
		
		selectah::selectah_status_t respond_to_client(int socket);
		
		void close_client(int socket);
		void handle_get_next(int socket);
		void handle_get_all(int socket);
		client::state_t rpl_get_next(int socket);
		
		client::state_t receive_msg_id(int socket);	
		
}; /* session_server */

}; /* net01 */

#endif /* SESSION_SERVER_H */


/*
void copy(const client &other) {
					requests = other.requests;
					in_msg = other.in_msg;

					assert(other.in_msg_ostrm.get() == 0);
					in_msg_ostrm.release();

					assert(other.state != CLI_OK);
					state = CLI_OK;
				}
				

*/