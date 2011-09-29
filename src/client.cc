#include "client.h"

#include "sock.h"
#include "proto_coord.h"
#include <cassert>

using namespace std;
using namespace net01;

void client::loop() {
	while(1) {
		//check_input();
		select_fds(1);
	}
}

char client::prompt() const {
	string str;
	char cmd;
	cout << endl << "j->join, m->msg, p->poll, d->drop, f->find, s->start, e->exit: ";
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

		case POLL_CMD:
			do_poll();
			break;

		case FIND_CMD:
			do_find();
			break;

		case START_CMD:
			do_start();
			break;
		
		default:
			/* do nothing */;
	} /*switch*/

	if(is_open() ) {
		cout << "listening for messages. press enter for prompt..." << endl;
	}
}

void client::do_start() {
	string sess_name;
	proto_coord::send_status_t status;
	struct sockaddr_in sin;
	socklen_t sinlen = sizeof(sin);

	cout << "enter session name: ";
	cin >> sess_name;

	if(sess_name.size() < 1) {
		return;
	}
	else if(sess_name.size() > 8) {
		cout << "session name too long" << endl;
		return;
	}

	status = proto_coord::send_sess_op(m_udp_socket, &m_coord_sin, sizeof(m_coord_sin), sess_name.c_str(), sess_name.size(), proto_coord::REQ_START );
	if(status == proto_coord::SND_ERR) {
		cout << "session name must only contain characters a-z" << endl;
		return;
	}
	
	assert(status == proto_coord::SND_DONE);

	char response;
	int msg_len;
	sleep(3);
	if( (msg_len = recvfrom(m_udp_socket, &response, 1, 0, (sockaddr *)&sin, &sinlen)) < 0) {
		if(errno == EAGAIN) {
			cout << "error starting session: response timeout" << endl;
			return;
		}
		throw errno;
	}
	else if(msg_len == 0) {
		cout << "error starting session: invalid response" << endl;
		return;
	}
	
	if(response == proto_coord::RPL_START) {
		cout << "started session " << sess_name << endl;
	}
	else {
		cout << "error starting session: session already exists" << endl;
	}

}

void client::do_find() {
	string sess_name;
	proto_coord::send_status_t status;
	struct sockaddr_in sin;
	socklen_t sinlen = sizeof(sin);

	cout << "enter session name: ";
	cin >> sess_name;

	if(sess_name.size() < 1) {
		return;
	}
	else if(sess_name.size() > 8) {
		cout << "session name too long" << endl;
		return;
	}

	status = proto_coord::send_sess_op(m_udp_socket, &m_coord_sin, sizeof(m_coord_sin), sess_name.c_str(), sess_name.size(), proto_coord::REQ_FIND );
	if(status == proto_coord::SND_ERR) {
		cout << "session name must only contain characters a-z" << endl;
		return;
	}
	
	assert(status == proto_coord::SND_DONE);

	char response[16];
	int msg_len;
	sleep(3);
	if( (msg_len = recvfrom(m_udp_socket, response, 16, 0, (sockaddr *)&sin, &sinlen)) < 0) {
		if(errno == EAGAIN) {
			cout << "error starting session: response timeout" << endl;
			return;
		}
		throw errno;
	}
	else if(msg_len == 0) {
		cout << "error starting session: invalid response" << endl;
		return;
	}
	
	if(response[0] == proto_coord::RPL_FIND) {
		uint16_t port = 0;
		proto_coord::recv_status_t status = proto_coord::recv_sess_port(response, msg_len, port);

		if(port == 0 || status != proto_coord::RCV_DONE) {
			cout << "error finding session: invalid response" << endl;
			return;
		}

		cout << "session " << sess_name << " at port: " << port << endl;
	}
	else {
		cout << "could not find session " << sess_name << endl;
	}
}

int client::milli_since_last_msg() {
	struct timeval t;

	long mtime, seconds, useconds;    

	gettimeofday(&t, NULL);

	seconds  = t.tv_sec  - m_last_msg_time.tv_sec;
	useconds = t.tv_usec - m_last_msg_time.tv_usec;

    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

    //cout << "t: " << t.tv_sec << endl << "last_poll: " << m_last_msg_time.tv_sec << endl;
	//cout << mtime << endl;
	//cout << m_poll_interval << endl;
	
	return mtime;
}

int client::milli_since_last_poll() {
	struct timeval t;

	long mtime, seconds, useconds;    

	gettimeofday(&t, NULL);

	seconds  = t.tv_sec  - m_last_poll.tv_sec;
	useconds = t.tv_usec - m_last_poll.tv_usec;

    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

    //cout << "t: " << t.tv_sec << endl << "last_poll: " << m_last_msg_time.tv_sec << endl;
	//cout << mtime << endl;
	//cout << m_poll_interval << endl;
	
	return mtime;
}

selectah::selectah_status_t client::process(int n) {
	
	if(is_open() && ((milli_since_last_msg() >= m_msg_interval) && (milli_since_last_poll() >= m_poll_interval) ) ) {
		//poll();
	}
	
	if(!is_open() ) {
		check_input();
	}
	
	//cout << ",";
	cout.flush();
	return SEL_OK;
}

selectah::selectah_status_t client::on_rfds(const fd_set *rfd_set) {
	fd_set set;
	
	//cout << "on rfds" << endl;
	//FD_COPY(rfd_set, &set); /* this doenst work on linux? */
	//memcpy(&set, rfd_set, sizeof(set) ); /* doesnt work either? */
	set = *rfd_set;

	if(is_open() ) {
		if(FD_ISSET(m_channel->socket, &set) != 0) {
			receive_responses();
		}
	}

	if(FD_ISSET(STDIN_FILENO, &set) != 0) {
		//cout << "check input" << endl;
		check_input();
	}
}

