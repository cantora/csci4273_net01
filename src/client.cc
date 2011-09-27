#include "client.h"

#include "sock.h"

extern "C" {
#include "poll.h"
}

using namespace std;
using namespace net01;

void client::loop() {
	while(1) {
		//check_input();
		select_fds(1);
	}
}

char client::prompt() const {
	char cmd;
	cout << endl << "enter j->join, m->send msg, d->drop cx, e->exit: ";
	cin >> cmd;

	return cmd;
}

void client::check_input() {
	char cmd;
	cmd = prompt();

	switch(cmd) {
		case SND_MSG_CMD:
			//cout << "send msg" << endl;
			do_snd_msg();
			break;
		
		case JOIN_CMD:
			//cout << "join" << endl;
			do_join();
			break;
		
		case DROP_CMD:
			//cout << "drop" << endl;
			do_drop();
			break;

		case EXIT_CMD:
			cout << "exit" << endl;
			exit(0);
			break;

		default:
			/* do nothing */;
	} /*switch*/

}

selectah::selectah_status_t client::process(int n) {

	return SEL_OK;
}

selectah::selectah_status_t client::on_rfds(const fd_set *rfd_set) {
	fd_set set;

	//cout << "on rfds" << endl;
	FD_COPY(rfd_set, &set);

	if(is_open() ) {
		if(FD_ISSET(m_channel->socket, &set) != 0) {
			consume_msg();						
		}
	}

	if(FD_ISSET(0, &set) != 0) {
		cout << "check input" << endl;
		check_input();
	}
}

void client::close_channel() {
	assert(m_channel.get() != 0);

	remove_rfd(m_channel->socket);
	close(m_channel->socket);
	m_channel.release();
}

void client::consume_msg() { 
	if(m_channel->in_msg_ostrm.get() == 0) {
		cout << "start receiving new message" << endl;
		m_channel->reset_in_msg();
	}
	
	proto_chat::recv_status_t status;
	status = proto_chat::recv_msg(m_channel->socket, *(m_channel->in_msg_ostrm), m_channel->in_msg_recd);
		
	if(status == proto_chat::RCV_DONE) {
		m_channel->in_msg_ostrm.release();
				
		cout << m_channel->incoming.str() << endl;
	}
	else if(status == proto_chat::RCV_CLOSE) {
		cout << "lost connection" << endl;
		close_channel();
	}
	else if(status == proto_chat::RCV_MSG_MAX) {
		cout << "message too big" << endl;
		assert(false);
	}
	else {
		assert(status == proto_chat::RCV_CONT);
	}	
}

void client::do_join() {
	string host;
	unsigned short port;

	cout << "enter host: ";
	cin >> host;
	cout << "enter port: ";
	cin >> port;

	cout << "join " << host << ":" << port << endl;

	try {
		join(host, port);
	}
	catch(int e) {
		cout << "error: " << strerror(e) << endl;
		if(is_open() ) {
			close_channel();
		}
	}
}

void client::do_snd_msg() {
	string str;
	
	if(!is_open() ) {
		cout << "error: no channel has been joined" << endl;
		return;
	}

	cout << "enter message: ";
	cin >> str;
	
	istringstream istrm(str);
	try {
		send_msg(istrm);
	}
	catch(int e) {
		cout << "error: " << strerror(e) << endl;
	}
}

void client::do_drop() {
	
	if(!is_open() ) {
		cout << "error: no channel has been joined" << endl;
		return;
	}

	close_channel();
}

void client::join(const string &host, unsigned short port) {
	struct sockaddr_in sin;
	int s;

	if(is_open() ) {
		close_channel();
	}

	sock::host_sin("localhost", port, &sin);

	//cout << "create connection" << endl;
	s = sock::tcp_cx(&sin);
	//cout << "set non blocking" << endl;
	sock::set_nonblocking(s);

	//cout << "init channel" << endl;
	m_channel.reset(new channel(s) );
	//cout << "add rfd" << endl;

	cout << "joined channel" << endl;
	add_rfd(s);
	
}

void client::send_msg(istream &istrm) {
	proto_chat::send_status_t status;
	int sent;

	assert(is_open());

	status = proto_chat::send_msg(m_channel->socket, istrm, sent);
	
	if(status == proto_chat::SND_CLOSE) {
		close_channel();
	}

	//assert(status != SND_MSG_MAX);
	assert(status == proto_chat::SND_DONE);

}


