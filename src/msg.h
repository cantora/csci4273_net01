#ifndef MSG_H
#define MSG_H

#include <iostream>
#include <sstream>
#include <memory>

namespace net01 {

class msg {
	public:
		msg() {}
		msg(const std::string &str) : m_data(str, m_mode) {}
		msg(const char *c_str) : m_data(c_str, m_mode) {}
		msg(const msg &other) : m_data(other.m_data.str(), m_mode) {}
		
		void operator= (msg &other) { m_data.str(other.m_data.str()); }

		size_t size() const {return m_data.str().size(); }
		std::auto_ptr<std::istream> istrm() const;

		std::auto_ptr<std::ostream> ostrm(); // { return m_data;}

		bool operator== (const msg &other) const;
		bool operator== (const std::string &str) const;

	private:
		std::stringbuf m_data;
		static const std::ios_base::openmode m_mode;
		
}; /* msg */

}; /* net01 */
#endif /* MSG_H */