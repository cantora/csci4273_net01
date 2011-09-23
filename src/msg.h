#ifndef MSG_H
#define MSG_H

#include <iostream>
#include <sstream>
#include <memory>

namespace net01 {

class msg {
	public:
		msg() {}
		msg(const std::string &str) : m_data(str) {}
		msg(const char *c_str) : m_data(c_str) {}
		//msg(const msg &other);		

		size_t size() const {return m_data.size(); }
		std::auto_ptr<std::istream> istrm() const;

		void set_data(std::string &str) { m_data = str;}

		bool msg::operator== (msg &other) const;
		bool msg::operator== (std::string &str) const;

	private:
		std::string m_data;
		
}; /* msg */

}; /* net01 */
#endif /* MSG_H */