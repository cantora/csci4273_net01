#ifndef SESSION_SERVER_H
#define SESSION_SERVER_H

#include <map>
#include <queue>
//#include <iostream>
#include <cassert>
#include <ctime>

#include "sel_tcp_srv.h"
#include "proto_chat.h"
#include "sock.h"
#include "msg.h"

namespace net01 {

class session_server : public sel_tcp_srv {
	public:
		session_server(const std::string &name, int socket, int udp_socket, struct sockaddr_in coord_sin, const std::string &token) : sel_tcp_srv(socket), m_name(name), m_udp_socket(udp_socket), m_coord_sin(coord_sin), m_token(token) {
			m_timeout.tv_sec = 0;
			m_timeout.tv_usec = 100000; //100 ms
		
			m_last_activity = time(NULL);
		}

		~session_server() {
			std::cout << "goodbye from " << m_name << std::endl;
			close(m_udp_socket);
		}

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
					assert(other.in_msg_recd == 0);
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
		const std::string m_name;
		const std::string m_token;
		const int m_udp_socket;
		const struct sockaddr_in m_coord_sin;
			
		time_t m_last_activity;
		

		selectah::selectah_status_t consume_msg(int socket);

		void socket_to_username(int socket, std::string &username) const;
		client::state_t get_request(int socket, char &opcode);
		
		selectah::selectah_status_t respond_to_client(int socket);
		
		void close_client(int socket);
		void handle_get_next(int socket);
		void handle_get_all(int socket);
		client::state_t rpl_get_next(int socket);
		
		client::state_t receive_msg_id(int socket);	
	
		void self_terminate();		
}; /* session_server */

}; /* net01 */

#endif /* SESSION_SERVER_H */