void client::receive_responses() {
	proto_chat::recv_status_t status;
	char opcode = 0;
	int recd = 0;

	//cout << "get data from server" << endl;

	assert(is_open() );
	
	status = proto_chat::RCV_DONE;

	while(is_open() ) {
		status = proto_chat::recv_opcode(m_channel->socket, opcode);
		if(status == proto_chat::RCV_CLOSE ) {
			close_channel();
			return;
		}
		else if(status == proto_chat::RCV_CONT) {
			//cout << "no data to read from server" << endl;
			return;
		}
		
		assert(status == proto_chat::RCV_DONE);
	
		switch(opcode) {
			case proto_chat::RPL_ERR : /* keep going */
			case proto_chat::RPL_ERR_MSG_TOO_LONG : 
				cout << "server error" << endl;
				close_channel();
				break;
			
			case proto_chat::MSG_START : 
				//cout << "start receiving message from server" << endl;
				consume_msg();
				break;
	
			default	: 
				cout << "unexpected reply from server: 0x" << hex << (int) opcode << endl;
				assert(false);
		}	
	} /* while */	
}

void client::close_channel() {
	assert(m_channel.get() != 0);
	
	cout << "close channel to server" << endl;
	remove_rfd(m_channel->socket);
	close(m_channel->socket);
	m_channel.release();
}

void client::consume_msg() { 

	if(m_channel->in_msg_ostrm.get() == 0) {
		//cout << "start receiving new message" << endl;
		m_channel->reset_in_msg();
		m_channel->in_msg_ostrm = m_channel->incoming.ostrm();
	}
	/*else {
		cout << "continue receiving message. recd: " << m_channel->in_msg_recd << endl;
		cout << "message received so far: " << m_channel->incoming.str() << endl;
	}*/	
	
	uint32_t msg_id_tmp = 0xffffffff;

	proto_chat::recv_status_t status = proto_chat::RCV_CONT;
	while(status == proto_chat::RCV_CONT) {
		status = proto_chat::recv_msg(m_channel->socket, *(m_channel->in_msg_ostrm), msg_id_tmp, m_channel->in_msg_recd);
	}
		
	if(status == proto_chat::RCV_DONE) {
		if(m_channel->last_msg_id+1 == msg_id_tmp) {
			m_channel->last_msg_id = msg_id_tmp;
			cout << m_channel->last_msg_id << ": " << m_channel->incoming.str() << endl;			
		}
		else {
			//assert(m_channel->last_msg_id == msg_id_tmp);
			cout << "got out of order message at id " << msg_id_tmp << ": " << m_channel->incoming.str() << endl;
		}
		
		gettimeofday(&m_last_msg_time, NULL);
		m_channel->in_msg_ostrm.release();		
		//cout << ".";
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
		assert(false); /* shouldnt get here */
	}	
}

void client::do_join() {
	string host(inet_ntoa(m_coord_sin.sin_addr) );
	unsigned short port;

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
	
		return;
	}

	//poll();
}

void client::do_snd_msg() {
	string str;
	
	if(!is_open() ) {
		cout << "error: no channel has been joined" << endl;
		return;
	}

	cout << "enter message: ";
	getline(cin, str);
	getline(cin, str);
	
	if(str.size() >= proto_chat::MAX_MSG_LEN) {
		cout << "truncating message to maximum length of " << proto_chat::MAX_MSG_LEN - 1 << " characters" << endl;
		str.resize(proto_chat::MAX_MSG_LEN - 1);
	}
	
	if(str.size() < 1) {
		return;
	}

	istringstream istrm(str);
	try {
		int sent = send_msg(istrm);
		if(sent > 0) {
			assert(sent > str.size());
		}
	}
	catch(int e) {
		cout << "error: " << strerror(e) << endl;
		if(is_open() ){
			close_channel();
		}

		return;
	}
	cout << "sent message \"" << str << '"' << endl;

	//poll();
}

void client::do_poll() {
	

	if(!is_open() ) {
		cout << "error: no channel has been joined" << endl;
		return;
	}

	cout << "poll for msgs greater than " << m_channel->last_msg_id << endl;
	poll();
}

void client::poll() {
	proto_chat::send_status_t status;

	assert(is_open());

	status = proto_chat::send_opcode(m_channel->socket, proto_chat::REQ_GET_ALL);
	
	if(status == proto_chat::SND_CLOSE) {
		close_channel();
		return;
	}

	assert(status == proto_chat::SND_DONE);

	//cout << "send msg id" << endl;

	status = proto_chat::send_msg_id(m_channel->socket, m_channel->last_msg_id);
	if(status == proto_chat::SND_CLOSE) {
		close_channel();
		return;
	}

	assert(status == proto_chat::SND_DONE);

	gettimeofday(&m_last_poll, NULL);
}

void client::do_drop() {
	
	if(!is_open() ) {
		cout << "error: no channel has been joined" << endl;
		return;
	}

	close_channel();
	cout << "closed connection to server" << endl;	
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

int client::send_msg(istream &istrm) {
	proto_chat::send_status_t status;
	int sent = 0;

	assert(is_open());
	//todo: handle sigpipe

	status = proto_chat::send_msg(m_channel->socket, istrm, 0, sent);
	
	if(status == proto_chat::SND_CLOSE) {
		close_channel();
		return 0;
	}

	//assert(status != SND_MSG_MAX);
	assert(status == proto_chat::SND_DONE);

	return sent;
}


