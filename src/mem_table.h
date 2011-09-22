#ifndef MEM_TABLE_H
#define MEM_TABLE_H

#include "seq_table.h"
#include <sstream>

/*
 * a table of character buffers
 * in memory
 */
namespace net01 {

class mem_table : public seq_table {
	public:
		
		~mem_table();

		virtual size_t size() const {return m_vec.size(); }
		
		virtual void insert(const std::string &str);
		
		virtual std::istream_iterator<char> at(int index);
		virtual void at(int index, std::string *str);

	private:
		std::vector<std::istringstream *> m_vec;
		
}; /* mem_table */

}; /* net01 */
#endif /* MEM_TABLE_H */