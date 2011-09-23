#include "msg.h"

using namespace std;
using namespace net01;

auto_ptr<istream> msg::istrm() const { 
	return auto_ptr<istream>(new istringstream(m_data, istringstream::in | istringstream::binary) );
}

bool msg::operator== (msg &other) const {
	return (m_data == other.m_data);
}

bool msg::operator== (string &str) const {
	return (m_data == str);
}
