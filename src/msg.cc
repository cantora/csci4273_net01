#include "msg.h"

using namespace std;
using namespace net01;

const std::ios_base::openmode msg::m_mode = (std::ios_base::out | std::ios_base::in | std::ios_base::app | std::ios_base::in | std::ios_base::ate | std::ios_base::binary);

auto_ptr<istream> msg::istrm() const { 
	return auto_ptr<istream>(new istringstream(m_data.str(), istringstream::in | istringstream::binary) );
}

auto_ptr<ostream> msg::ostrm() { 
	return auto_ptr<ostream>(new ostream(&m_data) );
}

bool msg::operator== (const msg &other) const {
	return (m_data.str() == other.m_data.str());
}

bool msg::operator== (const string &str) const {
	return (m_data.str() == str);
}
